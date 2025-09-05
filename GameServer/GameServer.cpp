// GameServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

mutex m;
queue<int32> q;
HANDLE handle;

void Producer() {
	while (true) {
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		::SetEvent(handle); // 이벤트 신호 상태로 변경
		this_thread::sleep_for(chrono::milliseconds(10000));
	}
}

void Consumer() {
	while (true) {
		{
			// 이벤트가 발생 하는 경우에만 해당 스레드를 깨워서 행동을 실행
			::WaitForSingleObject(handle, INFINITE); // 이벤트가 신호 상태가 될 때까지 대기
			// 이벤트가 신호 상태가 되면 자동으로 리셋됨
			// Manual-Reset 이벤트는 수동으로 리셋해야 함
			// ::ResetEvent(handle); // 수동 리셋
			unique_lock<mutex> lock(m);
			if (!q.empty()) {
				int32 v = q.front();
				q.pop();
				cout << "Consumed: " << v << endl;
			}
		}
	}
}


int main()
{
	// 커널 오브젝트 -> 커널에서 관리하는 객체
	// Usage count
	// Singnal, None-Singnal
	// Auto -Reset, Manual-Reset
	handle = ::CreateEvent(NULL /*보안 속성*/, FALSE /*리셋 방법*/, FALSE /*초기 시그널 상태*/, NULL);
	thread t1(Producer);	
	thread t2(Consumer);

	t1.join();
	t2.join();

	::CloseHandle(handle);
}
