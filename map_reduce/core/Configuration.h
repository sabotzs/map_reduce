#ifndef MAP_REDUCE_CORE_CONFIGURATION_H_
#define MAP_REDUCE_CORE_CONFIGURATION_H_

#include <string>
#include "Map.h"
#include "Reduce.h"

namespace mr::core::detail {
	const size_t KB = 1024;
	const size_t MB = 1024 * KB;

	struct Configuration {
		Mapper mapper = nullptr;
		Reducer reducer = nullptr;
		size_t thread_count = 0;
		size_t partitions = 1;
		size_t split_size = 1 * MB;
		std::string tmp_dir = "./.map_reduce_tmp_dir/";
		std::string out_dir = "";
		std::vector<std::string> input_files = {};
	};
}

#endif // !MAP_REDUCE_CORE_CONFIGURATION_H_
