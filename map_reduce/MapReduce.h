#ifndef MAP_REDUCE_H_
#define MAP_REDUCE_H_

#include "core/Map.h"
#include "core/Reduce.h"
#include "core/Configuration.h"

namespace mr {
	using Mapper = core::detail::Mapper;
	using MapInput = core::detail::MapInput;
	using MapOutput = core::detail::MapOutput;
	using Reducer = core::detail::Reducer;
	using ReduceInput = core::detail::ReduceInput;
	using ReduceOutput = core::detail::ReduceOutput;
	using Configuration = core::detail::Configuration;
}

#endif // !MAP_REDUCE_H_
