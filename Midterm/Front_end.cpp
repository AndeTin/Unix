#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>

const char* request_fifo = "./my_fifo";
const char* response_fifo = "./response_fifo";

void send_request(const std::string& request) {
    int fd = open(request_fifo, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return;
    }
    write(fd, request.c_str(), request.size());
    close(fd);
}

std::string receive_response() {
    char buffer[1024];
    int fd = open(response_fifo, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return "Error opening FIFO for reading\n";
    }
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
    } else {
        perror("read");
    }
    close(fd);
    return std::string(buffer);
}

void show_menu() {
    std::cout << "=== Account Management System ===\n";
    std::cout << "1. Add Account\n";
    std::cout << "2. Remove Account\n";
    std::cout << "3. Query Account\n";
    std::cout << "4. Show All Accounts\n";
    std::cout << "5. Exit\n";
    std::cout << "Enter your choice: ";
}

int main() {
    mkfifo(request_fifo, 0666);
    mkfifo(response_fifo, 0666);

    int choice;
    while (true) {
        show_menu();
        std::cin >> choice;
        printf("\n");

        std::ostringstream oss;
        oss << choice << " ";
        
        std::string id, name;
        int deposit;

        switch (choice) {
            case 1:
                std::cout << "Enter ID: ";
                std::cin >> id;
                std::cout << "Enter Name: ";
                std::cin >> name;
                std::cout << "Enter Deposit: ";
                std::cin >> deposit;
                while (std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(256, '\n');
                    std::cout << "Invalid input. Please enter a number: ";
                    std::cin >> deposit;
                }
                oss << id << " " << name << " " << deposit;
                send_request(oss.str());
                std::cout << receive_response();
                printf("\n\n");
                break;

            case 2:
                std::cout << "Enter ID to remove: ";
                std::cin >> id;
                oss << id;
                send_request(oss.str());
                std::cout << receive_response();
                printf("\n\n");
                break;

            case 3:
                std::cout << "Enter ID to query: ";
                std::cin >> id;
                oss << id;
                send_request(oss.str());
                std::cout << receive_response();
                printf("\n\n");
                break;

            case 4:
                send_request(oss.str());
                std::cout << receive_response();
                printf("\n\n");
                break;

            case 5:
                send_request(oss.str());
                std::cout << "Exiting...\n";
                return 0;

            default:
                std::cout << "Invalid choice, please try again.\n";
                break;
        }
    }
}
