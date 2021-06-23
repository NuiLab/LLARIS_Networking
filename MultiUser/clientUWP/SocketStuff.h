#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <thread>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

#define DEF_PORT "40666"
#define BUFFER_SIZE 0x40000
#define BUFFER_CHUNK 1460
#define SERVER_IP "143.110.147.50"




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

class ClientTCP
{
	public:
		ClientTCP() : receiveMessage(BUFFER_SIZE) {}
		~ClientTCP() { closeConnection(); }

		//Start connection
		std::string openConnection()
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
				freeaddrinfo(resultInfo);
				closeConnection();
				return "connection failure";
			}//Suggestion to try connecting to the next available address
			//	This might be suggesting that the given address already had another client occupying it.
			//	Refer here
			//		https://docs.microsoft.com/en-us/windows/win32/winsock/connecting-to-a-socket

			freeaddrinfo(resultInfo);
			if (connectMe == INVALID_SOCKET)
			{
				closeConnection();
				return "failure";
			}

			//Set a timeout length for recv calls
			//Manual:
			//	https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-setsockopt
			int timeout = 5000;
			setsockopt(connectMe, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(int));

			return "";//Zero length string to notify of success
		}

		//close connection
		std::string closeConnection()
		{
			if (connectMe != INVALID_SOCKET)
			{
				//	Close the connection when messaging ceases.
				//		This is disconnecting the sending end of the client, which in other circumstances might 
				//			come after the disconnect from host (i.e. in the below when message size == 0)
				/*int result = shutdown(connectMe, SD_SEND);
				//Failure or not, resest the Socket to being "invalid"
				connectMe = INVALID_SOCKET;
				if (result == SOCKET_ERROR)
				{
					//Communicate->Text = "Could not shutdown connection request: Last error was " + WSAGetLastError();
					closesocket(connectMe);
					WSACleanup();
					return "Shutdown Failure, BUT Socket Closed";
				}*/
				closesocket(connectMe);
				connectMe = INVALID_SOCKET;
				WSACleanup();
				return "Closed Socket";
			}
			return "Socket Already Closed";
		}

		//use connection
		std::string receiveConnection(std::chrono::time_point<std::chrono::steady_clock> &lastUpdate)
		{
			if (connectMe != INVALID_SOCKET)
			{
				std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();
				int result = recv(connectMe, &receiveMessage[0], BUFFER_CHUNK, 0);
				if (result > 0)
				{
					/*if (!(result = receivingMessage(result, &receiveMessage[0], connectMe)))
					{
						return "Failure to RECEIVE";
					}*/
					lastUpdate = std::chrono::steady_clock::now();
				}
				else if (result == 0)
				{
					if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime).count() > 4)
					{
						//This zero message is likely from a timeout
						return "\b";
					}
					else
					{
						closeConnection();
						return std::string();//zero length string to identify socket not open
					}
				}
				else
				{
					//closeConnection();
					return std::string();//problem, so just return zero length
				}
				std::string temp(receiveMessage.begin(), receiveMessage.begin() + result);
				temp.push_back('\0');//precaution
				return temp;
			}
			return std::string();//zero length string to identify that socket is not open
		}

		std::string sendConnection(std::string &message)
		{
			if (connectMe != INVALID_SOCKET)
			{
				std::vector<char> msg(message.size() + 4);
				//message size is the length of the actual message plus the 4 bytes of the 32 bit uint
				//	therefore, just use the above vectors size
				uint32_t length = static_cast<uint32_t>(msg.size());
				std::memcpy(&msg[0], &length, 4);
				msg.insert(msg.begin() + 4, message.begin(), message.end());

				int result = send(connectMe, &msg[0], static_cast<int>(length), 0);// (int)strlen(message), 0);
				if (result == SOCKET_ERROR)
				{
					closeConnection();
					return std::string();
				}
				return "Message Sent";
			}
			return std::string();
		}

	private:
		SOCKET connectMe = INVALID_SOCKET;
		std::vector<char> receiveMessage;
};






