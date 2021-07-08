#pragma once

#include <vector>
#include <fstream>
#include <chrono>
//#include <string> //Temporary use for message with String container below
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma  comment (lib, "ws2_32.lib") //Note this pragma is instead of manually including for link

#define PORT 40666
#define BUFFER_SIZE 0X8000//512
#define SERVER_IP "10.0.0.41"

constexpr uint32_t SHIFT_ITR = 7;
constexpr uint32_t ITERATIONS = 100;

std::string attempt(std::string filepath)
{
	//Startup Winsock
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &data);
	if (wsOK != 0)
	{
		//std::cout << "WinSock failed to start: " << wsOK << '\n';
		return "Winsock failed to start";
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
	//const char* message = "Hello Server\n";
	std::vector<char> message(BUFFER_SIZE);//char message[BUFFER_SIZE];
	for (uint32_t i = 0; i < (BUFFER_SIZE - 1); ++i)
	{
		//Cycle through the printable ASCII characters
		message[i] = static_cast<char>((i % 95) + 32);
	}
	message[BUFFER_SIZE - 1] = '\0';
	//Information transmission time, there and back again.
	std::chrono::time_point<std::chrono::steady_clock> startTime, endTime;
	uint32_t counter = 0;
	uint32_t array_counter = 0;
	uint32_t dT[SHIFT_ITR][ITERATIONS] = { 0 };
	int cur_len = BUFFER_SIZE;

	//Loop until all timings collected
	do
	{
		startTime = std::chrono::steady_clock::now();
		int sendOK = sendto(out, &message[0], cur_len, 0, (sockaddr*)&server, sizeof(server));


		/*if (sendOK == SOCKET_ERROR)
		{
			std::cout << "Could not send message to server: " << WSAGetLastError() << '\n';
		}*/

		int bytesIn = recvfrom(out, &receive[0], BUFFER_SIZE, 0, (sockaddr*)&recServer, &recServerLength);
		if (bytesIn > 0)//Alternatively, SOCKET_ERROR is -1
		{
			endTime = std::chrono::steady_clock::now();
			if (counter < ITERATIONS)
			{
				dT[array_counter][counter] = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
				counter++;
			}
			else
			{
				if (array_counter < (SHIFT_ITR - 1))
				{
					cur_len >>= 1;
					message.resize(cur_len);
					message.back() = '\0';
					counter = 0;
					array_counter++;
				}
				else
				{
					break;//END THE LOOP
				}
			}
			//std::cout << "bytes received: " << bytesIn << '\n';
			//if (bytesIn < BUFFER_SIZE) receive[bytesIn] = '\0';
			//else receive[BUFFER_SIZE - 1] = '\0';
			//std::cout << "received: " << bytesIn << " bytes in message: " << receive[0] << '\n';
		}
	} while (true);

	int lastCall = sendto(out, &message[0], 1, 0, (sockaddr*)&server, sizeof(server));

	int bytesIn = recvfrom(out, &receive[0], BUFFER_SIZE, 0, (sockaddr*)&recServer, &recServerLength);
	if (bytesIn > 0)
	{
		cur_len = 0;
		message.resize(BUFFER_SIZE);
		//convert the gathered integers to char
		for (uint32_t i = 0; i < SHIFT_ITR; ++i)
		{
			for (uint32_t j = 0; j < ITERATIONS; ++j)
			{
				char temp[4];
				std::memcpy(temp, &dT[i][j], 4);
				message[cur_len++] = temp[0];
				message[cur_len++] = temp[1];
				message[cur_len++] = temp[2];
				message[cur_len++] = temp[3];
			}
		}
		int sendOK = sendto(out, &message[0], cur_len, 0, (sockaddr*)&server, sizeof(server));
	}
	//Close socket
	closesocket(out);

	//Close WinSock
	WSACleanup();

	//std::cout << "counter: " << counter << '\n';
	std::ofstream file(filepath + "\\data.csv", std::ios::out);
	if (file)
	{
		for (uint32_t i = 0; i < SHIFT_ITR; ++i)
		{
			file << "\nMB_TIME, " << (BUFFER_SIZE >> i) << "\n";
			for (uint32_t j = 0; j < ITERATIONS; ++j)
			{
				//std::cout << "time[" << i << "]: " << dT[i] << '\n';
				file << dT[i][j] << ',';
			}
		}
	}
	else return "file not created!\n";

	return filepath;
}