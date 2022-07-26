#ifndef MAP_REDUCE_CORE_MASTER_H_
#define MAP_REDUCE_CORE_MASTER_H_

#include <memory>
#include <unordered_map>

#include "Context.h"
#include "Configuration.h"
#include "SplitGenerator.h"
#include "concurrent/ThreadPool.h"
#include "filesystem/FileManager.h"

namespace mr::core::detail {
	class Master {
		using ThreadPool = mr::concurrent::detail::ThreadPool;
		using FileManager = mr::filesystem::detail::FileManager;
		template<typename T>
		using Generator = mr::lazy::detail::Generator<T>;
	public:
		Master(const Configuration& config)
			: config(config)
			, thread_pool(config.thread_count)
			, file_manager(config)
		{}

		void run() {
			for (auto& filename : config.input_files) {
				unfinished_splits[filename] = 0;
				unfinished_partitions[filename] = 0;
			}
			process_splits();
			merge_partitions();
			wait_all_merges();
		}
	private:
		void process_splits() {
			for (const auto& filename : config.input_files) {
				auto split_generator = SplitGenerator(filename, config.split_size);
				auto splits = split_generator.generate_splits();
				for (auto& split : splits) {
					// Create the context for split processing
					auto context = std::make_shared<Context>();
					context->filename = filename;
					context->mapper = config.mapper;
					context->reducer = config.reducer;
					context->map_input = std::make_unique<MapInput>(filename, std::move(split));
					// Submit the process task
					++unfinished_splits[context->filename];
					thread_pool.submit_task(
						[this, context] () {
							map(*context);
							transform(*context);
							reduce(*context);
							write_to_partitions(*context);
							--unfinished_splits[context->filename];
						}
					);
				}
			}
		}

		void merge_partitions() {
			std::string processed_filename;
			while (true) {
				processed_filename = "";
				// Iterate over all remaining files whose splits have not been all processed
				for (auto& [filename, processed] : unfinished_splits) {
					// If all file splits have been processed get the filename
					if (processed == 0) {
						processed_filename = filename;
						break;
					}
				}
				// If a processed file has been found, remove it from the table and merge its partitions
				if (!processed_filename.empty()) {
					unfinished_splits.erase(processed_filename);
					merge_file_splits(processed_filename);
				}
				// End when all splits have been processed
				if (unfinished_splits.empty()) {
					return;
				}
			}
		}

		void merge_file_splits(const std::string& filename) {
			auto& tmp_files = file_manager.tmp_files_for(filename);
			for (auto& file : tmp_files) {
				std::string key;
				std::string value;
				file->seekg(std::ios::beg);

				// Parse the data from temporary files
				std::unordered_map<std::string, std::vector<std::string>> grouped_split_values;
				while (!file->eof()) {
					*file >> key;
					*file >> value;
					file->get(); // get the newline
					file->peek();
					if (!grouped_split_values.contains(key)) {
						grouped_split_values[key] = {};
					}
					grouped_split_values[key].emplace_back(value);
				}

				// Create context from grouped values
				auto context = std::make_shared<Context>();
				context->filename = filename;
				context->reducer = config.reducer;
				context->reduce_inputs = {};
				context->reduce_inputs.reserve(grouped_split_values.size());
				for (auto& [key, value] : grouped_split_values) {
					context->reduce_inputs.emplace_back(key, std::move(value));
				}
				grouped_split_values.clear();

				++unfinished_partitions[context->filename];
				thread_pool.submit_task(
					[this, context] () {
						reduce(*context);
						write_to_output(*context);
						--unfinished_partitions[context->filename];
					}
				);
			}
		}

		void wait_all_merges() {
			std::string finished;
			while (true) {
				finished = "";
				for (auto& [file, merged] : unfinished_partitions) {
					if (merged == 0) {
						finished = file;
						break;
					}
				}
				if (!finished.empty()) {
					unfinished_partitions.erase(finished);
				}
				if (unfinished_partitions.empty()) {
					return;
				}
			}
		}

		void write_to_partitions(Context& context) {
			std::hash<std::string> hasher;
			auto& tmp_files = file_manager.tmp_files_for(context.filename);
			for (auto& [key, value] : context.reduce_outputs) {
				auto index = hasher(key) % config.partitions;
				*(tmp_files[index]) << std::format("{0} {1}\n", key, value);
			}
			context.reduce_outputs.clear();
		}

		void write_to_output(Context& context) {
			auto& out_file = file_manager.out_file_for(context.filename);
			for (auto& [key, value] : context.reduce_outputs) {
				*out_file << std::format("{0} {1}\n", key, value);
			}
			context.reduce_outputs.clear();
		}
	private:
		std::unordered_map<std::string, std::atomic_size_t> unfinished_splits;
		std::unordered_map<std::string, std::atomic_size_t> unfinished_partitions;
		Configuration config;
		FileManager file_manager;
		ThreadPool thread_pool;
	};
}

#endif // !MAP_REDUCE_CORE_MASTER_H_
