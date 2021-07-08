#pragma once
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma  comment (lib, "ws2_32.lib") //Note this pragma is instead of manually including for link

#define PORT 40666
#define BUFFER_SIZE 512
#define SERVER_IP "10.0.0.41"

std::string attempt()
{
	//Startup Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &data);
	if (wsOK != 0)
	{
		return "WinSock failed to start";
	}

	//Create hint structure for the server
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

	//Socket create
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);//Look Ma, no binding

	//Write to socket
	// 	   Example using C++ container
	//std::string message("Hello UDP Server");
	//int sendOK = sendto(out, message.c_str(), message.size() + 1, 0, (sockaddr*)&server, sizeof(server));//use sendto(...) for UDP whereas TCP uses send(...)
	//Plus one acceptable for the c-string size below as it, by default, ends with terminating null character
	const char* message = "Hello UDP Server!";
	int sendOK = sendto(out, message, (int)strlen(message) + 1, 0, (sockaddr*)&server, sizeof(server));


	if (sendOK == SOCKET_ERROR)
	{
		return "Could not send message to server";
	}

	//Close socket
	closesocket(out);

	//Close WinSock
	WSACleanup();

	return "Sent a message, but UDP client does not bind; so check server!!";
}