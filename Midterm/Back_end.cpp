#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <list>
#include <algorithm>

const char* request_fifo = "./my_fifo";
const char* response_fifo = "./response_fifo";
const char* db_path = "./data.db";

struct Account {
    std::string id;
    std::string name;
    int deposit;

    bool operator<(const Account& other) const {
        return id < other.id;
    }
};

std::list<Account> account_list;

void load_data() {
    std::ifstream file(db_path);
    if (!file.is_open()) {
        std::cerr << "Could not open data.db for reading. Starting with empty data.\n";
        return;
    }

    std::string id, name;
    int deposit;
    while (file >> id >> name >> deposit) {
        account_list.push_back({id, name, deposit});
    }
    file.close();
    account_list.sort();
}

void save_data() {
    std::ofstream file(db_path, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Could not open data.db for writing.\n";
        return;
    }

    for (const auto& account : account_list) {
        file << account.id << " " << account.name << " " << account.deposit << "\n";
    }
    file.close();
}

std::string add_account(const std::string& id, const std::string& name, int deposit) {
    for (const auto& account : account_list) {
        if (account.id == id) {
            return "Account with this ID already exists.\n";
        }
    }
    account_list.insert(
        std::upper_bound(account_list.begin(), account_list.end(), Account{id, name, deposit}),
        {id, name, deposit}
    );
    save_data();
    return "Account added successfully.\n";
}

std::string remove_account(const std::string& id) {
    for (auto it = account_list.begin(); it != account_list.end(); ++it) {
        if (it->id == id) {
            account_list.erase(it);
            save_data();
            return "Account removed successfully.\n";
        }
    }
    return "Account not found.\n";
}

std::string query_account(const std::string& id) {
    for (const auto& account : account_list) {
        if (account.id == id) {
            std::ostringstream oss;
            oss << "ID: " << account.id << ", Name: " << account.name << ", Deposit: " << account.deposit << "\n";
            return oss.str();
        }
    }
    return "Account not found.\n";
}

std::string show_all_accounts() {
    if (account_list.empty()) {
        return "No accounts available.\n";
    }
    std::ostringstream oss;
    for (const auto& account : account_list) {
        oss << "ID: " << account.id << ", Name: " << account.name << ", Deposit: " << account.deposit << "\n";
    }
    return oss.str();
}

void process_request(const std::string& request) {
    std::istringstream iss(request);
    int command;
    iss >> command;

    std::string response;
    std::string id, name;
    int deposit;

    switch (command) {
        case 1: // Add Account
            iss >> id >> name >> deposit;
            response = add_account(id, name, deposit);
            break;
        case 2: // Remove Account
            iss >> id;
            response = remove_account(id);
            break;
        case 3: // Query Account
            iss >> id;
            response = query_account(id);
            break;
        case 4: // Show All Accounts
            response = show_all_accounts();
            break;
        case 5: // Exit
            response = "Server shutting down...\n";
            break;
        default:
            response = "Invalid command.\n";
            break;
    }

    int response_fd = open(response_fifo, O_WRONLY);
    if (response_fd != -1) {
        write(response_fd, response.c_str(), response.size());
        close(response_fd);
    } else {
        perror("open response_fifo");
    }
}

int main() {
    load_data();

    mkfifo(request_fifo, 0666);
    mkfifo(response_fifo, 0666);

    int server_fd;
    char buffer[1024];

    while (true) {
        server_fd = open(request_fifo, O_RDONLY);
        if (server_fd == -1) {
            perror("open request_fifo");
            continue;
        }

        ssize_t bytes_read = read(server_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string request(buffer);

            process_request(request);

            if (request == "5") { // Exit command
                close(server_fd);
                break;
            }
        } else if (bytes_read == -1) {
            perror("read request_fifo");
        }

        close(server_fd);
    }

    unlink(request_fifo);
    unlink(response_fifo);
    return 0;
}
