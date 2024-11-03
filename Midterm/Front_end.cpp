#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

const char* fifopath = "./my_fifo";

int main() {
    mkfifo(fifopath, 0666);

    int fd = open(fifopath, O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    std::string id, name;
    int deposit;
    int choice;
    char buf[1024];

    while (true) {
        printf("Server is running\n");

        while (std::cin.fail() || choice < 1 || choice > 4) {
            std::cin.clear();
            std::cin.ignore(256, '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 4: ";
            std::cin >> choice;
        }

        std::string message;
        switch (choice) {
            case 1:
                std::cout << "Enter ID: ";
                std::cin >> id;
                std::cout << "Enter Name: ";
                std::cin >> name;
                std::cout << "Enter Deposit: ";
                std::cin >> deposit;
                message = "1 " + id + " " + name + " " + std::to_string(deposit);
                write(fd, message.c_str(), message.size());
                break;
            case 2:
                std::cout << "Enter ID: ";
                std::cin >> id;
                message = "2 " + id;
                write(fd, message.c_str(), message.size());
                ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
                if (bytes_read > 0) {
                    buf[bytes_read] = '\0';
                    std::cout << buf;
                }

                break;
            case 3:
                std::cout << "Enter ID: ";
                std::cin >> id;
                message = "3 " + id;
                write(fd, message.c_str(), message.size());
                ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
                if (bytes_read > 0) {
                    buf[bytes_read] = '\0';
                    std::cout << buf;
                }
                break;
            case 4:
                message = "4";
                write(fd, message.c_str(), message.size());
                ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
                if (bytes_read > 0) {
                    buf[bytes_read] = '\0';
                    std::cout << buf;
                }
                break;
        }
        // Write the request to back_end
        write(fd, data.c_str(), data.size());

        // Read the response from back_end
        ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
        if (bytes_read > 0) {
            buf[bytes_read] = '\0';
            std::cout << buf;
        }
    }

    return 0;
}
