#include "BaseServer.h"

using namespace std;

template <typename T>
BaseServer<T>::BaseServer()
{
    Port = PORT;
    ClientCount = 0;
    ServerSocket = INVALID_SOCKET;
    Version = MAKEWORD(0, 0);
};

template <typename T>
void BaseServer<T>::Start()
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

template <typename T>
bool BaseServer<T>::InitializeSocket()
{
    //Version of winsock we want to use (v2.2)
	/*
		MAKEWORD takes two bytes, and smashes them to create a 16 bit word
		(In this case 0x0202)
	*/
    Version = MAKEWORD(2, 2);
    int result = WSAStartup(Version, &WSAData);

    if (result != 0)
    {
        cout << "WSA Startup failed\n";
        cout << "WSA Status: " << WSAData.szSystemStatus << endl;
        return false;
    }

    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (ServerSocket == INVALID_SOCKET)
    {
        cout << "Socket initialization failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return false;
    }

    return true;
}

template <typename T>
bool BaseServer<T>::BindSocket()
{
    sockaddr_in serverIPAddress;
    serverIPAddress.sin_family = AF_INET;
    serverIPAddress.sin_addr.s_addr = INADDR_ANY; //Allows connection on EVERY network interface with IP address
    serverIPAddress.sin_port = htons(Port);

    int result = bind(ServerSocket, (SOCKADDR*) &serverIPAddress, sizeof(serverIPAddress));
    if (result == SOCKET_ERROR)
    {
        cout << "Failed to bind socket to IP" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return false;
    }

    return true;
}

template <typename T>
void BaseServer<T>::Listen()
{
    int result = listen(ServerSocket, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        cout << "Failed to enable listening on socket" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return;
    }

	cout << "Listening for incoming connections on port " << Port << " ..." << endl;

    while (true)
    {
        sockaddr_in clientAddress;
        int szClientAddress = sizeof(clientAddress);

        SOCKET clientSocket = accept(ServerSocket, (SOCKADDR*) &clientAddress, &szClientAddress);

        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Failed to accept client request" << endl;
            cout << "Error: " << WSAGetLastError() << endl;
            closesocket(ServerSocket);
            WSACleanup();
            ExitProcess(EXIT_FAILURE);
        }

        int clientCount = ClientCount.load(); //obtain the current value of the thread-safe client counter
        if (clientCount < MAX_CLIENTS)
        {
            /*
				Start new client communication on separate thread
                ---------------------------------------------------
                Since this is a member function that we are spinning off in a 
                thread, we have to use & and this in the thread call
                otherwise, we could just do: thread(HandleClient, clientSocket)
            */
            ClientConnectionData data { clientCount + 1, clientSocket, clientAddress };
            thread new_connection(&BaseServer::HandleClient, this, data);
            new_connection.detach(); //Detach so that the client data doesn't get invalidated on next loop

        }
        else 
        {
            cout << "\nMax Clients Exceeded. Closing last attempted connection...\n";
            /*
				strcpy_s(sendBuffer, "No connections currently available\n");
				send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
            */
            closesocket(clientSocket);
        }
    }

    closesocket(ServerSocket);
    WSACleanup();
}

template <typename T>
void BaseServer<T>::HandleClient(ClientConnectionData data)
{
    bool success;
    T recvData;

    //Increase the count of clients in use
    int clientCount = ClientCount.load();
    ClientCount.store(clientCount + 1);

    int monitorResult;
	while (true)
	{
        fd_set readset; //data structure that represents a set of socket descriptors (array of integers)
        FD_ZERO(&readset); //clear out the set of sockets
        FD_SET(data.clientSocket, &readset); //add specific socket to to the set

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
            cout << "\nTIMEOUT: Client #" << data.threadId << "\n";
            break;
        }

        if (FD_ISSET(data.clientSocket, &readset)) //socket is ready to read data
        {
            success = Receive(data.clientSocket, &recvData);

            if (success == false)
            {
                break;
            }

            T responseData = GenerateResponse(recvData);

            //TODO Maybe use this
            //if responseData == nullptr
            /*
				char response[MAX_BUFFER_SIZE];
				sprintf_s(response, "Message Received\n");
            */
            success = Send(data.clientSocket, responseData);
		
		}
	}
	closesocket(data.clientSocket);

    cout << "\nDISCONNECTED: Client #" << data.threadId << "\n";

	int currentCount = ClientCount.load();
	ClientCount.store(currentCount - 1);
}
template <typename T>
bool BaseServer<T>::Receive(SOCKET clientSocket, T* receiveData)
{
	int bytesReceived = recv(clientSocket, (char *) receiveData, sizeof(receiveData), 0);

	if (bytesReceived <= SOCKET_ERROR)
	{
        return false;
	}

    return true;
}
template <typename T>
T BaseServer<T>::GenerateResponse(T clientRequestData)
{
    return clientRequestData;
}

template <typename T>
bool BaseServer<T>::Send(SOCKET clientSocket, T sendData)
{
    int bytesSent = send(clientSocket, (char*) &sendData, sizeof(sendData), 0);
    if (bytesSent <= SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

//explicitly inform compiler of the instantiations that will be used 
template class BaseServer<int>;