# Human Resource Machine Compiler

This project aims to create a compiler for the game Human Resource Machine using a custom language called "Human Resource Machine LazyCoder Language" (HRML). The language simplifies the game's coding challenges by abstracting some of its assembly-like constructs, allowing you to write code more efficiently. The project includes components such as lexical analysis, parsing, intermediate representation (IR), code generation, and optimization.

## Getting Started

### Prerequisites

Ensure you have the following dependencies installed before building the project:

- **CMake** (version 3.25 or higher)
- A **C++** compiler (e.g., `g++` or `clang++`)
- **Flex** and **Bison** (for lexical analysis and parsing)
- **spdlog** (for logging)
- **Boost** (for various utilities)

To install these on Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y cmake g++ flex bison libspdlog-dev libboost-all-dev
```

### Build Instructions

The project is located in the `compiler/` directory. To build it using CMake, follow the steps below:

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/HRMCompiler.git
   cd HRMCompiler/compiler
   ```

2. **Create a Build Directory:**
   ```bash
   mkdir build
   ```

3. **Configure the Build:**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```
   Replace `Release` with `Debug` or other build types if needed.

4. **Build the Project:**
   ```bash
   cmake --build build -- -j$(nproc)
   ```

5. **Run Tests:**
   ```bash
   cd build
   ctest -C Release
   ```

### GitHub Actions Integration

This project includes a GitHub Actions workflow that automatically builds and tests the code on every push or pull request to the `master` branch. The workflow performs the following:

- Installs the necessary dependencies.
- Configures and builds the project using CMake.
- Runs the tests using `ctest`.

You can customize the build type by modifying the `BUILD_TYPE` environment variable in the GitHub Actions workflow file.

### Language Overview

The Human Resource Machine LazyCoder Language (HRML) is designed to simplify the process of solving levels in Human Resource Machine. Key features include:

- **Simple Arithmetic Operations:** `+`, `-`, `*`, `/`, `%`
- **Logical Operations:** `&`, `|`, `>`, `<`, `==`, `!=`, etc.
- **Control Structures:** `if`, `while`, `for`
- **Memory Access:** Direct access to "floor" slots with the `floor[]` syntax.
- **Basic Subprograms:** Supports simplified `function` and `sub` constructs.

### License

This project is licensed under the MIT License. See the `LICENSE` file for more details.

---

For more details on the project, including the design process and language features, check out the following articles:

1. [Building a Human Resource Machine Compiler - Preface](#)
2. [Building a Human Resource Machine Compiler - Language Design](#)
3. [Building a Human Resource Machine Compiler - Lexical Analysis](#)