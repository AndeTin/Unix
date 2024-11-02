#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

const char* fifopath = "./my_fifo";
const char* filepath = "./data.db";

class data {
    private:
        struct node {
            std::string id;
            std::string name;
            int deposit;
            node* next = nullptr;
        }
        *head, *tail;
        int data_count = 0;
    public:
        data() {
            head = nullptr;
            tail = nullptr;
        }
void insert_data(string id, string name, int deposit) {
    // Create a new node with the provided data
    node* newNode = new node;
    newNode->id = id;
    newNode->name = name;
    newNode->deposit = deposit;
    newNode->next = nullptr;

    // If the list is empty, make the new node both the head and tail
    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    }
    // If the new node should be the first node in the list
    else if (id < head->id) {
        newNode->next = head;
        head = newNode;
    }
    else {
        // Find the correct insertion point
        node* current = head;
        node* previous = nullptr;

        while (current != nullptr && current->id < id) {
            previous = current;
            current = current->next;
        }

        // Insert the new node at the found position
        newNode->next = current;
        if (previous != nullptr) {
            previous->next = newNode;
        }

        // If the new node is inserted at the end, update the tail
        if (current == nullptr) {
            tail = newNode;
        }
    }

    data_count++; // Increment the count of nodes
}

        void remove_data(string id) {
            node* current = head;
            node* prev = nullptr;
            while (current != nullptr) {
                if (current->id == id) {
                    if (prev == nullptr) {
                        head = current->next;
                        data_count--;
                        return;
                    }
                    else {
                        prev->next = current->next;
                        data_count--;
                        return;
                    }
                }
                else {
                    prev = current;
                    current = current->next;
                }
            }
            std::cout << "Data not found" << std::endl;
        }
        void display_data() {
            if (data_count == 0) {
                std::cout << "No data available" << std::endl;
                return;
            }
            node* current = head;
            while (current != nullptr) {
                std::cout <<"----------------------------------------------------" << std::endl;
                std::cout << "ID: " << current->id << std::endl;
                std::cout << "Name: " << current->name << std::endl;
                std::cout << "Deposit: " << current->deposit << std::endl;
                std::cout << std::endl;
                current = current->next;
            }
        }
        void search_data(string id) {
            node* current = head;
            while (current != nullptr) {
                if (current->id == id) {
                    std::cout << "ID: " << current->id << std::endl;
                    std::cout << "Name: " << current->name << std::endl;
                    std::cout << "Deposit: " << current->deposit << std::endl;
                    return;
                }
                current = current->next;
            }
            std::cout << "Data not found" << std::endl;
        }
        bool check_duplicate(string id) {
            node* current = head;
            while (current != nullptr) {
                if (current->id == id) {
                    std::cout << "ID already exists" << std::endl;
                    return true;
                }
                current = current->next;
            }
            return false;
        }
        void save_data() {
            std::ofstream file(filepath);
            if (file.is_open()) {
                node* current = head;
                while (current != nullptr) {
                    file << current->id << " " << current->name << " " << current->deposit << std::endl;
                    current = current->next;
                }
                file.close();
            }
            else {
                std::cout << "Unable to open file" << std::endl;
            }
        }
}

int main() {
    data data;
    std::string id, name;
    int deposit;
    int choice;
    mkfifo(fifopath, 0666);
    if (mkfifo(fifopath, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }
    char buf[1024];
    int bytesRead = 0;
    std::string data_str(buf, bytes_read);


    while (true) {
        int fd = open(fifopath, O_RDONLY);
            if (fd == -1) {
                perror("open");
                return 1;
            }
        ssize_t bytes_read = read(fd, buf, sizeof(buf));
        if (bytes_read == -1) {
            perror("read");
            return 1;
        }

        while (std::cin.fail()) {
            std::cin.clear(); // clear the error state
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard invalid input
            std::cout << "Invalid input. Please enter an integer: ";
            std::cin >> choice;
        }
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the string
            std::cout << "Message received from FIFO: " << buffer << std::endl;
        } else {
            std::cout << "No message received." << std::endl;
        }

        switch (choice) {
            case 1:
                std::cout << "Enter ID: ";
                std::cin >> id;
                if (data.check_duplicate(id)) {
                    printf("ID already exists.\n");
                    break;
                }
                std::cout << "Enter Name: ";
                std::cin >> name;
                std::cout << "Enter Deposit: ";
                std::cin >> deposit;
                while (std::cin.fail()) {
                    std::cin.clear(); // clear the error state
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // discard invalid input
                    std::cout << "Invalid input. Please enter an integer for Deposit: ";
                    std::cin >> deposit;
                }
                data.insert_data(id, name, deposit);
                data.save_data();
                break;
            case 2:
                std::cout << "Enter ID: ";
                std::cin >> id;
                data.remove_data(id);
                data.save_data();  
                break;
            case 3:
                std::cout << "Enter ID: ";
                std::cin >> id;
                data.search_data(id);
                break;
            case 4:
                data.display_data();
                break;
            default:
                std::cout << "Please enter a integer between 1 to 5." << std::endl;
                continue;
        }
    }
    close(fd);
    return 0;
}