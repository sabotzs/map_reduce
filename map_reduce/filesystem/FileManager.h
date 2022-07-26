#ifndef MAP_REDUCE_CORE_FILE_MANAGER_H_
#define MAP_REDUCE_CORE_FILE_MANAGER_H_

#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include "core/Configuration.h"

namespace mr::filesystem::detail {
	class FileManager {
		using Configuration = mr::core::detail::Configuration;
		using TmpFiles = std::vector<std::shared_ptr<std::fstream>>;
		using OutFile = std::shared_ptr<std::ofstream>;
	public:
		FileManager(const Configuration& config)
			: tmp_dir(config.tmp_dir)
		{
			if (!tmp_dir.ends_with('/')) {
				tmp_dir += '/';
			}
			std::string out_dir = config.out_dir;
			if (!out_dir.ends_with('/')) {
				out_dir += '/';
			}
			std::filesystem::create_directory(tmp_dir);
			std::filesystem::create_directory(out_dir);
			create_tmp_files(config.input_files, tmp_dir, config.partitions);
			create_out_files(config.input_files, out_dir);
		}

		~FileManager() {
			std::filesystem::remove_all(tmp_dir);
		}

		TmpFiles& tmp_files_for(const std::string& filename) {
			return tmp_files[filename];
		}

		OutFile& out_file_for(const std::string& filename) {
			return out_files[filename];
		}
	private:
		void create_tmp_files(const std::vector<std::string>& files, const std::string& dir, size_t count) {
			for (auto& filename : files) {
				auto filepath = std::filesystem::path(filename);
				auto base = filepath.filename().replace_extension("").string();
				auto extension = filepath.extension().string();
				auto subdir = dir + base + '/';
				std::filesystem::create_directory(subdir);

				tmp_files[filename] = {};
				tmp_files[filename].reserve(count);
				for (size_t i = 0; i < count; ++i) {
					auto name = subdir + base + '-' + std::to_string(i) + extension;
					tmp_files[filename].emplace_back(
						std::make_shared<std::fstream>(name,
							std::ios::in | std::ios::end | std::ios::app)
					);
				}
			}
		}

		void create_out_files(const std::vector<std::string>& files, const std::string& dir) {
			for (auto& filename : files) {
				auto filepath = std::filesystem::path(filename);
				auto no_path = filepath.filename().string();
				auto name = dir + no_path;
				out_files[filename] = std::make_shared<std::ofstream>(name);
			}
		}
	private:
		std::string tmp_dir;
		std::unordered_map<std::string, TmpFiles> tmp_files;
		std::unordered_map<std::string, OutFile> out_files;
	};
}

#endif // !MAP_REDUCE_CORE_FILE_MANAGER_H_
