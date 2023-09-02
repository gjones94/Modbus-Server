#pragma once
//Inform compiler to link the winsock2 library
#pragma comment(lib, "ws2_32.lib") 

#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <csignal>
#include <mutex>
#include <Windows.h>
#include <ws2tcpip.h> // Include for inet_ntop

using namespace std;

/* ======CONFIGURABLE======*/
#define PORT 502
#define SOCKET_TIMEOUT 60
#define MAX_CLIENTS 30
#define MAX_BUFFER_SIZE 1024
/* ====END CONFIGURABLE====*/

#define MAX_IP_LENGTH 16
#define RESULT_TIMEOUT 0

const char CLIENT_DISCONNECT[] = "Exit";

typedef struct ClientConnection
{
	SOCKET client_socket;
	sockaddr_in client_address;
	bool is_finished;
	mutex finish_lock;

	ClientConnection(SOCKET socket, sockaddr_in address) : client_socket(socket), client_address(address) 
	{
		is_finished = false;
	}

	~ClientConnection()
	{
		cout << "Client Connection Deconstructed" << endl;
	}

} ClientConnection;

typedef struct ClientConnectionHandler
{
	thread *client_thread;
	ClientConnection* client_connection;

	ClientConnectionHandler(ClientConnection* connection, thread *thread) : client_connection(connection), client_thread(thread) {}

	~ClientConnectionHandler()
	{
		cout << "Client handler deconstructed!" << endl;
	}

} ClientConnectionHandler;

template <typename T>
class BaseServer
{
	public:
		BaseServer();
	 	void Start();

	private:
		WSAData wsa_data;
		WORD version;
		SOCKET server_socket;
		unsigned short port;
		unsigned short thread_count;
		vector<ClientConnectionHandler*> client_connection_handlers;

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
		void HandleClient(ClientConnection *data);
		
		/// <summary>
		/// Receive request and send response
		/// </summary>
		/// <param name="socket"></param>
		/// <returns></returns>
		virtual bool ReceiveAndRespond(SOCKET socket);

		virtual void PrintClientCount();

		//bool Send(SOCKET clientSocket, T sendData);

		//bool Receive(SOCKET clientSocket, T *receiveData);

		//T GenerateResponse(T clientRequestData);
};

