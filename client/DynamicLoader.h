#ifndef MAP_REDUCE_CLIENT_DYNAMIC_LOADER_H_
#define MAP_REDUCE_CLIENT_DYNAMIC_LOADER_H_

#include <string>
#include <Windows.h>

static class DynamicLoader {
public:
	static void* load_library(const char* lib) {
		return LoadLibraryA(lib);
	}

	static void* load_symbol(void* lib, const char* sym) {
		return GetProcAddress(static_cast<HINSTANCE>(lib), sym);
	}

	static std::string get_error() {
		auto error_message_id = GetLastError();
		if (error_message_id == 0) {
			return std::string{};
		}
		// Generate error message based on last error
		LPSTR buff = nullptr;
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
		DWORD lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
		size_t size = FormatMessageA(flags, nullptr, error_message_id, lang, buff, 0, nullptr);
		// Copy the message into string and free allocated buffer
		std::string message(buff, size);
		LocalFree(buff);
		return message;
	}

	static bool close_library(void* lib) {
		return FreeLibrary(static_cast<HINSTANCE>(lib));
	}
};

#endif // !MAP_REDUCE_CLIENT_DYNAMIC_LOADER_H_
