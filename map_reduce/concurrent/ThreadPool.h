#ifndef MAP_REDUCE_CONCURRENT_THREAD_POOL_H_
#define MAP_REDUCE_CONCURRENT_THREAD_POOL_H_

#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mr::concurrent::details {
	class ThreadPool {
		using Task = std::function<void()>;
	private:
		std::atomic_bool done = false;
		std::vector<std::thread> threads;
		std::queue<Task> task_queue;
		std::mutex mtx; // Used to protect the task queue from race condition
		std::condition_variable cv; // Used to sleep threads waiting on available task in the queue
	};
}

#endif // !MAP_REDUCE_CONCURRENT_THREAD_POOL_H_
