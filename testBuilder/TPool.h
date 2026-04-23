#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class WatcherThread
{
public:
	WatcherThread();
	~WatcherThread();

	void SetWJob(std::function<void()> job);
private:
	std::atomic<bool> m_hasJob = false;
	std::atomic<bool> m_exit{ false };
	std::mutex m_lock;
	std::condition_variable m_cv;
	std::function <void()> m_job;
	std::thread* m_thread;
};

static WatcherThread watcherThread;

extern WatcherThread* GetWatcherThread();