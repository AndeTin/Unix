#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

const char* fifopath = "./my_fifo";
const char* filepath = "./data.db";

class Data {
private:
    struct Node {
        std::string id;
        std::string name;
        int deposit;
        Node* next;

        Node(const std::string& id, const std::string& name, int deposit, Node* next = nullptr)
            : id(id), name(name), deposit(deposit), next(next) {}
    };

    Node* head;
    Node* tail;
    int data_count;

public:
    bool rm = true;
    Data() : head(nullptr), tail(nullptr), data_count(0) {}

    ~Data() {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    bool check_duplicate(const std::string& id) const {
        for (Node* current = head; current; current = current->next) {
            if (current->id == id) {
                std::cout << "ID already exists" << std::endl;
                return true;
            }
        }
        return false;
    }

    void insert_data(const std::string& id, const std::string& name, int deposit) {
        if (check_duplicate(id)) {
            std::cout << "ID already exists, cannot insert." << std::endl;
            return;
        }

        Node* newNode = new Node(id, name, deposit, nullptr);

        if (!head || id < head->id) {
            newNode->next = head;
            head = newNode;
            if (!tail) tail = newNode;
        } else {
            Node* current = head;
            Node* prev = nullptr;

            while (current && id > current->id) {
                prev = current;
                current = current->next;
            }

            newNode->next = current;
            prev->next = newNode;

            if (!newNode->next) tail = newNode;
        }

        data_count++;
    }

    void remove_data(const std::string& id) {
        Node* current = head;
        Node* prev = nullptr;

        while (current) {
            if (current->id == id) {
                if (prev) prev->next = current->next;
                else head = current->next;

                if (!current->next) tail = prev;

                delete current;
                data_count--;
                rm = true;  // Successful removal
                return;
            }
            prev = current;
            current = current->next;
        }
        rm = false;  // Indicate removal failed (not found)
    }

    void save_data() const {
        std::ofstream file(filepath);
        if (file.is_open()) {
            for (Node* current = head; current; current = current->next) {
                file << current->id << " " << current->name << " " << current->deposit << std::endl;
            }
            if (file.fail()) {
                std::cout << "Error writing data to file" << std::endl;
            }
            file.close();
        } else {
            std::cout << "Unable to open file" << std::endl;
        }
    }

    std::string get_data(const std::string& id) const {
        std::ostringstream oss;
        for (Node* current = head; current; current = current->next) {
            if (current->id == id) {
                oss << "ID: " << current->id << "\n";
                oss << "Name: " << current->name << "\n";
                oss << "Deposit: " << current->deposit << "\n";
                return oss.str();
            }
        }
        return "Data not found\n";
    }

    std::string get_all_data() const {
        std::ostringstream oss;
        for (Node* current = head; current; current = current->next) {
            oss << "ID: " << current->id << "\n";
            oss << "Name: " << current->name << "\n";
            oss << "Deposit: " << current->deposit << "\n";
            oss << "----------------------------------" << "\n";
        }
        return oss.str();
    }
};

int main() {
    mkfifo(fifopath, 0666);

    int fd = open(fifopath, O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    Data data;
    char buf[1024];

    while (true) {
        ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
        if (bytes_read > 0) {
            buf[bytes_read] = '\0';

            std::istringstream iss(buf);
            int choice;
            iss >> choice;

            std::string id, name;
            int deposit;
            std::string response;

            switch (choice) {
                case 1:
                    iss >> id >> name >> deposit;
                    data.insert_data(id, name, deposit);
                    data.save_data();
                    response = "Account added successfully\n";
                    write(fd, response.c_str(), response.size());
                    break;
                case 2:
                    iss >> id;
                    data.remove_data(id);
                    if (data.rm) {
                        response = "Account removed successfully\n";
                    } else {
                        response = "Data not found\n";
                    }
                    data.save_data();
                    write(fd, response.c_str(), response.size());
                    break;
                case 3:
                    iss >> id;
                    response = data.get_data(id);
                    write(fd, response.c_str(), response.size());
                    break;
                case 4:
                    response = data.get_all_data();
                    write(fd, response.c_str(), response.size());
                    break;
                case 5:
                    std::cout << "Exiting..." << std::endl;
                    close(fd);
                    unlink(fifopath);
                    return 0;
                default:
                    response = "Invalid choice received\n";
                    write(fd, response.c_str(), response.size());
                    break;
            }
        }
    }
}
