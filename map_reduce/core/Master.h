#ifndef MAP_REDUCE_CORE_MASTER_H_
#define MAP_REDUCE_CORE_MASTER_H_

#include "concurrent/ThreadPool.h"
#include "Configuration.h"

namespace mr::core::detail {
	class Master {
		using ThreadPool = mr::concurrent::detail::ThreadPool;
	public:
		Master(const Configuration& config)
			: config(config)
			, thread_pool(config.thread_count)
		{}
	private:
		ThreadPool thread_pool;
		Configuration config;
	};
}

#endif // !MAP_REDUCE_CORE_MASTER_H_
