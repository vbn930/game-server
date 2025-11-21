#include "pch.h"
#include "DeadLockProfiler.h"


/*=============
* Dead Lock Profiler
==============*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	// 아이디를 찾거나 발급
	int32 lockId = 0;
	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end()) {
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else {
		lockId = findIt->second;
	}

	// 락을 이미 잡고 있다면
	if (_lockStack.empty() == false) {

		// 기존에 발견되지 않은 락이라면 데드락 체크를 위해 기록
		const int32 prevId = _lockStack.top();
		if (lockId != prevId) {
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) {
				history.insert(lockId);
				// 데드락 체크
				CheckCycle();
			}
		}
	}

	_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);
	if (_lockStack.empty()) {
		CRASH("DeadLockProfiler::PopLock Lock Stack Empty");
	}

	int32 lockId = _nameToId[name];
	if (_lockStack.top() != lockId) {
		CRASH("DeadLockProfiler::PopLock Lock Mismatch");
	}

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	// 초기화
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);
	_discoveredCount = 0;

	// 모든 노드에 대해 DFS 수행
	for (int32 i = 0; i < lockCount; i++) {
		Dfs(i);
	}

	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] == -1) {
		return;
	}

	_discoveredOrder[here] = _discoveredCount++;

	// 모든 인접한 정점 순회
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end()) {
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet) {
		// 아직 발견되지 않은 정점이라면
		if (_discoveredOrder[there] == -1) {
			_parent[there] = here;
			Dfs(there);
		}
		
		// here가 there 보다 먼저 발견 되었다면, 순방향 간선
		if (_discoveredOrder[here] < _discoveredOrder[there]) {
			continue;
		}

		// 순방향이 아니고 there가 아직 종료되지 않았다면, 사이클 발견 - 역방향 간선
		if (_finished[there] == false) {
			printf("Dead Lock Detected Cycle:\n");
			printf(" %s -> %s\n", _idToName[here], _idToName[there]);
			int32 p = here;
			while (p != there) {
				printf(" %s -> %s\n", _idToName[_parent[p]], _idToName[p]);
				p = _parent[p];
			}

			CRASH("DeadLockProfiler::Dfs Dead Lock Detected");
		}
	}

	_finished[here] = true;
}
