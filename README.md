# Project Name: Human Resource Machine Compiler

This project aims to create a compiler for the game Human Resource Machine using a custom language called "Human Resource Machine LazyCoder Language" (HRML). The language simplifies the game's coding challenges by abstracting some of its assembly-like constructs, allowing you to write code more efficiently. The project includes components such as lexical analysis, parsing, intermediate representation (IR), code generation, and optimization.

In addition, the project provides a VSCode extension for HRML language support, allowing syntax highlighting and language-specific configuration.

## Getting Started

### Prerequisites

Ensure you have the following dependencies installed before building the project:

- **CMake** (version 3.25 or higher)
- A **C++** compiler (e.g., `g++` or `clang++`)
- **Flex** and **Bison** (for lexical analysis and parsing)
- **spdlog** (for logging)
- **Boost** (for various utilities)

To install these on Debian/Ubuntu:

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

5. **Run Tests (no tests yet):**
   ```bash
   cd build
   ctest -C Release
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

4. **Install the Extension Locally:**
   - In VSCode, press `F1`, type "Extensions: Install from VSIX...", and select the `.vsix` file generated in the previous step.
   - You can also install it manually by running:

     ```bash
     code --install-extension path_to_vsix_file
     ```

### Installation Instructions for Users

If you're not developing but just want to install the extension:

1. **Download the `.vsix` File:**
   Download the packaged extension from the repository or from a release.

2. **Install the Extension in VSCode:**
   - Open VSCode.
   - Press `F1`, type "Extensions: Install from VSIX...", and select the `.vsix` file.
   - Alternatively, you can use the terminal:

     ```bash
     code --install-extension path_to_vsix_file
     ```

Once installed, the extension will provide:
- **Syntax highlighting** for `.hrml` files.
- **Language-specific configuration** for HRML.

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