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
			size_t size;
			char c;
			std::string split;
			std::string last;
			split.resize(split_size);

			while (!file.eof()) {
				size = 0;
				// Copy last to split
				for (size_t i = 0; i < last.size(); ++i) {
					split[i] = last[i];
					++size;
				}
				// Fill split from the file
				for (size_t i = last.size(); !file.eof() && i < split_size; ++i) {
					split[i] = file.get();
					++size;
				}

				c = file.peek();

				if (file.eof()) { // If there is nothing else to read
					co_yield split.substr(0, size);
				} else if (!is_space(c) && !is_space(split.back())) { // If we are in the middle of a word
					while (!is_space(split[size - 1])) {
						--size;
					}
					// Put the read prefix of that word into last
					last = split.substr(size, split_size - size);
					// Yield the split without the prefix
					co_yield split.substr(0, size);
				} else {
					co_yield split;
				}
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
