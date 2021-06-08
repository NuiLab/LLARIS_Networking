#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

constexpr int port = 40666;
constexpr int bufferSize = 4096;

bool receivingMessage(int initialSize, char* array, int socket)
{
	int received = 0;
	while (true)
	{
		received = recv(socket, (array + initialSize), (bufferSize - initialSize), 0);
		if (received > 0)
		{
			initialSize += received;
			//std::cout << received << " Gathering long message... " << (bufferSize - initialSize) << "\n";
		}
		else break;
	}
	if (received < 0) return false;
	//std::cout << "Total Bytes Gathered: " << initialSize << '\n';
	return true;
}

bool sendingMessage(int initialSize, char* array, int socket)
{
	int sent = 0;
	while (bufferSize-initialSize > 0)
	{
		sent = send(socket, (array + initialSize), (bufferSize - initialSize), 0);
		if (sent > 0)
		{
			initialSize += sent;
			//std::cout << sent << " Sending long message... " << (bufferSize - initialSize) << "\n";
		}
	}
	if (sent < 0) return false;
	//std::cout << "Sent Total Bytes: " << initialSize << '\n';
	return true;
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
		std::cerr << "Failed to establish listening in this socket\n";
		return -3;
	}

	//Answer calling client
	sockaddr_in client;
	socklen_t clientSize;
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket < 0)
	{
		std::cerr << "Connection with client failed\n";
		return -4;
	}

	//Stop listening
	close(listening);
	
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
		//std::cout << "Message: " << std::string(message, 0, result) << '\n';
		int tempSize = send(clientSocket, message, result, 0);//plus one not needed for the len - will include '/0'
		if (tempSize > 0)
		{
			if(!sendingMessage(tempSize, message, clientSocket))break;
			std::cout << "sent message size: " << tempSize << "\n";
		}
	}

	//close the socket
	close(clientSocket)
	
	return 0;
}
