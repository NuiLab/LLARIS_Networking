#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <cstring>
#include <cstdint>

#pragma comment(lib, "Ws2_32.lib")

#define DEF_PORT "40666"
#define BUFFER_SIZE 512
#define BUFFER_CHUNK 1460
#define SERVER_IP "127.0.0.0"


int receivingMessage(int initialSize, char* array, int socket)
{
	uint32_t totalSize;
	std::memcpy(&totalSize, array, 4);
	int received = 0;
	int delta = totalSize - initialSize;
	while (delta > 0)
	{
		
		received = recv(socket, (array + initialSize), BUFFER_CHUNK, 0);
		if (received > 0)
		{
			initialSize += received;
			delta = totalSize - initialSize;
		}
		else if (received < 0)break;
	}
	if (received < 0) return 0;
	return initialSize;
}

std::string attempt()
{
	//Force the dynamic library to load in
	WSADATA wsaData;

	int result;
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		//Communicate->Text = "Could not load Windows Socket DLL: " + result;
		return "failure";
	}

	//Build Socket
	struct addrinfo* resultInfo = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	result = getaddrinfo(SERVER_IP, DEF_PORT, &hints, &resultInfo);
	if (result != 0)
	{
		//Communicate->Text = "Could not get the address information: " + result;
		WSACleanup();
		return "failure";
	}

	SOCKET connectMe = INVALID_SOCKET;
	ptr = resultInfo;
	connectMe = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (connectMe == INVALID_SOCKET)
	{
		//Communicate->Text = "Could not create socket: Last error was " + WSAGetLastError();
		freeaddrinfo(resultInfo);
		WSACleanup();
		return "failure";
	}

	//Connect Socket
	result = connect(connectMe, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		closesocket(connectMe);
		connectMe = INVALID_SOCKET;
	}//Suggestion to try connecting to the next available address
	//	This might be suggesting that the given address already had another client occupying it.
	//	Refer here
	//		https://docs.microsoft.com/en-us/windows/win32/winsock/connecting-to-a-socket

	freeaddrinfo(resultInfo);
	if (connectMe == INVALID_SOCKET)
	{
		//Communicate->Text = "Could not connect to server: Might not be an err, might be occupied address";
		WSACleanup();
		return "failure";
	}

	//Communicate with Server
	char receive[BUFFER_SIZE];
	const char* message = "Hello Server\n";
	int bufferLength = BUFFER_SIZE;

	result = send(connectMe, message, (int)strlen(message), 0);
	if (result == SOCKET_ERROR)
	{
		//Communicate->Text = "Could not send message: Last error was " + WSAGetLastError();
		closesocket(connectMe);
		return "failure";
	}

	//Communicate->Text = "Bytes sent: " + result;

	//	Close the connection when messaging ceases.
	//		This is disconnecting the sending end of the client, which in other circumstances might 
	//			come after the disconnect from host (i.e. in the below when message size == 0)
	result = shutdown(connectMe, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		//Communicate->Text = "Could not shutdown connection request: Last error was " + WSAGetLastError();
		closesocket(connectMe);
		WSACleanup();
		return "failure";
	}
	
	int returnSize = 0;//temporary variable to get the return size
	//	Wait for server to finish closing the connection
	do
	{
		result = recv(connectMe, receive, bufferLength, 0);
		if (result > 0) //Communicate->Text = "Bytes received: " + result;
		{
			if (!(returnSize = receivingMessage(result, receive, connectMe)))
			{
				return "failure to get echo";
			}
			break;
		}
		else if (result == 0);//Communicate->Text = "Connection closed";
		else; //Communicate->Text = "Could not receive message: Last error was " + WSAGetLastError();
	} while (result > 0);

	//Disconnect Client
	closesocket(connectMe);
	WSACleanup();
	receive[returnSize] = '\0';
	return &receive[0];
}
