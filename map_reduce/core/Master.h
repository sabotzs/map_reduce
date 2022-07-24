#ifndef MAP_REDUCE_CORE_MASTER_H_
#define MAP_REDUCE_CORE_MASTER_H_

#include <memory>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Parser.h"
#include "Configuration.h"
#include "SplitProcessor.h"
#include "concurrent/ThreadPool.h"
#include "filesystem/FileManager.h"

namespace mr::core::detail {
	class Master {
		using ThreadPool = mr::concurrent::detail::ThreadPool;
		using FileManager = mr::filesystem::detail::FileManager;
	public:
		Master(const Configuration& config)
			: config(config)
			, thread_pool(config.thread_count)
			, file_manager(config.tmp_dir, config.partitions)
		{}
	private:
		void process_splits() {
			for (const auto& filename : config.input_files) {
				file_manager.create_tmp_files_for(filename);
				splits_processed[filename] = 0;

				Parser parser(filename);
				auto split_generator = parser.parse_splits(config.split_size);
				for (auto& split : split_generator) {
					auto processor = std::make_shared<SplitProcessor>(filename, std::move(split),
						config.mapper, config.reducer, file_manager);
					thread_pool.submit_task(
						[this, filename, processor] {
							++splits_processed[filename];
							processor->process();
							--splits_processed[filename];
						}
					);
				}
			}
		}
	private:
		ThreadPool thread_pool;
		Configuration config;
		FileManager file_manager;
		std::unordered_map<std::string, std::atomic_size_t> splits_processed;
	};
}

#endif // !MAP_REDUCE_CORE_MASTER_H_
