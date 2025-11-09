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

int64 result = 0;

int64 Calculate() {
	int64 sum = 0;
	for (int64 i = 0; i < 1000000000; ++i) {
		sum += i;
	}

	return sum;
}

void PromiseWorker(std::promise<int64>&& promise) {
	int64 sum = Calculate();
	promise.set_value(sum); // 약속을 지킨다.
}

void TaskWorker(std::packaged_task<int64()>&& task) {
	task(); // 작업 실행
}

int main()
{
	// 동기 실행
	int64 sum = Calculate();
	cout << "Sum: " << sum << endl;

	// std::future를 사용한 비동기 실행 -> 하나의 함수만 잠시 비동기로 실행할때 유용하다
	{
		// 1. std::deferred -> lazy evaluation 지연해서 실행하세요
		// 2. std::async -> 별도의 스	레드에서 즉시 실행하세요 (별도의 스레드가 생성됨. 즉, 자동으로 멀티스레드 환경으로 됨 - 명시적으로 스레드를 생성할 필요 없이 간접적으로 생성 해준다)
		// 3. async | deferred -> 둘중 알아서 실행하세요

		// 언젠가 미래에 계산이 완료될 것이다.
		std::future<int64> future = std::async(std::launch::async, Calculate); // 전용 스레드에서 비동기 호출

		// TODO
		std::future_status status = future.wait_for(1ms);
		int64 sum = future.get();

		class Knight {
		public:
			int64 GetHP() {
				return 100;
			}
		};

		Knight knight;
		std::future<int64> future2 = std::async(std::launch::async, &Knight::GetHP, &knight); // 멤버 함수 비동기 호출 (knight.GetHP()를 호출 하는것과 같다)
	}

	// std::promise를 사용한 비동기 실행 -> 여러개의 작업을 비동기로 실행할때 유용하다
	{
		std::promise<int64> promise;
		std::future<int64> future = promise.get_future();
		std::thread worker(PromiseWorker, std::move(promise)); // 약속을 지키는 작업을 별도의 스레드에서 실행

		// TODO

		int64 sum = future.get(); // 약속이 지켜질 때까지 대기
		worker.join();

	}

	// std::packaged_task를 사용한 비동기 실행 -> 여러개의 작업을 비동기로 실행할때 유용하다
	{
		std::packaged_task<int64()> task(Calculate); // 작업을 포장
		std::future<int64> future = task.get_future();
		std::thread worker(TaskWorker, std::move(task)); // 작업을 별도의 스레드에서 실행
		// TODO
		int64 sum = future.get(); // 작업이 완료될 때까지 대기
		worker.join();
	}

	// 결론: mutex, condition_variable, atomic 등을 직접 사용하기 보다 std::future, std::promise, std::packaged_task, std::async 등을 사용하여 비동기 작업을 처리하는 것이 더 쉽고 안전하다. (결과 값만 받아오는 작업을 할 시에)
	// 특히나 한번 발생하는 단발성 작업에 대해서 유용하다.
	// 1. async: 원하는 함수를 비동기로 실행
	// 2. promise + thread: 여러 작업을 비동기로 실행하고, 각 작업이 완료되었을 때 결과를 약속
	// 3. packaged_task + thread: 여러 작업을 비동기로 실행하고, 각 작업이 완료되었을 때 결과를 포장
}
