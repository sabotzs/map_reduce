#include <string>
#include <iostream>
#include <optional>
#include "MapReduce.h"
#include "core/Master.h"
#include "DynamicLoad.h"

namespace mr {
	using Master = core::detail::Master;
}

const std::string short_thread_count_option	= "-t";
const std::string long_thread_count_option = "--thread-count";
const std::string short_split_size_option = "-s";
const std::string long_split_size_option = "--split-size";
const std::string short_partitions_option = "-p";
const std::string long_partitions_option = "--partitions";
std::string program_name;

void print_help() {
	std::cout << "USAGE\n"
		"\tclient [OPTION]... FUNC FILE... DIRECTORY\n"
		"\tclient [OPTION]... FUNC SRCDIR DSTDIR\n\n"
		"DESCRIPTION\n"
		"\tApply map reduce using functions implemented in shared library FUNC\n"
		"\tto FILE(s) and generate the output files in DIRECTORY\n\n"
		"\t" + short_thread_count_option + ", " + long_thread_count_option + "=COUNT\n"
		"\t\tuse COUNT threads for the procedure (default is count of physical threads)\n\n"
		"\t" + short_split_size_option + ", " + long_split_size_option + "=COUNT[bkMG]\n"
		"\t\tprocess input files in chunks of COUNT [kMG]bytes (default is 1 MB)\n"
		"\t\t(b - byte(optional), k - kilo, M - mega, G - giga)\n\n"
		"\t" + short_partitions_option + ", " + long_partitions_option + "=COUNT\n"
		"\t\tcreate COUNT temporary files for grouping values with same keys\n"
		"\t\t(default is 1)\n\n"
		"\t--help\n"
		"\t\tdisplay this message\n";
}

void print_general_error_msg() {
	std::cerr << program_name +
		": you must specify shared library, input files and target directory\n"
		"Try '" + program_name + " --help' for more information\n";
}

std::optional<size_t> get_thread_count(size_t& index, char** args) {
	std::string option(args[index]);

	if (option == short_thread_count_option || option == long_thread_count_option) {
		return std::stoull(args[++index]);
	} else if (option.starts_with(long_thread_count_option + "=")) {
		size_t len = long_thread_count_option.size() + 1;
		return std::stoull(option.substr(len));
	}
	return std::nullopt;
}

std::optional<size_t> get_split_size(size_t& index, char** args) {
	static auto get_bytes = [](const std::string& ss) -> size_t {
		size_t len = ss.size();
		switch (ss.back()) {
		case 'b': return std::stoull(ss.substr(0, len - 1));
		case 'k': return std::stoull(ss.substr(0, len - 1)) * mr::KB;
		case 'M': return std::stoull(ss.substr(0, len - 1)) * mr::MB;
		case 'G': return std::stoull(ss.substr(0, len - 1)) * mr::GB;
		default	: return std::stoull(ss);
		}
	};

	std::string option(args[index]);
	if (option == short_split_size_option || option == long_split_size_option) {
		return get_bytes(args[++index]);
	} else if (option.starts_with(long_split_size_option + "=")) {
		size_t len = long_split_size_option.size() + 1;
		return get_bytes(option.substr(len));
	}
	return std::nullopt;
}

std::optional<size_t> get_partitions(size_t& index, char** args) {
	std::string option(args[index]);

	if(option == short_partitions_option || option == long_partitions_option) {
		std::string partitions(args[++index]);
		return std::stoull(partitions);
	} else if (option.ends_with(long_partitions_option + "=")) {
		size_t len = long_partitions_option.size() + 1;
		return std::stoull(option.substr(len));
	}
	return std::nullopt;
}

bool try_parse_thread_count(mr::Configuration& config, size_t& index, char** args) {
	try {
		auto tc = get_thread_count(index, args);
		config.thread_count = tc.value_or(config.thread_count);
		return tc.has_value();
	} catch (...) {
		std::cerr << program_name + ": Invalid thread count argument\n"
			"Try '" + program_name + " --help' for more information\n";
		throw;
	}
}

