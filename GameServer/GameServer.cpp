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

class TestLock {
	USE_LOCK;


public:
	void TestRead() {
		READ_LOCK;
		if (!_queue.empty()) {
			int32 value = _queue.front();
			cout << "Read value: " << value << " from thread ID: " << LThreadId << endl;
		}
		else {
			cout << "Queue is empty in thread ID: " << LThreadId << endl;
		}
	}

	void TestPush() {
		WRITE_LOCK;
		int32 value = rand() % 100;
		_queue.push(value);
		cout << "Pushed value: " << value << " from thread ID: " << LThreadId << endl;
	}


	void TestPop() {
		WRITE_LOCK;
		if (!_queue.empty()) {
			int32 value = _queue.front();
			_queue.pop();
			cout << "Popped value: " << value << " from thread ID: " << LThreadId << endl;
		}
		else {
			cout << "Queue is empty, cannot pop in thread ID: " << LThreadId << endl;
		}
	}
private:
	queue<int32> _queue;
};

TestLock GTestLock;
void ThreadReadMain() {
	for (int i = 0; i < 5; i++) {
		GTestLock.TestRead();
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

void ThreadPushMain() {
	for (int i = 0; i < 5; i++) {
		GTestLock.TestPush();
		this_thread::sleep_for(chrono::milliseconds(150));
	}
}

void ThreadPopMain() {
	for (int i = 0; i < 5; i++) {
		GTestLock.TestPop();
		this_thread::sleep_for(chrono::milliseconds(200));
	}
}


int main()
{
	
	GThreadManager->Launch(ThreadPushMain);
	GThreadManager->Launch(ThreadReadMain);
	GThreadManager->Launch(ThreadPopMain);

	GThreadManager->Join();
	std::cout << "Hello World from GameServer!" << std::endl;
	return 0;
}
