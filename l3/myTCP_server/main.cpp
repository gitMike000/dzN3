#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <cstring>

#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
#include <socket_wrapper/socket_class.h>

#include <sys/socket.h>
#include <netdb.h>


int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int port { std::stoi(argv[1]) };

    struct addrinfo hints, *addr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;// AF_INET6;//AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    getaddrinfo(NULL, argv[1], &hints, &addr);

    socket_wrapper::Socket sock = { addr->ai_family, addr->ai_socktype, addr->ai_protocol };

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    if  (bind(sock, addr->ai_addr, addr->ai_addrlen) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    // socket address used to store client address
    ssize_t recv_len = 0;

    std::cout << "Running echo server...\n" << std::endl;    

    bool run = true;

    listen(sock, 5);

    int newsockfd = -1;

    struct sockaddr_storage client_addr = { 0 };
    socklen_t client_len = sizeof(client_addr);
    char ipbuff[INET_ADDRSTRLEN];

    while (newsockfd == -1)
        newsockfd = accept(sock, reinterpret_cast< sockaddr *>(&client_addr), &client_len);

    std::cout << "Connected" << std::endl;

    close(sock);

    std::string str_buffer;

    while (run)
    {
        if (newsockfd < 0)
        {
            throw std::runtime_error("ERROR on accept");
        }

        //char buffer[256] = { 0 };
        //recv_len = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);

        //std::string str_buffer;
        std::string str_buffer;
        str_buffer.resize(256,'\0');
        recv_len = recv(newsockfd, &str_buffer[0], str_buffer.size(), 0);

        if (recv_len > 0)
        {
            //buffer[recv_len] = '\0';
            //str_buffer = std::string(buffer);

            str_buffer[recv_len] = '\0';

            char hbuf[NI_MAXHOST] = "\0";
            getnameinfo(reinterpret_cast<sockaddr *>(&client_addr), client_len, hbuf, sizeof(hbuf), nullptr, 0, NI_NAMEREQD);

            if (AF_INET == ((struct sockaddr_in *)&client_addr)->sin_family)
            {
                inet_ntop(client_addr.ss_family, &(((struct sockaddr_in *)&client_addr)->sin_addr), ipbuff, INET_ADDRSTRLEN);
            }
            else  if (AF_INET6 == ((struct sockaddr_in6 *)&client_addr)->sin6_family)
            {
                inet_ntop(client_addr.ss_family, &(((struct sockaddr_in6 *)&client_addr)->sin6_addr), ipbuff, INET_ADDRSTRLEN);
            }
            else
            {
                *ipbuff = client_addr.ss_family;
            }
            std::cout
                << "Client with address "
                << ipbuff
                << "(name=" << hbuf<<")"
                << ":" << ((struct sockaddr_in *)&client_addr)->sin_port
                << " sent datagram "
                << "[length = "
                << recv_len
                << "]:\n'''\n"
                << str_buffer
                << "\n'''"
                << std::endl;

            send(newsockfd, str_buffer.c_str(), recv_len, 0);
               //Exit
            if (str_buffer.find("exit") == 0)
            {
                std::cout << "Breaking" << std::endl;
                run = false;
            };
        }
    }

    close(newsockfd);
    freeaddrinfo(addr);

    return EXIT_SUCCESS;
}
