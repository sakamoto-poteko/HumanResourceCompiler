# Human Resource Machine Compiler

[![Compiler](https://github.com/sakamoto-poteko/HumanResourceCompiler/actions/workflows/compiler-matrix.yml/badge.svg)](https://github.com/sakamoto-poteko/HumanResourceCompiler/actions/workflows/compiler-matrix.yml)
[![VSCode Extension](https://github.com/sakamoto-poteko/HumanResourceCompiler/actions/workflows/vsce.yml/badge.svg)](https://github.com/sakamoto-poteko/HumanResourceCompiler/actions/workflows/vsce.yml)

This project aims to create a compiler for the game Human Resource Machine using a custom language called "Human Resource Machine LazyCoder Language" (HRML). The language simplifies the game's coding challenges by abstracting some of its assembly-like constructs, allowing you to write code more efficiently. 

In addition, the project provides a VSCode extension for HRML language support, allowing syntax highlighting and language-specific configuration.

## Getting Started

### Prerequisites

Ensure you have the following dependencies installed before building the project. The exact dependencies vary based on the platform you are using.

- **CMake** (version 3.25 or higher)
- A modern **C++** compiler with C++20 support
- **Flex** and **Bison** (for lexical analysis and parsing)
- **spdlog** (for logging)
- **Boost** (for various utilities)
- **YAML-CPP** (for writing yaml in intermediate steps)
- **Google Test** (for testing)

### Build Instructions

The project is located in the `compiler/` directory. You can build it using CMake.

#### Manual Build Steps

1. **Clone the Repository:**
    ```bash
    git clone https://github.com/sakamoto-poteko/HumanResourceCompiler.git
    cd HumanResourceCompiler/compiler
    ```

2. **Create a Build Directory:**
    ```bash
    mkdir build
    ```

3. **Install Dependencies:**

    *Debian/Ubuntu*
    
    To install dependencies on Debian and derivations, use the following commands:
    
    ```bash
    sudo apt-get install -y cmake ninja-build flex bison libspdlog-dev libboost-dev libboost-graph-dev libboost-program-options-dev libgtest-dev
    ```
    
    *macOS*
    
    To install dependencies on macOS:
    
    ```bash
    brew update
    brew install cmake ninja boost spdlog flex bison googletest
    ```
    
    *Windows*
    
    On Windows, dependencies are installed using `vcpkg`. You can follow these steps:
    
    ```powershell
    git clone https://github.com/microsoft/vcpkg.git
    .\vcpkg\bootstrap-vcpkg.bat
    ```
    

3. **Configure the Build:**
    ```bash
    # *nix
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
    # Or with Ninja
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -G Ninja
    ```

    ```powershell
    # Windows w/ vcpkg
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake
    ```

    Replace `Release` with `Debug` or other build types if needed.

4. **Build the Project:**
    ```bash
    cmake --build build --config Release -- -j$(nproc || sysctl -n hw.ncpu || 2)
    ```

5. **Run Tests:**
    ```bash
    cd build
    ctest -C Release --output-on-failure
    ```

## VSCode Extension Setup

This project also includes a VSCode extension for Human Resource Machine LazyCoder (HRML) language support, which provides syntax highlighting and configuration.

### Development Instructions

1. **Install Dependencies:**
   - Install **Node.js** and **npm** if not already installed.

     ```bash
     sudo apt-get install nodejs npm
     ```

   - Install **vsce** (Visual Studio Code Extension Manager).

     ```bash
     npm install -g @vscode/vsce
     ```

2. **Navigate to the Extension Directory:**
   ```bash
   cd human-resource-lazycoder-vscode
   ```

3. **Package the Extension:**
   Run the following command to package the extension into a `.vsix` file:

   ```bash
   vsce package
   ```

Once installed, the extension will provide:
- **Syntax highlighting** for `.hrml` files.

### Language Overview

The Human Resource Machine LazyCoder Language (HRML) is designed to simplify the process of solving levels in Human Resource Machine. Key features include:

- **Simple Arithmetic Operations:** `+`, `-`, `*`, `/`, `%`
- **Logical Operations:** `&&`, `||`, `>`, `<`, `==`, `!=`, etc.
- **Control Structures:** `if`, `while`, `for`
- **Memory Access:** Direct access to "floor" slots with the `floor[]` syntax.
- **Basic Subprograms:** Supports simplified `function` and `sub` constructs.

### License

This project is licensed under the MIT License. See the `LICENSE` file for more details.

---

For more details on the project, including the design process and language features, check out the following articles:

1. [Building a Human Resource Machine Compiler - Preface](https://farm.poteko.moe/en/HumanResourceCompiler/hrc0.html)
2. [Building a Human Resource Machine Compiler - Language Design](https://farm.poteko.moe/en/HumanResourceCompiler/hrc1.html)
3. [Building a Human Resource Machine Compiler - Lexical Analysis](https://farm.poteko.moe/en/HumanResourceCompiler/hrc2.html)
4. [Building a Human Resource Machine Compiler - Syntax Parser (1)](https://farm.poteko.moe/en/HumanResourceCompiler/hrc3.html)

More on the way.