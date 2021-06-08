#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

constexpr int port = 40666;
constexpr int bufferSize = 4096;

//Maximum message capacity is within the bufferSize!!
//	Therefore, the recv loop is allowed to run until either full message is received or array capacity is reached
int receivingMessage(int initialSize, char* array, int socket)
{
	int received = 0;
	int delta = bufferSize - initialSize;
	while (delta > 0)
	{
		received = recv(socket, (array + initialSize), delta, 0);
		if (received > 0)
		{
			initialSize += received;
			delta = bufferSize - initialSize;
			//std::cout << received << " Gathering long message... " << (bufferSize - initialSize) << "\n";
		}
		else break;
	}
	if (received < 0) return 0;
	//std::cout << "Total Bytes Gathered: " << initialSize << '\n';
	return initialSize;
}

//Maximum message length is constrained to the bufferSize, but message might be shorter than this.
//	Therefore, additional argument exists for message length - but will be clamped to bufferSize at maximum.
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
			//std::cout << sent << " Sending long message... " << (bufferSize - initialSize) << "\n";
		}
	}
	if (sent < 0) return 0;
	//std::cout << "Sent Total Bytes: " << initialSize << '\n';
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
	close(listening);//Only one client in this program
	
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
				std::cerr << "Gathering message failed\n";
				break;
			}
			//std::cout << "Bytes received: " << result << '\n';
		}
		//std::cout << "Message: " << std::string(message, 0, result) << '\n';
		int tempSize = send(clientSocket, message, result, 0);//plus one not needed for the len - will include '/0'
		if (tempSize > 0)
		{
			if(!(tempSize = sendingMessage(result, tempSize, message, clientSocket)))break;
			//std::cout << "sent message size: " << tempSize << "\n";
		}
	}

	//close the socket
	close(clientSocket)
	
	return 0;
}
