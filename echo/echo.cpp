#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <memory>

std::pair< bool, std::string > RecvMessage(int work_socket) {
    ssize_t capacity = 32;
    std::unique_ptr< char[] > buf(new char[capacity]);
    std::string recieved_line;
    
    ssize_t line_size;
    while( (line_size = recv(work_socket, buf.get(), capacity, MSG_PEEK)) == capacity ) {
        if( line_size == -1 ) {
            return std::make_pair(false, recieved_line);
        }
        capacity *= 2;
        buf = std::unique_ptr< char[] >(new char[capacity]);
    }

    if( recv(work_socket, buf.get(), line_size, 0) == -1) {
        return std::make_pair(false, recieved_line);
    }

    recieved_line = std::string(buf.get(), line_size);

    return std::make_pair(true, recieved_line);
}

int main(){
	std::unique_ptr< sockaddr_in > self_addr(new sockaddr_in);
    std::unique_ptr< sockaddr_in > remote_addr(new sockaddr_in);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if( sock < 0 ) {
        std::cerr << "Failed to create socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if( setsockopt(sock ,SOL_SOCKET, SO_REUSEADDR,  &enable, sizeof enable) < 0 ) {
        std::cerr << "Failed to set socket option SO_REUSEADDR" << std::endl;
        exit(EXIT_FAILURE);
    }

    self_addr->sin_family = AF_INET;
    self_addr->sin_addr.s_addr = inet_addr("172.16.40.1");
    self_addr->sin_port = htons(7);

    if( bind(sock, reinterpret_cast< const sockaddr* >(self_addr.get()), sizeof(sockaddr)) < 0 ) {
        std::cerr << "Failed to bind socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket successfully binded" << std::endl;


    if( listen(sock, 3) < 0 ) {
        std::cerr << "Failed to start listen" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Listening..." << std::endl;

    int client_socket;
    uint16_t client_port;
    socklen_t addr_len = sizeof(sockaddr_in);
    bool isOpen = false;
    while( true ) {
        if( !isOpen ) {
            if( client_socket = accept(sock, reinterpret_cast< sockaddr* >(remote_addr.get()), &addr_len) ; client_socket < 0 ) {
                std::cerr << "Failed to accept client" << std::endl;
                continue;
            }
            isOpen = true;
            client_port = ntohs((remote_addr.get())->sin_port);
            std::cout << "Client accepted. Port: " << client_port << std::endl; 

        }

        std::pair<bool, std::string> message = RecvMessage(client_socket);
        if( message.first == false) {
            std::cout << "Failed to recieve message. Port: " << client_port << std::endl;
            close(client_socket);
            isOpen = false;
            continue;
        }

        std::cout << "Recieved message: " << message.second << std::endl;

        if( message.second.empty() ) {
            std::cout << "Finish exchange" << std::endl;
            close(client_socket);
            isOpen = false;
            continue;
        }

        std::cout << "Sending message: " << message.second << std::endl;

        if( send(client_socket, message.second.c_str(), message.second.size(), 0) < 0 ) {
            std::cout << "Failed to send message. Port: " << client_port << std::endl;
            close(client_socket);
            isOpen = false;
            continue;
        }
        std::cout << "Message sent\n\n" << std::endl;

    }
    return 0;
}