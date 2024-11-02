#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

const char* fifopath = "./my_fifo";

int main() {
    std::string id, name;
    int deposit;
    int choice;
    mkfifo(fifopath, 0666);
    if (mkfifo(fifoPath, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }
    int fd = open(fifopath, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    while(true){
        printf("Please select an option:\n
            \t1. Add a new account\n
            \t2. Remove an account\n
            \t3. Search for an account\n
            \t4. Display all accounts\n
            \t5. Exit\n");
        std::cin >> choice;
        while (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(256, '\n');
            std::cout << "Invalid input. Please enter a number between 1 to 5: ";
            std::cin >> choice;
        }
        switch (choice) {
            case 1:
                std::cout << "Enter the account ID: ";
                std::cin >> id;
                std::cout << "Enter the account name: ";
                std::cin >> name;
                std::cout << "Enter the initial deposit: ";
                std::cin >> deposit;
                while (std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(256, '\n');
                    std::cout << "Invalid input. Please enter a number: ";
                    std::cin >> deposit;
                }
                std::string data = choice + " " + id + " " + name + " " + std::to_string(deposit);
                write(fd, data.c_str(), data.size());
                break;
            case 2:
                std::cout << "Enter the account ID to remove: ";
                std::cin >> id;
                std::string data = choice + " " + id;
                write(fd, data.c_str(), data.size());
                break;
            case 3:
                std::cout << "Enter the account ID to search: ";
                std::cin >> id;
                std::string data = choice + " " + id;
                write(fd, data.c_str(), data.size());
                break;
            case 4:
                std::string data = choice;
                write(fd, data.c_str(), data.size());
                break;
            case 5:
                close(fd);
                return 0;
        }
    }
    return 0;
}