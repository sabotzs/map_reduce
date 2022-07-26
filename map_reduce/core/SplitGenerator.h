#ifndef MAP_REDUCE_CORE_SPLIT_GENERATOR_H_
#define MAP_REDUCE_CORE_SPLIT_GENERATOR_H_

#include <string>
#include <fstream>
#include "lazy/Generator.h"

namespace mr::core::detail {
	class SplitGenerator {
	public:
		SplitGenerator(const std::string& filename, size_t size)
			: file(filename), size(size) {}

		mr::lazy::detail::Generator<std::string> generate_splits() {
			size_t parsed_size = 0;
			std::string split;
			std::string last;
			split.resize(size);

			while (!file.eof()) {
				parsed_size = 0;
				// Copy last to split
				for (size_t i = 0; i < last.size(); ++i) {
					split[i] = last[i];
					++parsed_size;
				}
				// Fill split from the file
				for (size_t i = last.size(); !file.eof() && i < size; ++i) {
					split[i] = file.get();
					++parsed_size;
					file.peek();
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
					last = split.substr(parsed_size, size - parsed_size);
					// Yield the split without the prefix
					co_yield split.substr(0, parsed_size);
				}
				else {
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
		size_t size;
	};
}

#endif // !MAP_REDUCE_CORE_SPLIT_GENERATOR_H_
