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

template<typename T>
class LockFreeStack {
	struct Node;

	struct CountedNodePtr {
		int32 externalCount = 0;
		Node* ptr = nullptr;
	};

	struct Node {
		Node(const T& value) : data(make_shared<T>(value))
		{}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:

	void Push(const T& value) {
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false);
	}

	shared_ptr<T> TryPop() {
		CountedNodePtr oldHead = _head;
	}

private:

	void IncreaseHeadCount(CountedNodePtr& oldCounter) {
		while (true) {
			CountedNodePtr newCounter = oldCounter;
			++newCounter.externalCount;

			if (_head.compare_exchange_strong(oldCounter, newCounter)) {
				break;
			}
		}
	}
	atomic<CountedNodePtr> _head;
};