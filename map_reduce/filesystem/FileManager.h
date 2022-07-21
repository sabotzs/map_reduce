#ifndef MAP_REDUCE_CORE_FILE_MANAGER_H_
#define MAP_REDUCE_CORE_FILE_MANAGER_H_

#include <memory>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>

namespace mr::filesystem::detail {
	class FileManager {
		using TempFiles = std::vector<std::shared_ptr<std::ofstream>>;
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

		void create_tmp_files_for(const std::string& filename) {
			std::string file_tmp_dir = tmp_dir + filename + '/';
			std::string tmp_file_basename = file_tmp_dir + filename + "-";

			std::filesystem::create_directory(file_tmp_dir);

			tmp_files[filename] = {};
			tmp_files[filename].reserve(partitions);
			for (size_t i = 0; i < partitions; ++i) {
				auto ofs = std::make_shared<std::ofstream>(tmp_file_basename + std::to_string(i));
				tmp_files[filename].emplace_back(ofs);
			}
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
