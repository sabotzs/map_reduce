#ifndef MAP_REDUCE_CORE_FILE_MANAGER_H_
#define MAP_REDUCE_CORE_FILE_MANAGER_H_

#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>

namespace mr::filesystem::detail {
	class FileManager {
		using TempFiles = std::vector<std::shared_ptr<std::fstream>>;
	public:
		FileManager(const std::string tmp_dir, size_t partitions)
			: tmp_dir(tmp_dir)
			, partitions(partitions)
			, tmp_files()
		{
			if (!std::filesystem::is_directory(this->tmp_dir)) {
				this->tmp_dir += '/';
			}
			std::filesystem::create_directory(this->tmp_dir);
		}

		void open_tmp_files_for_writing(const std::string& filename) {
			std::string file_tmp_dir = tmp_dir + filename + '/';
			std::string tmp_file_basename = file_tmp_dir + filename + "-";

			std::filesystem::create_directory(file_tmp_dir);

			tmp_files[filename] = {};
			tmp_files[filename].reserve(partitions);
			for (size_t i = 0; i < partitions; ++i) {
				auto name = tmp_file_basename + std::to_string(i);
				auto ofs = std::make_shared<std::fstream>(name, std::ios::out);
				tmp_files[filename].emplace_back(ofs);
			}
		}

		void close_tmp_files_for_writing(const std::string& filename) {
			tmp_files.erase(filename);
		}

		void open_tmp_files_for_reading(const std::string& filename) {
			std::string file_tmp_dir = tmp_dir + filename + '/';
			std::string tmp_file_basename = file_tmp_dir + filename + "-";

			tmp_files[filename] = {};
			tmp_files[filename].reserve(partitions);
			for (size_t i = 0; i < partitions; ++i) {
				auto name = tmp_file_basename + std::to_string(i);
				auto ifs = std::make_shared<std::fstream>(name, std::ios::in);
				tmp_files[filename].emplace_back(ifs);
			}
		}

		void close_tmp_files_for_reading(const std::string& filename) {
			tmp_files.erase(filename);
		}

		TempFiles& tmp_files_for(const std::string& filename) {
			return tmp_files[filename];
		}
	private:
		std::string tmp_dir;
		size_t partitions;
		std::unordered_map<std::string, TempFiles> tmp_files;
	};
}

#endif // !MAP_REDUCE_CORE_FILE_MANAGER_H_
