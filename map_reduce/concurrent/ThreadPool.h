#ifndef MAP_REDUCE_CONCURRENT_THREAD_POOL_H_
#define MAP_REDUCE_CONCURRENT_THREAD_POOL_H_

#include <vector>
#include <queue>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace mr::concurrent::detail {
	class ThreadPool {
		using Task = std::function<void()>;
	private:
		void worker() {
			while (!done) {
				Task task;
				pop_task(task);
				if (done) return;
				task();
			}
		}

		void push_task(Task&& task) {
			{
				std::lock_guard<std::mutex> lock(mtx);
				task_queue.push(std::move(task));
			}
			cv.notify_one();
		}

		void pop_task(Task& task) {
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [this] {return !task_queue.empty() || done; });
			if (done) return;
			task = std::move(task_queue.front());
			task_queue.pop();
		}
	private:
		std::atomic_bool done = false;
		std::vector<std::thread> threads;
		std::queue<Task> task_queue;
		std::mutex mtx; // Used to protect the task queue from race condition
		std::condition_variable cv; // Used to sleep threads waiting on available task in the queue
	};
}

#endif // !MAP_REDUCE_CONCURRENT_THREAD_POOL_H_
