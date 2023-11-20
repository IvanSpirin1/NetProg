#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <iostream>
#include <memory>

int main(){
	std::unique_ptr< sockaddr_in > self_addr(new sockaddr_in);
    std::unique_ptr< sockaddr_in > remote_addr(new sockaddr_in);
    int sr_code;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if( sock < 0 ) {
        std::cerr << "Failed to create socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    timeval timeout;
    timeout.tv_sec = 10;
    if( setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) < 0) {
        std::cerr << "Failed to set socket option SO_RCVTIMEO" << std::endl;
        exit(EXIT_FAILURE);
    }

    self_addr->sin_family = AF_INET;
    self_addr->sin_addr.s_addr = INADDR_ANY;
    self_addr->sin_port = htons(0);

    if( bind(sock, reinterpret_cast< const sockaddr* >(self_addr.get()), sizeof(sockaddr)) < 0 ) {
        std::cerr << "Failed to bind socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    remote_addr->sin_family = AF_INET;
    remote_addr->sin_addr.s_addr = inet_addr("172.16.40.1");
    remote_addr->sin_port = htons(13);

    char buf[256];

    if( sendto(sock, buf, 256, 0, reinterpret_cast< const sockaddr* >(remote_addr.get()), sizeof(sockaddr)) < 0 ) {
        std::cerr << "Failed to start exchange" << std::endl;
        exit(EXIT_FAILURE);
    }

    socklen_t clen = sizeof(sockaddr);

    sr_code = recvfrom(sock, buf, 256, 0, reinterpret_cast< sockaddr* >(remote_addr.get()), &clen);
    if( sr_code == -1 ) {
        std::cerr << "Failed to recieve message" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string daytime(buf, sr_code);

    std::cout << daytime << std::endl;
    
    return 0;
}