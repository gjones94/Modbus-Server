#include "BaseServer.h"
#include <random>

using namespace std;

template <typename T> BaseServer<T>::BaseServer()
{
    port = PORT;
    client_count = 0;
    server_socket = INVALID_SOCKET;
    version = MAKEWORD(0, 0);
};

template <typename T> void BaseServer<T>::Start()
{
    bool success = InitializeSocket();

    if (success)
    {
        success = BindSocket();
    }

    if (success)
    {
		Listen();
    }
}

template <typename T> bool BaseServer<T>::InitializeSocket()
{
    //Version of winsock we want to use (v2.2)
	/*
		MAKEWORD takes two bytes, and smashes them to create a 16 bit word
		(In this case 0x0202)
	*/
    version = MAKEWORD(2, 2);
    int result = WSAStartup(version, &wsa_data);

    if (result != 0)
    {
        cout << "WSA Startup failed\n";
        cout << "WSA Status: " << wsa_data.szSystemStatus << endl;
        return false;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET)
    {
        cout << "Socket initialization failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return false;
    }

    return true;
}

template <typename T> bool BaseServer<T>::BindSocket()
{
    sockaddr_in serverIPAddress;
    serverIPAddress.sin_family = AF_INET;
    serverIPAddress.sin_addr.s_addr = INADDR_ANY; //Allows connection on EVERY network interface with IP address
    serverIPAddress.sin_port = htons(port);

    int result = bind(server_socket, (SOCKADDR*) &serverIPAddress, sizeof(serverIPAddress));
    if (result == SOCKET_ERROR)
    {
        cout << "Failed to bind socket to IP" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        return false;
    }

    return true;
}

template <typename T> void BaseServer<T>::Listen()
{
    u_long mode = 1; //non blocking
    ioctlsocket(server_socket, FIONBIO, &mode);
    int result = listen(server_socket, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        cout << "Failed to enable listening on socket" << endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

	cout << "Listening for incoming connections on port " << port << " ..." << endl;
    PrintClientCount();

    while (true)
    {
        sockaddr_in clientAddress;
        int szClientAddress = sizeof(clientAddress);

        //Non-blocking
        SOCKET clientSocket = accept(server_socket, (SOCKADDR*) &clientAddress, &szClientAddress);

        if (clientSocket != SOCKET_ERROR)
        {
			if (client_count < MAX_CLIENTS)
			{
                StartClientThread(clientSocket, clientAddress);
			}
			else 
			{
				cout << "\nCONNECTION ERROR: Max clients exceeded. Closing last attempted connection...\n";
				closesocket(clientSocket);
			}
        }

        RemoveInactiveClients();

        this_thread::sleep_for(chrono::milliseconds(100));

        //TODO - add watch for SIG_TERM cancel from user, cancel and join all active client threads

    }

    closesocket(server_socket);
    WSACleanup();
}

template <typename T> void BaseServer<T>::StartClientThread(SOCKET client_socket, sockaddr_in client_address)
{
	/*
		NOTE: Use pointers for the client_connection and client_thread.
		Otherwise the stack variables will be destroyed and invalidate the references we passed to the connection handler
	*/
    ClientConnection* client_connection = new ClientConnection(client_socket, client_address, 1);
    thread* client_thread = new thread(&BaseServer::HandleClient, this, client_connection);

    //Add handler for client thread to allow the main thread to monitor the client
    client_connection_handlers.push_back(new ClientConnectionHandler(client_connection, client_thread));
    client_count++;
}

template <typename T> void BaseServer<T>::HandleClient(ClientConnection *connection)
{
    bool success;
    int monitorResult;

    thread::id thread_id = this_thread::get_id();
    char* client_ip_address = GetIPAddress(connection->client_address);

    cout << "CONNECTED [" << client_ip_address << "]" << " THREAD [" << thread_id << "]" << endl;

	while (true)
	{
        fd_set readset; //data structure that represents a set of socket descriptors (array of integers)
        FD_ZERO(&readset); //clear out the set of sockets
        FD_SET(connection->client_socket, &readset); //add specific socket to to the set

        timeval timeout;
        timeout.tv_sec = SOCKET_TIMEOUT; 
        timeout.tv_usec = 0;
        monitorResult = select(0, &readset, nullptr, nullptr, &timeout);

        if (monitorResult == SOCKET_ERROR)
        {
            cout << "\nERROR: Error occured while waiting for socket select monitoring\n";
            return;
        }
        else if (monitorResult == RESULT_TIMEOUT)
        {
			unique_lock<mutex> lock(connection->client_state_mutex); //this will auto release once function completes and lock is out of scope
            connection->client_state = INACTIVE;
            break;
        }

        //socket is ready to read data
        if (FD_ISSET(connection->client_socket, &readset)) 
        {
            success = ReceiveAndRespond(connection->client_socket);
            if (success)
            {
                cout << "COMMUNICATION SUCCESS : [" << thread_id << "]" << endl;
            }
            if (success == false)
            {
				unique_lock<mutex> lock(connection->client_state_mutex); //this will auto release once function completes and lock is out of scope
				connection->client_state = CLOSED;
                break;
            }
		}
	}

    cout << "DISCONNECTED [" << client_ip_address << "]" << " THREAD[" << thread_id << "]" << endl;
	closesocket(connection->client_socket);
}

template <typename T> void BaseServer<T>::RemoveInactiveClients()
{
    //Iterate through the client connection handlers
    for (vector<ClientConnectionHandler*>::iterator client_handler = client_connection_handlers.begin(); client_handler != client_connection_handlers.end();)
    {
		//lock client_state since client thread has direct access to it
        unique_lock<mutex> client_state_lock((*client_handler)->client_connection->client_state_mutex); 

		// make a copy of the client state
        ClientConnectionState client_state = (*client_handler)->client_connection->client_state; 

        client_state_lock.unlock();

        if (client_state == INACTIVE || client_state == CLOSED)
        {
			//make sure thread is not detached or already joined
            bool joinable = (*client_handler)->client_thread->joinable(); 
            if (joinable)
            {
				(*client_handler)->client_thread->join();
            }

            client_count--;

            //get the next valid iterator after removing from the list
            client_handler = client_connection_handlers.erase(client_handler);
        }
        else
        {
            //otherwise go to next client handler
            client_handler++;
        }
    }
}

template <typename T> bool BaseServer<T>::ReceiveAndRespond(SOCKET socket)
{
    char buffer[BUFFER_SIZE];

    int bytesReceived = recv(socket, (char*) buffer, BUFFER_SIZE, 0);
    if (bytesReceived <= SOCKET_ERROR)
    {
        return false;
    }

    int bytesSent = send(socket, (char*) buffer, BUFFER_SIZE, 0);
    if (bytesSent <= SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

template<typename T>
char* BaseServer<T>::GetIPAddress(sockaddr_in ip_address)
{
	char *ip_address_str = new char[IP_ADDRESS_LENGTH];

	if (inet_ntop(AF_INET, &ip_address.sin_addr, ip_address_str, IP_ADDRESS_LENGTH))
	{
		return ip_address_str;
	}

    return nullptr;
}

template <typename T> void BaseServer<T>::PrintClientCount()
{
    cout << endl;
    cout << "====================================" << endl;
    cout << "Active Clients: " << client_count << endl;
    cout << "====================================" << endl;
    cout << endl;
}

//explicitly inform compiler of the instantiations that will be used 
template class BaseServer<char*>;