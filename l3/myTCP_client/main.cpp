#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <cassert>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

int main(int argc, char const *argv[])
{

    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip4/ip6 or hostname> <port>" << std::endl;
        return EXIT_FAILURE;
    }

    struct addrinfo hints = { 0 };
    struct addrinfo *serv_addr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    int status = 0;
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &serv_addr)) != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return EXIT_FAILURE;
    }

    const int port { std::stoi(argv[2]) };

    char ipbuff[INET_ADDRSTRLEN];
    //serv_addr=serv_addr->ai_next;

    if (AF_INET == serv_addr->ai_addr->sa_family)
    {
        sockaddr_in const * const sin = reinterpret_cast<const sockaddr_in* const>(serv_addr->ai_addr);
        inet_ntop(AF_INET, &(sin->sin_addr), ipbuff, INET_ADDRSTRLEN);
    }
    else if (AF_INET6 == serv_addr->ai_addr->sa_family)
    {
        sockaddr_in6 const * const sin = reinterpret_cast<const sockaddr_in6* const>(serv_addr->ai_addr);
        inet_ntop(AF_INET6, &(sin->sin6_addr), ipbuff, INET_ADDRSTRLEN);
    }

    socket_wrapper::SocketWrapper sock_wrap;
    socket_wrapper::Socket sock = {serv_addr->ai_addr->sa_family, serv_addr->ai_socktype, serv_addr->ai_protocol};
    if (!sock)
    {
      std::cerr << sock_wrap.get_last_error_string() << std::endl;
      return EXIT_FAILURE;
    }

    if (connect(sock, serv_addr->ai_addr, serv_addr->ai_addrlen) != 0)
    {
       std::cerr << sock_wrap.get_last_error_string() << std::endl;
       // Socket will be closed in the Socket destructor.
       return EXIT_FAILURE;
    }

//    struct sockaddr_storage client_addr = {0};
//    socklen_t client_len = sizeof(client_addr);

    std::cout << "Starting client for "
              <<  ipbuff
              << " on the port " << port << "...\n";


    ssize_t recv_len = 0;

    bool run = true;
    std::cout<<"Enter message ('exit' to quit)"<<std::endl;

    while (run)
    {
        std::string str_buffer;
        std::getline(std::cin, str_buffer);

//        sendto(sock, str_buffer.c_str(), str_buffer.length(), 0, reinterpret_cast<const sockaddr *>(&serv_addr),
//               sizeof(serv_addr));

        send(sock, str_buffer.c_str(), str_buffer.length(), 0);

        char buffer_rec[256] = { 0 };
//        recv_len = recvfrom(sock, buffer_rec, sizeof(buffer_rec), 0,
//                            reinterpret_cast<sockaddr *>(&client_addr),
//                            &client_len);
        recv_len = recv(sock, buffer_rec, sizeof(buffer_rec), 0);

        if (recv_len > 0)
        {
            str_buffer = std::string(buffer_rec);
            std::cout << str_buffer << std::endl;
        };

           //Exit
        if ((str_buffer == "exit"))
        {
            std::cout << "Breaking" << std::endl;
            run = false;
        };

        std::cout << std::endl;
    }

    freeaddrinfo(serv_addr);
    // How him shutdowning?
    shutdown(sock,2);
    close(sock);
    return EXIT_SUCCESS;
}
