/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abettini <abettini@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/05 09:48:38 by abettini          #+#    #+#             */
/*   Updated: 2024/02/22 12:19:51 by abettini         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include <vector>
#define PORT 6663

int main()
{
    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return (1);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Error setsocketopt" << std::endl;
        return (1);
    }

    // Set up server address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    serverAddress.sin_port = htons(PORT); // Example port number, you can choose any available port

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return (1);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return (1);
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    std::vector<pollfd> fds;
    fds.push_back({serverSocket, POLLIN, 0});

    std::string msg;
    while (true)
    {
        int numready = poll(fds.data(), fds.size(), -1);
        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == serverSocket)
                {
                    int clientSocket = accept(serverSocket, nullptr, nullptr);
                    if (clientSocket != -1)
                    {
						char	welcom[] = "Welcome to the Internet Relay Network nick!user@host\n";
						char 	yourhost[] = "Your host is servername, running version version\n";
						char	create[] = "server_name version user_modes chan_modes\n";
                        std::cout << "New connection accepted" << std::endl;
                        fds.push_back({clientSocket, POLLIN, 0}); // Add the new client socket to the poll set
                        //std::string reply = "test msg";
                        //send(clientSocket, reply.c_str(), reply.length(), 0);
                        send(clientSocket, welcom, sizeof(welcom), 0);
                        send(clientSocket, yourhost, sizeof(yourhost), 0);
                        send(clientSocket, create, sizeof(create), 0);
                    }
                }
                else
                {
                    char buffer[1024];
                    memset(buffer, '\0', 1024 - 1);
                    ssize_t bytesRead = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesRead == -1) {
                        std::cerr << "Error receiving data" << std::endl;
                    }
                    else if (bytesRead == 0) {
                        std::cout << "Connection closed by client" << std::endl;
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        --i;
                    }
                    else {
                        //std::cout << "Received " << bytesRead << " bytes: " << buffer << std::endl;
                        msg += buffer;
                        if (msg.find('\n') != std::string::npos)
                        {
                            std::cout << msg << std::endl;
                            if (msg == "exit\n")
                            {
                                close(serverSocket);
                                return (0);
                            }
                            msg.clear();
                        }
                    }
                }
            }
        }
    }
    close(serverSocket);
    return 0;
}