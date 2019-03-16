#include "pch.h"
#include "ThreadPool.h"


ThreadPool::ThreadPool(int num) :isSuspend(true), isExit(false)
{
	while (num)
	{
		activeThread.emplace_back([this]
		{
			std::unique_lock<std::mutex> lock(this->mtx);
			while (!this->isExit)
			{
				std::function<void()> task;
				this->cond.wait(lock, [this]() {return this->isExit || !this->tasks.empty(); });          // false continue to block
				if (!this->tasks.empty())
				{
					task = std::move(this->tasks.front());     //rvalue reference
					this->tasks.pop();
					task();
				}
			}

		});
		num--;
	}
}

ThreadPool::~ThreadPool()
{
	isExit = true;
	cond.notify_all();
	for (std::thread &t : activeThread)
	{
		t.join();
	}
}

