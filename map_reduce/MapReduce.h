#ifndef MAP_REDUCE_H_
#define MAP_REDUCE_H_

#include "core/Map.h"
#include "core/Reduce.h"
#include "core/Configuration.h"

namespace mr {
	const size_t KB = core::detail::KB;
	const size_t MB = core::detail::MB;
	const size_t GB = core::detail::GB;

	using Mapper = core::detail::Mapper;
	using MapInput = core::detail::MapInput;
	using MapOutput = core::detail::MapOutput;
	using Reducer = core::detail::Reducer;
	using ReduceInput = core::detail::ReduceInput;
	using ReduceOutput = core::detail::ReduceOutput;
	using Configuration = core::detail::Configuration;
}

#endif // !MAP_REDUCE_H_
