#ifndef MAP_REDUCE_CORE_MAP_H_
#define MAP_REDUCE_CORE_MAP_H_

#include <string>
#include <unordered_map>

namespace mr::core::detail {
	struct MapInput {
		std::string key;
		std::string value;

		MapInput(const std::string& key, std::string&& value)
			: key(key)
			, value(std::move(value))
		{}
	};

	struct MapOutput {
		std::vector<std::pair<std::string, std::string>> kvp;

		void write(const std::string& key, const std::string& value) {
			kvp.emplace_back(key, value);
		}
	};
}

#endif // !MAP_REDUCE_CORE_MAP_H_
