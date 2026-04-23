#include "Watcher.h"

WatcherThread::WatcherThread() {
	m_thread = new std::thread([this] {
		while (true) {
			std::function<void()> job;

			if (m_exit) {
				return;
			}

			std::unique_lock<std::mutex> lock(m_lock);

			m_cv.wait(lock, [this] {
				return m_hasJob || m_exit;
				});

			m_job();
		}
		});
}

WatcherThread::~WatcherThread() {
	std::unique_lock<std::mutex> lock(m_lock);
	m_exit = true;
	lock.unlock();

	m_cv.notify_all();

	m_thread->join();

	delete m_thread;
}

void WatcherThread::SetWJob(std::function<void()> job) {
	std::unique_lock<std::mutex> lock(m_lock);
	m_job = std::move(job);
	m_hasJob = true;
	lock.unlock();
	m_cv.notify_one();
}

WatcherThread* GetWatcherThread() {
	return &watcherThread;
}