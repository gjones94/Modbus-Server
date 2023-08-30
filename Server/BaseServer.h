#pragma once
//Inform compiler to link the winsock2 library
#pragma comment(lib, "ws2_32.lib") 


#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <csignal>
#include <mutex>
#include <Windows.h>
#include <ws2tcpip.h> // Include for inet_ntop

using namespace std;

/* ======CONFIGURABLE======*/
#define PORT 502
#define SOCKET_TIMEOUT 60
#define MAX_CLIENTS 30
#define MAX_REQUEST_SIZE 1024
/* ====END CONFIGURABLE====*/

#define MAX_IP_LENGTH 16
#define RESULT_TIMEOUT 0

typedef struct ClientConnectionData
{
	int threadId;
	SOCKET clientSocket;
	sockaddr_in clientAddress;

} ClientConnectionData;

template <typename T>
class BaseServer
{
	public:
		BaseServer();
		void SetPort(unsigned short port);
	 	void Start();

	private:
		WSAData WSAData;
		WORD Version;
		SOCKET ServerSocket;
		unsigned short Port;

		/// <summary>
		/// ClientId, Socket, IP address
		/// </summary>
		ClientConnectionData ClientData;
		/// <summary>
		/// Keeps track of the number of active clients
		/// </summary>
		atomic<int> ClientCount;

		/// <summary>
		/// Initializes socket using WSAData
		/// </summary>
		/// <returns></returns>
		bool InitializeSocket();

		/// <summary>
		/// Binds socket to host IP address
		/// </summary>
		/// <returns></returns>
		bool BindSocket();

		/// <summary>
		/// Starts listening for client connections on continuous loop
		/// </summary>
		void Listen();

		/// <summary>
		/// Client communication handler that runs on a separate thread
		/// </summary>
		/// <param name="data"></param>
		void HandleClient(SOCKET socket);

		/// <summary>
		/// Sends data through the established client socket
		/// </summary>
		/// <param name="clientSocket"></param>
		/// <param name="sendData"></param>
		/// <returns></returns>
		virtual bool Send(SOCKET clientSocket, const T* sendData);

		virtual T GetResponse(char* requestData);
};

