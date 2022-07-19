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
		static const size_t DEFAULT_THREAD_COUNT = 4;
	public:
		ThreadPool(size_t count = 0) {
			const size_t hc = std::thread::hardware_concurrency();
			const size_t thread_count = count ? count :
				hc ? hc : DEFAULT_THREAD_COUNT;
			try {
				threads.reserve(thread_count);
				for (size_t i = 0; i < thread_count; ++i) {
					threads.emplace_back(&ThreadPool::worker, this);
				}
			} catch (...) {
				done = true;
				cv.notify_all();
				throw std::current_exception();
			}
		}

		~ThreadPool() {
			done = true;
			cv.notify_all();
			for (auto& thread : threads) {
				if (thread.joinable()) {
					thread.join();
				}
			}
		}
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
