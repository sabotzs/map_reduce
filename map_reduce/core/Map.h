#ifndef MAP_REDUCE_CORE_MAP_H_
#define MAP_REDUCE_CORE_MAP_H_

#include <string>

namespace mr::core::detail {
	struct MapInput {
		std::string key;
		std::string value;

		MapInput(const std::string& key, std::string&& value)
			: key(key)
			, value(std::move(value))
		{}
	};
}

#endif // !MAP_REDUCE_CORE_MAP_H_
