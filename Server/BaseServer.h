#pragma once
#pragma comment(lib, "ws2_32.lib") //Inform compiler to link the winsock2 library

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

/* ======CONFIGURABLE====== */
#define PORT 502
#define SOCKET_TIMEOUT 60
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
/* ====END CONFIGURABLE==== */

#define IP_ADDRESS_LENGTH 16
#define RESULT_TIMEOUT 0

enum ClientConnectionState
{
	ACTIVE,
	INACTIVE,
	CLOSED
};

typedef struct SendBuffer
{
	byte* buffer;
	int buffer_sz;

} SendBuffer;

typedef struct ClientConnection
{
	int client_id;
	SOCKET client_socket;
	sockaddr_in client_address;
	ClientConnectionState client_state;
	mutex client_state_mutex;

	ClientConnection(SOCKET socket, sockaddr_in address, int id) : client_socket(socket), client_address(address), client_id(id)
	{
		client_state = ACTIVE;
	}

} ClientConnection;

typedef struct ClientConnectionHandler
{
	thread *client_thread;
	ClientConnection* client_connection;

	ClientConnectionHandler(ClientConnection* connection, thread *thread) : client_connection(connection), client_thread(thread) {}

	~ClientConnectionHandler()
	{
		delete client_thread;
		delete client_connection;
	}

} ClientConnectionHandler;

template <typename T>
class BaseServer
{
	public:
		BaseServer();
		void SetPort(unsigned short port);
	 	void Start();

	private:
		WSAData wsa_data;
		WORD version;
		SOCKET server_socket;
		unsigned short server_port;
		unsigned short client_count;
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
		/// Listens for client connections
		/// </summary>
		/// <remarks>
		/// ========================================================
		/// Start separate client thread upon accepting connection.
		/// Close client threads that have finished.
		/// ========================================================
		/// </remarks>
		void Listen();

		/// <summary>
        /// Start new client communication on separate thread.
		/// </summary>
		/// <param name="SOCKET [client_socket]"></param>
		/// <param name="sockaddr_in [client_address]"></param>
		void StartClientThread(SOCKET client_socket, sockaddr_in client_address);

		/// <summary>
		/// Free up resources from clients
		/// that have ended communications
		/// </summary>
		void RemoveInactiveClients();

		/// <summary>
		/// Client communication handler that runs on a separate thread
		/// </summary>
		/// <param name="data"></param>
		void HandleClient(ClientConnection *data);

		bool ReceiveAndRespond(SOCKET socket);

		/// <summary>
		/// Receive request and send response
		/// </summary>
		/// <param name="SOCKET [socket]"></param>
		/// <returns></returns>
		virtual SendBuffer* GetResponse(const char* recv_buffer, int buffer_size_bytes);

		/// <summary>
		/// Get IP Address string representation of sockaddr_in structure
		/// </summary>
		/// <param name="sockaddr_in [ip_address]"></param>
		/// <returns>char* [IP Address]</returns>
		char* GetIPAddress(sockaddr_in ip_address);
};