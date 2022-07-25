#ifndef MAP_REDUCE_CORE_MASTER_H_
#define MAP_REDUCE_CORE_MASTER_H_

#include <memory>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Context.h"
#include "Configuration.h"
#include "concurrent/ThreadPool.h"
#include "filesystem/FileManager.h"
#include "lazy/Generator.h"

namespace mr::core::detail {
	class Master {
		using ThreadPool = mr::concurrent::detail::ThreadPool;
		using FileManager = mr::filesystem::detail::FileManager;
		template<typename T>
		using Generator = mr::lazy::detail::Generator<T>;
	public:
		//Master(const Configuration& config)
		//	: config(config)
		//	, thread_pool(config.thread_count)
		//{}
	private:
		void process_splits() {
			for (const auto& filename : config.input_files) {
				splits_processed[filename] = 0;
				file_manager.open_tmp_files_for_writing(filename);

				auto split_generator = parse_splits(filename);
				for (auto& split : split_generator) {
					auto context = std::make_shared<Context>();
					context->filename = filename;
					context->map_input = std::make_unique<MapInput>(filename, std::move(split));
					thread_pool.submit_task(
						[this, context] () {
							++splits_processed[context->filename];
							map(*context);
							transform(*context);
							reduce(*context);
							write_splitted(*context);
							--splits_processed[context->filename];
						}
					);
				}
			}
		}

		void write_splitted(Context& context) {
			std::hash<std::string> hasher;
			auto& tmp_files = file_manager.tmp_files_for(context.filename);
			for (auto& [key, value] : context.reduce_outputs) {
				auto index = hasher(key) % config.partitions;
				*tmp_files[index] << std::format("{0} {1}\n", key, value);
			}
			context.reduce_outputs.clear();
		}

		Generator<std::string> parse_splits(const std::string& filename) {
			auto is_space = [](char c) {
				return c == ' ' || c == '\t' || c == '\n';
			};
			std::ifstream file(filename);
			size_t parsed_size;
			std::string split;
			std::string last;
			split.resize(config.split_size);

			while (!file.eof()) {
				parsed_size = 0;
				// Copy last to split
				for (size_t i = 0; i < last.size(); ++i) {
					split[i] = last[i];
					++parsed_size;
				}
				// Fill split from the file
				for (size_t i = last.size(); !file.eof() && i < config.split_size; ++i) {
					split[i] = file.get();
					++parsed_size;
				}

				char c = file.peek();

				if (file.eof()) { // If there is nothing else to read
					co_yield split.substr(0, parsed_size);
				}
				else if (!is_space(c) && !is_space(split.back())) { // If we are in the middle of a word
					while (!is_space(split[parsed_size - 1])) {
						--parsed_size;
					}
					// Put the read prefix of that word into last
					last = split.substr(parsed_size, config.split_size - parsed_size);
					// Yield the split without the prefix
					co_yield split.substr(0, parsed_size);
				}
				else {
					co_yield split;
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