bool try_parse_split_size(mr::Configuration& config, size_t& index, char** args) {
	try {
		auto ss = get_split_size(index, args);
		config.split_size = ss.value_or(config.split_size);
		return ss.has_value();
	} catch (...) {
		std::cerr << program_name + ": Invalid split size argument\n"
			"Try '" + program_name + " --help' for more information\n";
		throw;
	}
}

bool try_parse_partitions(mr::Configuration& config, size_t& index, char** args) {
	try {
		auto ps = get_partitions(index, args);
		config.partitions = ps.value_or(config.partitions);
		return ps.has_value();
	} catch (...) {
		std::cerr << program_name + ": Invalid partitions argument\n"
			"Try '" + program_name + " --help' for more information\n";
		throw;
	}
}

void parse_options(mr::Configuration& config, size_t& index, char** args) {
	for (size_t i = 0; i < 3; ++i) {
		bool parsed = false;
		parsed = parsed || try_parse_thread_count(config, index, args);
		parsed = parsed || try_parse_split_size(config, index, args);
		parsed = parsed || try_parse_partitions(config, index, args);
		if (parsed) {
			++index;
		}
	}
}

void parse_input_files(mr::Configuration& config, size_t& index, size_t argc, char** args) {
	for (; index < static_cast<size_t>(argc - 1); ++index) {
		auto path = std::filesystem::path(args[index]);
		if (std::filesystem::is_directory(path)) {
			for (auto& dir_entry : std::filesystem::directory_iterator(path)) {
				if (dir_entry.is_regular_file()) {
					config.input_files.push_back(dir_entry.path().string());
				}
			}
		} else if (std::filesystem::is_regular_file(path)) {
			config.input_files.push_back(path.string());
		} else {
			std::cerr << program_name + ": Invalid file provided " + args[index]
				+ "\nExpected regular file or directory\n";
			throw;
		}
	}
}

void parse_output_directory(mr::Configuration& config, size_t& index, char** args) {
	auto path = std::filesystem::path(args[index]);
	bool is_dir = std::filesystem::is_directory(path);
	bool exists = std::filesystem::exists(path);
	if ((exists && is_dir) || !is_dir) {
		config.out_dir = args[index];
	} else {
		std::cerr << program_name + ": Invalid last argument " + args[index]
			+ "\nExpected output directory\n";
		throw;
	}
}

void* load_library_and_symbols(mr::Configuration& config, size_t& index, char** args) {
	void* lib = dl::load_library(args[index++]);
	auto check_load_error = [&lib]() {
		std::string error_msg = dl::get_error();
		if (!error_msg.empty()) {
			std::cerr << error_msg << '\n';
			if (lib) {
				dl::close_library(lib);
			}
			throw;
		}
	};
	check_load_error();
	config.mapper = (mr::Mapper)dl::load_symbol(lib, "map");
	check_load_error();
	config.reducer = (mr::Reducer)dl::load_symbol(lib, "reduce");
	check_load_error();
	return lib;
}

int main(int argc, char** argv) {
	program_name = std::filesystem::path(argv[0]).filename().replace_extension("").string();
	if (argc == 1) {
		print_general_error_msg();
		return 1;
	}
	if (argc == 2) {
		if (std::string(argv[1]) == "--help") {
			print_help();
			return 0;
		}
		print_general_error_msg();
		return 1;
	}
	mr::Configuration config;
	size_t index = 1;
	void* lib;
	try {
		parse_options(config, index, argv);
		lib = load_library_and_symbols(config, index, argv);
		parse_input_files(config, index, static_cast<size_t>(argc), argv);
		parse_output_directory(config, index, argv);
	} catch (...) {
		return 1;
	}

	mr::Master master(config);
	master.run();
	dl::close_library(lib);
	return 0;
}
