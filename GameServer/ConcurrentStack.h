#pragma once


#include <mutex>
template<typename T>
class LockStack {
public:
	LockStack(){}

	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value) {
		lock_guard<mutex> lock(_mutex);
		_queue.push(std::move(value));
		_conVar.notify_one();
	}

	bool TryPop(T& value) {
		lock_guard<mutex> lock(_mutex);
		if (_queue.empty()) {
			return false;
		}

		value = std::move(_queue.top());
		_queue.pop();
		return true;
	}

	bool WaitPop(T& value) {
		unique_lock<mutex> lock(_mutex);
		_conVar.wait(lock, [this]() { return !_queue.empty(); });
		value = std::move(_queue.top());
		_queue.pop();
		return true;
	}

private:
	queue<T> _queue;
	mutex _mutex;
	condition_variable _conVar;
};

