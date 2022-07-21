#ifndef MAP_REDUCE_CORE_PARSER_H_
#define MAP_REDUCE_CORE_PARSER_H_

#include <fstream>
#include "lazy/Generator.h"

namespace mr::core::detail {
	class Parser {
		using SplitGenerator = mr::lazy::detail::Generator<std::string>;
	public:
		Parser(const std::string& filename)
			: file(filename) {}

		SplitGenerator parse_splits(size_t split_size) {
			std::string split;
			std::string last;
			while (!file.eof()) {
				split = last;
				split.reserve(split_size);
				while (!file.eof() && split.size() < split_size) {
					split.push_back(file.get());
				}
				auto c = file.peek();
				if (!file.eof() && !is_space(c)) {
					size_t end = split_size - 1;
					while (!is_space(split[end])) {
						--end;
					}
					last = split.substr(end, split_size - end);
					while (split.size() > end + 1) {
						split.pop_back();
					}
				}
				co_yield split;
			}
		}
	private:
		bool is_space(char c) {
			return c == ' ' || c == '\t' || c == '\n';
		}
	private:
		std::ifstream file;
	};
}

#endif // !MAP_REDUCE_CORE_PARSER_H_
