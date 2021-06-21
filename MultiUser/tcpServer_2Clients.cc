/*
Author(s): Dan Rehberg
Purpose: Server that can accept multiple clients, and relays messages between clients.
*/

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h> //For single threaded multiple socket management
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <vector>

constexpr int port = 40666;
constexpr int bufferSize = 0x40000;//4096;
constexpr int bufferChunk = 1460;
constexpr int maxClients = 3;

//Maximum message capacity is within the bufferSize!!
//      Therefore, the recv loop is allowed to run until either full message is received or array capacity is reached
int receivingMessage(int initialSize, char* array, int socket)
{
	uint32_t totalSize;
	std::memcpy(&totalSize, array, 4);
	std::cout << "total message size: " << totalSize << '\n';
        //char tempBuffer[bufferChunk];
        int received = 0;
        int delta = totalSize - initialSize;
        while (delta > 0)
        {
                //received = recv(socket, (array + initialSize), ((delta < bufferChunk) ? bufferChunk : delta), 0);
                //received = recv(socket, tempBuffer, bufferChunk, 0);
                received = recv(socket, (array + initialSize), bufferChunk, 0);
                if (received > 0)
                {
                        initialSize += received;
                        delta = totalSize - initialSize;
                        //std::cout << received << " Gathering long message... " << delta << "\n";
                        //if (received < bufferChunk)break;
                }
                else if (received < 0)break;
        }
        if (received < 0) return 0;
        std::cout << "Total Bytes Gathered: " << initialSize << '\n';
        return initialSize;
}

//Maximum message length is constrained to the bufferSize, but message might be shorter than this.
//      Therefore, additional argument exists for message length - but will be clamped to bufferSize at maximum.
int sendingMessage(int totalSize, int initialSize, char* array, int socket)
{
        int sent = 0;
        totalSize = (totalSize > bufferSize) ? bufferSize : totalSize;
        int delta = totalSize - initialSize;
        while (delta > 0)
        {
                sent = send(socket, (array + initialSize), delta, 0);
                if (sent > 0)
                {
                        initialSize += sent;
                        delta = totalSize - initialSize;
                        std::cout << sent << " Sending long message... " << (bufferSize - initialSize) << "\n";
                }
        }
        if (sent < 0) return 0;
        std::cout << "Sent Total Bytes: " << initialSize << '\n';
        return initialSize;
}

int main()
{
	//Create Socket
	int listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening < 0)
	{
		std::cerr << "Failed to create a socket\n";
		return -1;
	}

	//Bind to the port and IP
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);//Get correct endianness
	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);//all zero, bind to any address on this machine
	
	int result = bind(listening, (sockaddr*)&hint, sizeof(hint));
	if (result < 0)
	{
		std::cerr << "Failed to bind to port or IP\n";
		return -2;
	}

	//Make listenging through this socket
	result = listen(listening, SOMAXCONN);//argument two is the maximum number of connections
	if (result < 0)
	{
		std::cerr << "Failed to establish listening socket\n";
		return -3;
	}


	fd_set host, cphost;
	timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	int clientsLeft = maxClients - 1;
	int cur = 1;
	int sockets[maxClients];
	for (unsigned int i = 0; i < maxClients; ++i)
	{
		sockets[i] = -1;
	}
	FD_ZERO(&host);
	FD_SET(listening, &host);
	sockets[0] = listening;
	int highest = listening;
	char message[bufferSize];
	while (clientsLeft > 0)
	{
		cphost = host;
		int connections = select(highest+1, &cphost, NULL, NULL, NULL);//&timeout);
		std::cout << "listening socket is ready\n";
		int temp_cur = cur;
		for (int sock = 0; sock < temp_cur; ++sock)
		{
			int socket = sockets[sock];
			if (FD_ISSET(socket, &cphost))
			{
				if (socket == listening)
				{
					sockaddr_in client;
					socklen_t clientSize;
					char hostName[NI_MAXHOST];
                                        char service[NI_MAXSERV];
					int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
                                        if (clientSocket < 0)
                                        {
                                                std::cerr << "Connection with client failed\n";
                                                return -4;
                                        }
                                        for (unsigned int i = 0; i < NI_MAXHOST; ++i)
                                                hostName[i] = 0;
                                        for (unsigned int i = 0; i < NI_MAXSERV; ++i)
                                                service[i] = 0;

                                        result = getnameinfo((sockaddr*)&client, sizeof(client), hostName, NI_MAXHOST, service, NI_MAXSERV, 0);
                                        if (result < 0)
                                        {
                                                inet_ntop(AF_INET, &client.sin_addr, hostName, NI_MAXHOST);
                                                std::cout << hostName << " connected to service: " << ntohs(client.sin_port) << '\n';
                                        }
                                        else
                                        {
                                                std::cout << hostName << " connected to service: " << service << '\n';
                                        }
					if (cur < maxClients)
					{
                                        	//Add the new client to the array of active sockets on server
						highest = (highest > clientSocket) ? highest : clientSocket;
                                        	sockets[cur++] = clientSocket;
                                        	FD_SET(clientSocket, &host);
					}
					else
					{
						//No free space
						close(clientSocket);
					}
				}
				else
				{
					result = recv(socket, message, bufferSize, 0);
                                        if (result > 0)
                                        {
                                                if (!(result = receivingMessage(result, message, socket)))
                                                {
                                                        std::cerr << "error in iterative receive, closing server\n";
                                                        break;
                                                }
                                                //Verify entire message is obtained, then send it back
                                                for (unsigned int i = 1; i < temp_cur; ++i)
                                                {
                                                        int clientSocket = sockets[i];//host.fd_array[i];
                                                        //if (clientSocket != listening)
                                                        {
                                                                //Echo to all
                                                                if (send(clientSocket, message, result, 0) < 0)
                                                                {
                                                                        std::cerr << "Could not echo message\n";
                                                                }
                                                        }
                                                }
                                        }
                                        else
                                        {
                                                //Remove a client
                                                close(socket);
                                                FD_CLR(socket, &host);
                                                //Deduct counter to end server program
                                                --clientsLeft;
						--cur;
                                        }
				}
			}
		}
	}

	//close all open sockets
	for (unsigned int i = 0; i < maxClients; ++i)
	{
		int socket = sockets[i];//host.fd_array[i];
		if (socket > 0)
			close(socket);
		//FD_CLR(socket, &host);
	}

	return 0;
}
//Older method for single client...
/*
	//Stop listening
	close(listening);//if only accepting one client???
	
	//Make a connection
	for (unsigned int i = 0; i < NI_MAXHOST; ++i)
		host[i] = 0;
	for (unsigned int i = 0; i < NI_MAXSERV; ++i)
		service[i] = 0;
	
	result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0);
	if (result < 0)
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected to service: " << ntohs(client.sin_port) << '\n';
	}
	else
	{
		std::cout << host << " connected to service: " << service << '\n';
	}

	//Start communicating
	char message[bufferSize];
	while (true)
	{
		result = recv(clientSocket, message, bufferSize, 0);
		if (result < 0)
		{
			std::cerr << "Failed to receive message\n";
			break;
		}
		else if (result == 0)
		{
			std::cout << "Client closed connection\n";
			break;
		}
		else
		{
			if (!(result = receivingMessage(result, message, clientSocket)))
			{
				std::cout << "Error during iterative receive\n";
				break;
			}
			std::cout << "Total gathered: " << result << '\n';
		}
		std::cout << "Message: " << std::string(message+4, 0, result) << '\n';
		uint32_t length = static_cast<uint32_t>(result);
		std::memcpy(message, &length, 4);
		send(clientSocket, message, result, 0);
	}
*/
