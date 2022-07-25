#ifndef MAP_REDUCE_CORE_CONTEXT_H_
#define MAP_REDUCE_CORE_CONTEXT_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "Map.h"
#include "Reduce.h"

namespace mr::core::detail {
	struct Context {
		Mapper mapper;
		Reducer reducer;
		std::string filename;
		std::unique_ptr<MapInput> map_input;
		std::unique_ptr<MapOutput> map_output;
		std::vector<ReduceInput> reduce_inputs;
		std::vector<ReduceOutput> reduce_outputs;
	};

	void map(Context& context) {
		context.map_output = std::make_unique<MapOutput>();
		context.mapper(*context.map_input, *context.map_output);
		context.map_input.reset(nullptr);
	}

	void transform(Context& context) {
		std::unordered_map<std::string, std::vector<std::string>> grouped_values;
		for (auto& [key, value] : context.map_output->kvp) {
			if (!grouped_values.contains(key)) {
				grouped_values[key] = {};
			}
			grouped_values[key].emplace_back(std::move(value));
		}
		context.map_output.reset(nullptr);

		context.reduce_inputs = {};
		context.reduce_inputs.reserve(grouped_values.size());
		for (auto& [key, value] : grouped_values) {
			context.reduce_inputs.emplace_back(key, std::move(value));
		}
	}

	void reduce(Context& context) {
		auto size = context.reduce_inputs.size();
		context.reduce_outputs = {};
		context.reduce_outputs.resize(size);
		for (size_t i = 0; i < size; ++i) {
			context.reducer(context.reduce_inputs[i], context.reduce_outputs[i]);
		}
		context.reduce_inputs.clear();
	}
}

#endif // !MAP_REDUCE_CORE_CONTEXT_H_
