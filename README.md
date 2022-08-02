# map_reduce

A **C++20** header-only library with no external dependencies for running MapReduce tasks on multiple threads on a single computer.
On top of the library, there is a client application (currently working only on Windows) which runs arbitrary MapReduce procedure on provided data.

## Getting started

### What you'll need

- C++ Compiler supporting the following **C++20** features: **coroutines, concepts, format**
- **[Premake](https://premake.github.io/)** for generating project files

> **_Note_**: At the time of writing **libstdc++ (gcc) doesn't implement `<format>`**.[^1]

[^1]: https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.2020

### How to use the application

1. Clone the repository `git clone https://github.com/sabotzs/map_reduce.git`
2. Generate project files with premake, e.g. `premake5 vs2022`:
3. Build the generated project
4. Run the application with the provided dll (or [create your own](#create-your-own-mapreduce-procedure))

    ```bash
    cd examples
    client word_counter.dll data/ outputs/
    ```

For more information run `client --help`

## Create your own MapReduce procedure

In your header, include `MapReduce.h`, and declare the following `map` and `reduce` functions:

```c++
#include "MapReduce.h"

extern "C" {
    void __declspec(dllexport) map(const mr::MapInput& input, mr::MapOutput& output);
    void __declspec(dllexport) reduce(const mr::ReduceInput& input, mr::ReduceOutput& output);
}
```

Afterwards define the functions in the corresponding `.cpp` file.

Here's a reference to how you could use the provided types. You can also check [Map.h](map_reduce/core/Map.h) and [Reduce.h](map_reduce/core/Reduce.h).

```c++
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
```

## Future tasks

- [ ] Make client application compatible with Linux based OS.
- [ ] \(Optional) Make the library compatible with gcc.
