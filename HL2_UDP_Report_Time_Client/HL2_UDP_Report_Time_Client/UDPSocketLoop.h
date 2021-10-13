#pragma once
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <chrono>
#include <fstream>

#pragma  comment (lib, "ws2_32.lib") //Note this pragma is instead of manually including for link

#define PORT 40667
#define BUFFER_SIZE 4
#define SERVER_IP "10.0.0.41" //Public facing IPv4 if on seperate network

std::string attempt(std::string filepath)
{
	std::vector<std::pair<uint32_t, uint64_t>> vect;
	vect.reserve(1000);
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

	//Return information...
	sockaddr_in recServer;//This is the client found within this server side, so client information/metadata
	int recServerLength = sizeof(recServer);
	ZeroMemory(&recServer, recServerLength);

	//Wrtie to socket
	// 	   Example using lovely C++ container
	//std::string message(argv[1]);
	//int sendOK = sendto(out, message.c_str(), message.size() + 1, 0, (sockaddr*)&server, sizeof(server));//use sendto(...) for UDP whereas TCP uses send(...)
	//const char* message = "Hello UDP Server!";
	//char buf[BUFFER_SIZE] = { 0 };
	std::vector<char> receive(BUFFER_SIZE);//char receive[BUFFER_SIZE];

	int cur_len = BUFFER_SIZE;

	sendto(out, &receive[0], BUFFER_SIZE, 0, (sockaddr*)&server, sizeof(server));
	//Loop until all timings collected
	uint32_t count = 1;
	do
	{
		//int sendOK = sendto(out, &message[0], cur_len, 0, (sockaddr*)&server, sizeof(server));

		int bytesIn = recvfrom(out, &receive[0], BUFFER_SIZE, 0, (sockaddr*)&recServer, &recServerLength);
		if (bytesIn > 0)
		{
			std::memcpy(&count, &receive[0], 4);
			vect.push_back(std::pair<uint32_t, uint64_t>(count,
				(uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));
		}

	} while (count > 0);

	std::ofstream outfile(filepath + "/times.csv");

	for (const auto& e : vect)
		outfile << e.first << "," << e.second << "\n";

	//Close socket
	closesocket(out);

	//Close WinSock
	WSACleanup();

	return "Sent a message, but UDP client does not bind; so check server!!";
}