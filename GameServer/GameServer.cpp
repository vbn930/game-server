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
#include "Allocator.h"



int main()
{
	vector<int32, STLAllocator<int32>> v(100);

}
