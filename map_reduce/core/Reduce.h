#ifndef MAP_REDUCE_CORE_REDUCE_H_
#define MAP_REDUCE_CORE_REDUCE_H_

#include <string>
#include <vector>

namespace mr::core::detail {
	struct ReduceInput {
		std::string key;
		std::vector<std::string> values;

		ReduceInput(const std::string& key, std::vector<std::string>&& values)
			: key(key)
			, values(std::move(values))
		{}
	};

	struct ReduceOutput {
		std::string key;
		std::string value;

		ReduceOutput() = default;

		void write(const std::string& key, const std::string& value) {
			this->key = key;
			this->value = value;
		}
	};
}

#endif // !MAP_REDUCE_CORE_REDUCE_H_
