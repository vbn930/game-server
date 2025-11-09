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

// User level object -> 동일한 프로그램 (프로세스) 내에서만 공유
condition_variable cv; // -> 보통 조건 변수를 이용하는 방식을 추천 (linux, windows 둘다 사용)

void Producer() {
	while (true) {
		{
			// 1. 락 잡고
			// 2. 공유 변수 값 수정
			// 3. 락 풀고
			// 4. 조건 변수에 알림
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one(); // wait 중인 스레드 중 하나를 깨움

		// this_thread::sleep_for(chrono::milliseconds(10000));
	}
}

void Consumer() {
	while (true) {
		{
			unique_lock<mutex> lock(m);
			cv.wait(lock, []() {
				return !q.empty();
				});
			// 1. 락 잡으려고 시도
			// 2. 조건 확인
			// - 만족 -> 빠져나옴
			// - 불만족 -> 락을 풀고 대기 상태로 들어감

			// notify_one으로 깨워진거면 조건을 만족 한거 아닌가? -> 아님. 다른 스레드가 먼저 락을 잡고 조건을 변경했을 수도 있음
			// 이러한 경우를 Surprise Wakeup 이라고 함 -> 그래서 조건을 람다로 다시 확인하는 것

			int32 v = q.front();
			q.pop();
			cout << "Consumed: " << v << endl;
		}
	}
}


int main()
{
	thread t1(Producer);	
	thread t2(Consumer);

	t1.join();
	t2.join();
}
