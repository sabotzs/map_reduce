#include "pch.h"
#include "WordCounter.h"

bool is_space(char c) {
	return c == ' ' || c == '\t' || c == '\n';
}

void map(const mr::MapInput& input, mr::MapOutput& output) {
	const std::string& text = input.value;
	const size_t size = text.size();

	for (size_t i = 0; i < size; ++i) {
		while (i < size && is_space(text[i])) {
			++i;
		}
		size_t start = i;
		while (i < size && !is_space(text[i])) {
			++i;
		}
		if (start < i) {
			output.write(text.substr(start, i - start), "1");
		}
	}
}

void reduce(const mr::ReduceInput& input, mr::ReduceOutput& output) {
	int64_t result = 0;
	for (auto& val : input.values) {
		result += std::stoll(val);
	}
	output.write(input.key, std::to_string(result));
}
