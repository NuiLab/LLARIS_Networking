#pragma once
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <utility>

#pragma  comment (lib, "ws2_32.lib") //Note this pragma is instead of manually including for link

#define PORT 40667
#define BUFFER_SIZE 4//0X8000//512
#define CLIENT_META_SIZE 256
#define MAX_CLIENTS 3

uint32_t counter = 1;
SOCKET in;//The socket created by this host for others to connect to
std::vector<std::string> clientAddresses;
std::vector<std::pair<sockaddr_in, int>> clients;


std::string connectUDP()
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int wsOK = WSAStartup(version, &data);
	if (wsOK != 0)
	{
		//std::cout << "WinSock failed to start: " << wsOK << '\n';
		return "failed to connect";// 1;
	}

	//Create and Bind the socket to ip address and port
	in = socket(AF_INET, SOCK_DGRAM, 0);//Just some unsigned integer much like File Descriptor
	sockaddr_in serverHint;//Note, AF_INET above is version 4 of IP
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(PORT); //Convert from little to big endian

		//Now bind
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		//std::cout << "Binding to Socket failed: " << WSAGetLastError() << '\n';
		return "failed to connect";//return 2;
	}

	

	int resultCount = 0;
	/*while (resultCount < MAX_CLIENTS)
	{
		
	}*/
	return "Got all clients";
}

std::string gatherClient()
{
	//This is the client found within this server side, so client information/metadata
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);

	std::vector<char> buf(BUFFER_SIZE);
	while (true)
	{
		int bytesIn = recvfrom(in, &buf[0], BUFFER_SIZE, 0, (sockaddr*)&client, &clientLength);//this is counter to using the recv for TCP, but otherwise same
		if (bytesIn == SOCKET_ERROR)
		{
			//std::cout << "Could not receive messasge from client: " << WSAGetLastError() << '\n';
			//std::cout << "Trying again...\n";
			continue;//try again in loop...
		}
		//else if (bytesIn > 0)
		{
			char clientIP[256];
			ZeroMemory(clientIP, 256);
			inet_ntop(AF_INET, &client.sin_addr, clientIP, 256);
			std::string temp(clientIP);

			bool add = true;
			for (unsigned int i = 0; i < clientAddresses.size(); ++i)
			{
				if (clientAddresses[i] == temp)add = false;
			}

			if (add)
			{
				clients.push_back(std::pair<sockaddr_in, int>(client, sizeof(client)));
				break;
			}
		}
	}
	return "Got " + std::to_string(clients.size()) + " currently";
}

std::string sendInteger()
{
	std::vector<char> buf(BUFFER_SIZE);//char buf[BUFFER_SIZE];//Buffer where message is received
	std::memcpy(&buf[0], &counter, 4);
	counter++;
	for (unsigned int i = 0; i < clients.size(); ++i)
	{
		int bytesOut = sendto(in, &buf[0], BUFFER_SIZE, 0, (sockaddr*)&clients[i].first, clients[i].second);
		if (bytesOut < 0)
		{
			//std::cout << "Failed to send back a message: " << WSAGetLastError() << '\n';
			return "BobLobLaw";//return 3;
		}
	}
	return "Sent Int\n";
}

std::string sendZero()
{
	counter = 0;
	std::vector<char> buf(BUFFER_SIZE);//char buf[BUFFER_SIZE];//Buffer where message is received
	std::memcpy(&buf[0], &counter, 4);
	counter++;
	for (unsigned int j = 0; j < 3; ++j)
	{
		for (unsigned int i = 0; i < clients.size(); ++i)
		{
			int bytesOut = sendto(in, &buf[0], BUFFER_SIZE, 0, (sockaddr*)&clients[i].first, clients[i].second);
			if (bytesOut < 0)
			{
				//std::cout << "Failed to send back a message: " << WSAGetLastError() << '\n';
				return "failed to connect";//return 3;
			}
		}
	}
	//Close Socket
	closesocket(in);

	//Shutdown winsock
	WSACleanup();
	return "Sent Zero\n";
}