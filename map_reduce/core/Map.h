#ifndef MAP_REDUCE_CORE_MAP_H_
#define MAP_REDUCE_CORE_MAP_H_

#include <utility>
#include <string>
#include <vector>

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

	using Mapper = void(*)(const MapInput&, MapOutput&);
}

#endif // !MAP_REDUCE_CORE_MAP_H_
