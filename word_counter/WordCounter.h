#ifndef WORD_COUNTER_H_
#define WORD_COUNTER_H_

#include "pch.h"

extern "C" {
	void __declspec(dllexport) map(const mr::MapInput& input, mr::MapOutput& output);
	void __declspec(dllexport) reduce(const mr::ReduceInput& input, mr::ReduceOutput& output);
}

#endif // !WORD_COUNTER_H_
