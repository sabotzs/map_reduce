#ifndef MAP_REDUCE_LAZY_GENERATOR_H_
#define MAP_REDUCE_LAZY_GENERATOR_H_

#include <optional>
#include <coroutine>

namespace mr::lazy::detail {
	template<typename T>
	class Generator {
	public:
		struct promise_type;
		
		struct iterator;
		
		using Handle = std::coroutine_handle<promise_type>;

		explicit Generator(const Handle coroutine) noexcept
			: coroutine(coroutine) {}

		Generator() = default;
		
		Generator(const Generator&) = delete;
		Generator& operator=(const Generator&) = delete;
		
		Generator(Generator&& other) noexcept
			: coroutine(other.coroutine)
		{
			other.coroutine = {};
		}

		Generator& operator=(Generator&& other) noexcept {
			if (this != &other) {
				if (coroutine) {
					coroutine.destroy();
				}
				coroutine = other.coroutine;
				other.coroutine = {};
			}
			return *this;
		}

		~Generator() {
			if (coroutine) {
				coroutine.destroy();
			}
		}

		iterator begin() {
			if (coroutine) {
				coroutine.resume();
			}
			return iterator{ coroutine };
		}

		std::default_sentinel_t end() {
			return {};
		}
	private:
		Handle coroutine;
	};

	template<typename T>
	struct Generator<T>::promise_type {
		Generator<T> get_return_object() {
			return Generator{ Handle::from_promise(*this) };
		}

		static std::suspend_always initial_suspend() noexcept {
			return {};
		}

		static std::suspend_always final_suspend() noexcept {
			return {};
		}

		std::suspend_always yield_value(T value) noexcept {
			current_value = std::move(value);
			return {};
		}

		void return_void() noexcept {}

		void await_transform() = delete;

		[[noreturn]] static void unhandled_exception() {
			throw;
		}

		std::optional<T> current_value;
	};

	template<typename T>
	class Generator<T>::iterator {
	public:
		void operator++() {
			coroutine.resume();
		}

		const T& operator*() const {
			return *coroutine.promise().current_value;
		}

		T& operator*() {
			return *coroutine.promise().current_value;
		}

		bool operator==(std::default_sentinel_t) {
			return !coroutine || coroutine.done();
		}

		explicit iterator(const Handle coroutine)
			: coroutine(coroutine) {}
	private:
		Handle coroutine;
	};
}

#endif // !MAP_REDUCE_LAZY_GENERATOR_H_
