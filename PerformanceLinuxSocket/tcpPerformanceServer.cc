#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

constexpr int port = 40666;
constexpr int BUFFER_SIZE = 0X40000;//4096;
constexpr int bufferChunk = 1460;

int receivingMessage(int initialSize, char* array, int socket)
{
	uint32_t totalSize;
	std::memcpy(&totalSize, array, 4);
	//std::cout << "total message size: " << totalSize << '\n';
	int received = 0;
	int delta = totalSize - initialSize;
	while (delta > 0)
	{
		received = recv(socket, (array + initialSize), bufferChunk, 0);
		if (received > 0)
		{
			initialSize += received;
			delta = totalSize - initialSize;
			//std::cout << received << " Gathering long message... " << delta << "\n";
		}
		else if (received < 0)break;
	}
	if (received < 0) return 0;
	//std::cout << "Total Bytes Gathered: " << initialSize << '\n';
	return initialSize;
}
//Might not be an issue
/*
int sendingMessage(int totalSize, int initialSize, char* array, int socket)
{
        int sent = 0;
        totalSize = (totalSize > BUFFER_SIZE) ? BUFFER_SIZE : totalSize;
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
        std::cout << "Sent Total Bytes: " << initialSize << '\n';
        return initialSize;
}
*/

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
	close(listening);//if only accepting one client???
	
	//Make a connection, and gather names of host and client
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

	//Communicate with Client
	char receive[BUFFER_SIZE];//std::vector<char> receive(BUFFER_SIZE);//char receive[BUFFER_SIZE];
	int sendResult;
	int bufferLength = BUFFER_SIZE;

	uint32_t counter = 0;
	//Start communicating
	do {//communicate until client closes connection
		result = recv(clientSocket, receive, bufferLength, 0);
		if (result > 1)
		{
			if (!(result = receivingMessage(result, receive, clientSocket)))
			{
				std::cerr << "Problem while iterating over receive\n";
				break;
			}
			std::cout << "Received char count: " << result << "\n";
			//Echo back
			sendResult = send(clientSocket, receive, result, 0);
			if (sendResult < 0)//== SOCKET_ERROR)
			{
				//WINSOCK2 material commented out below, left in case moved back to Windows
				std::cerr << "Could not echo back\n";//: Last error was " << WSAGetLastError() << "\n";
				//closesocket(client);
				//WSACleanup();
				//return -7;
				break;
			}//else data was echoed back
			//std::cout << "Sent char count: " << sendResult << "\n";
		}
		else if (result == 1)
		{
			sendResult = send(clientSocket, receive, result, 0);
			if (sendResult < 0)//== SOCKET_ERROR)
			{
				//WINSOCK2 material commented out below, left in case moved back to Windows
				std::cerr << "Could not echo back\n";//: Last error was " << WSAGetLastError() << "\n";
				//closesocket(client);
				//WSACleanup();
				//return -7;
				break;
			}
			result = recv(clientSocket, receive, bufferLength, 0);
			if (result > 0)
			{
				if(!(receivingMessage(result, receive, clientSocket)))
				{
					std::cerr << "error in iteration recv\n";
					break;
				}
			}
			result = (result - 4) / 4;//Ignore first four bytes for message length, then every 4 bytes if one uint32_t of time data
			std::cout << "received the performance metrics\n";
			break;
		}
		else if (result == 0)
			std::cout << "Connection will now close\n";
		else
		{
			//WINSOCK2 material commented out below, left in case moved back to Windows
			std::cerr << "Could not receive from client\n";//: Last error was " << WSAGetLastError() << "\n";
			//closesocket(client);
			//WSACleanup();
			//return -8;
			break;
		}

	} while (result > 0);

	//Step A - part 1
	std::vector<uint32_t> results(result);
	for (int i = 1; i < result; i++)
	{
		std::memcpy(&results[i-1], &receive[i * 4], 4);
	}
	//End Step A - part 1
	//If not enough memory on server, comment out step 'A' above and below, and uncomment step 'B'
	//	Two ways of doing the same thing, step 'A' is technically more work to perform...
	std::ofstream file("data.csv");
	if (file)
	{
		uint32_t count = 0;
		for (unsigned int i = 0; i < 10; ++i)
		{
			file << "\nMB_TIME, " << (BUFFER_SIZE >> count) << "\n";
			count += 1;
			for (unsigned int j = i * 100; j < (i * 100 + 100); ++j)
			{
				//Step B
				//uint32_t temp;
				//std::memcpy(&temp, &receive[j*4], 4);
				//file << temp << ',';
				//End Step B
				
				//Step A - part 2
				file << results[j] << ',';
				//End Step A - part 2
			}
		}
	}
	file.close();

	//From recv chunk buffering fix, way to observe blocking vs non-blocking state of recv
	/*
	if (fcntl(clientSocket, F_GETFL) & O_NONBLOCK)
	{
		std::cout << "non-blocking\n";
	}
	else
	{
		std::cout << "blocking\n";
	}*/

	//close the socket
	close(clientSocket);

	return 0;
}
