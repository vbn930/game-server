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


int32 buffer[10000][10000];

int main()
{
	memset(buffer, 0, sizeof(buffer));
	
	{ // Row-major order
		uint64 start = GetTickCount64();
		int64 sum = 0;
		for (int32 i = 0; i < 10000; ++i) {
			for (int32 j = 0; j < 10000; ++j) {
				sum += buffer[i][j];
			}
		}
		uint64 end = GetTickCount64();
		std::cout << "Sum: " << sum << ", Time taken (row-major): " << (end - start) << " ms" << std::endl;
	}

	{ // Column-major order
		uint64 start = GetTickCount64();
		int64 sum = 0;
		for (int32 i = 0; i < 10000; ++i) {
			for (int32 j = 0; j < 10000; ++j) {
				sum += buffer[j][i];
			}
		}
		uint64 end = GetTickCount64();
		std::cout << "Sum: " << sum << ", Time taken (row-major): " << (end - start) << " ms" << std::endl;
	}

	// 결론: 행 우선 순회가 열 우선 순회보다 훨씬 빠르다.
	// 캐시의 지역성 덕분이다.
	// 기본적으로 2차원 배열은 메모리 공간에 행 단위로 연속적으로 저장된다.
	// 따라서 행 우선 순회는 메모리 접근 패턴이 캐시 라인과 일치하여 캐시 적중률이 높아진다.
}
