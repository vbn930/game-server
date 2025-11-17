// GameServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

#include <future>

#include "ThreadManager.h"

CoreGlobal Core;

void ThreadMain() {
	cout << "Hello from thread ID: " << LThreadId << endl;
	this_thread::sleep_for(chrono::seconds(1));
}

int main()
{
	for (uint32 i = 0; i < 5; i++) {
		GThreadManager->Launch(ThreadMain);
	}

	GThreadManager->Join();
	std::cout << "Hello World from GameServer!" << std::endl;
	return 0;
}
