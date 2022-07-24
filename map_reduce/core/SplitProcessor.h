#ifndef MAP_REDUCE_CORE_SPLIT_PROCESSOR_H_
#define MAP_REDUCE_CORE_SPLIT_PROCESSOR_H_

#include <format>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Map.h"
#include "Reduce.h"
#include "filesystem/FileManager.h"

namespace mr::core::detail {
	class SplitProcessor {
		using FileManager = mr::filesystem::detail::FileManager;
	public:
		SplitProcessor(
			const std::string& filename,
			std::string&& split,
			Mapper mapper,
			Reducer reducer,
			FileManager& file_manager
		)
			: mapper(mapper)
			, reducer(reducer)
			, file_manager(file_manager)
			, filename(filename)
			, map_input(std::make_unique<MapInput>(filename, std::move(split)))
			, map_output(nullptr)
			, reduce_input(nullptr)
			, reduce_output(nullptr)
		{}

		void process() {
			map();
			transform();
			reduce();
			write();
		}
	private:
		void map() {
			map_output = std::make_unique<MapOutput>();
			mapper(*map_input, *map_output);
			map_input.reset(nullptr);
		}

		void transform() {
			std::unordered_map<std::string, std::vector<std::string>> grouped_values;
			for (auto& [key, value] : map_output->kvp) {
				if (!grouped_values.contains(key)) {
					grouped_values[key] = {};
				}
				grouped_values[key].emplace_back(std::move(value));
			}
			map_output.reset(nullptr);

			reduce_input = std::make_unique<std::vector<ReduceInput>>();
			reduce_input->reserve(grouped_values.size());
			for (auto& [key, value] : grouped_values) {
				reduce_input->emplace_back(key, std::move(value));
			}
		}

		void reduce() {
			reduce_output = std::make_unique<std::vector<ReduceOutput>>();
			reduce_output->resize(reduce_input->size());
			for (size_t i = 0; i < reduce_input->size(); ++i) {
				reducer((*reduce_input)[i], (*reduce_output)[i]);
			}
			reduce_input.reset(nullptr);
		}

		void write() {
			std::hash<std::string> hasher;
			auto& tmp_files = file_manager.tmp_files_for(filename);
			size_t files_count = tmp_files.size();
			for (auto& output : *reduce_output) {
				size_t index = hasher(output.key) % files_count;
				*(tmp_files[index]) << std::format("{0} {1}\n", output.key, output.value);
			}
			reduce_output.reset(nullptr);
		}
	private:
		Mapper mapper;
		Reducer reducer;
		FileManager& file_manager;
		std::string filename;
		std::unique_ptr<MapInput> map_input;
		std::unique_ptr<MapOutput> map_output;
		std::unique_ptr<std::vector<ReduceInput>> reduce_input;
		std::unique_ptr<std::vector<ReduceOutput>> reduce_output;
	};
}

#endif // !MAP_REDUCE_CORE_SPLIT_PROCESSOR_H_
