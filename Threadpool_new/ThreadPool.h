#pragma once

#include <future>
#include <list>
#include <queue>
#include <stack>
#include <atomic>
#include <mutex>
#include <condition_variable>

class ThreadPool
{
public:
	ThreadPool(int num);
	~ThreadPool();

	template <typename T,typename ...Args>
	auto commit(T&& task, Args&& ...args)->std::future<typename std::result_of<T(Args...)>::type>;   //decltype(task(args...))

	//std::atomic_bool getIsExit();
protected:
	
private:
	std::list<std::thread> activeThread;
	std::queue<std::function<void()>> tasks;
	//std::stack<std::thread> idleThread;
	std::mutex mtx;
	std::condition_variable cond;
	std::atomic_bool isSuspend;
	std::atomic_bool isExit;
};

template<typename T, typename ...Args>
inline auto ThreadPool::commit(T && task, Args && ...args) -> std::future<typename std::result_of<T(Args ...)>::type>
{
	auto t = std::make_shared<std::packaged_task<typename std::result_of<T(Args...)>::type()>>(std::bind(std::forward<T>(task), std::forward<Args>(args)...));
	std::future<typename std::result_of<T(Args...)>::type> res = t->get_future();
	mtx.lock();
	tasks.emplace([t]() {(*t)(); });
	mtx.unlock();
	cond.notify_one();
	return res;
}
