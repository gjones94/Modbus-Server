#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <chrono>
#include "Server.h"

#define MAX_THREADS 5
using namespace std;

int threadCount = 0;
mutex threadCounterMutex;

void StartConnection(int threadId)
{
	//lock threadcount access
	unique_lock<mutex> lock(threadCounterMutex); //lock 
	
	if (threadCount >= MAX_THREADS)
	{
		printf("Connection failed. There are too many connections present\n");
		lock.unlock();
		return;
	}
	else 
	{
		threadCount++;
		printf("Thread: %d acquired lock\nThreadCount is at %d\n", threadId, threadCount);
	}

	//release lock on threadcount;
	lock.unlock();

	this_thread::sleep_for(chrono::milliseconds(10000));

	//Thread is finished, reduce active thread count
	lock.lock();
	threadCount--;
	lock.unlock();

	printf("Thread: %d finished. ThreadCount is at %d\n", threadId, threadCount);
}

int main(void)
{
	thread threads[10];

	for (int i = 1; i < 10; i++)
	{
		this_thread::sleep_for(chrono::milliseconds(10));
		threads[i] = thread(StartConnection, i);
	}

	for (int i = 1; i < 10; i++)
	{
		threads[i].join();
	}
 
	//Server server("127.0.0.1",502);

	//server.Start();

	return 1;
}


//PRACTICE
//
//struct ServerHandle
//{
//    Server Server;
//    bool Cancelled;
//};
//
//void StartServerThread(void *arguments)
//{
//    ServerHandle *handle = (ServerHandle*) arguments;
//    Server server = handle->Server;
//    bool isCancelled = handle->Cancelled;
//    server.Start();
//}
//
//struct Data
//{
//    int threadId;
//    int *counter;
//    bool *cancel;
//};
//
//void Increment(void *data)
//{
//    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
//
//    Data* objectData = (Data*) data;
//
//    int thisId = (int) objectData->threadId;
//    int *counter = (int*) objectData->counter;
//    bool *cancel = (bool*) objectData->cancel;
//
//    while (*counter < 100)
//    {
//        if (*cancel == true)
//        {
//            printf("Thread %d has been cancelled", thisId);
//            break;
//        }
//
//        (*counter)++;
//		SetConsoleTextAttribute(console, (WORD) thisId);
//        printf("Thread ID: %d | count : %d\n", thisId, *counter);
//        this_thread::sleep_for(chrono::milliseconds(200));
//    }
//}
//
//int main()
//{
//    int counter = 0;
//    bool cancel1 = false;
//    bool cancel2 = false;
//    Data data1;
//    Data data2;
//
//    data1.threadId = 1;
//    data1.counter = &counter;
//    data1.cancel = &cancel1;
//
//    data2.threadId = 2;
//    data2.counter = &counter;
//    data2.cancel = &cancel2;
//
//    thread t1(Increment, &data1);
//    this_thread::sleep_for(chrono::milliseconds(100));
//    thread t2(Increment, &data2);
//
//    this_thread::sleep_for(chrono::milliseconds(3000));
//
//    cancel1 = true; 
//
//    t1.join();
//    t2.join();
//   /* char ipAddress[] = "192.168.1.10";
//    USHORT port = 502;
//    Server server(ipAddress, port);
//    ServerHandle serverHandle;
//
//    serverHandle.Server = server;
//    serverHandle.Cancelled = false;
//
//    thread t1(StartServerThread, &serverHandle);
//
//    t1.join();*/
//}


//Functions as parameters
//typedef int (*CalculationDelegate)(int x, int y);
//
//int calculate(CalculationDelegate func, int x, int y)
//{
//	return func(x, y);
//}
//
//int add(int x, int y)
//{
//	return x + y;
//}
//
//int main(void)
//{
//
//	int value = calculate(add, 3, 2);
//	printf("Value: %d\n", value);
//
//	return 0;
//}

