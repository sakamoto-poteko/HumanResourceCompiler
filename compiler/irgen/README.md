# IR Generator

*This is the design doc by GPT. Our design is largely based on it but differs in some details.*

## Overview

This module handles the **Intermediate Representation (IR) Generation** phase of the compiler, which includes:
- **Control Flow Graph (CFG) construction**: A representation of the flow of control through the program.
- **Static Single Assignment (SSA)** form: A transformation that simplifies the optimization process by ensuring each variable is assigned exactly once.
- **High-Level IR**: A more abstract and target-independent representation of the program.
- **Low-Level IR**: A machine-specific IR closer to the constraints and instructions of our accumulator-based architecture.

The IR is first generated as **high-level IR**, which provides an expressive, target-independent abstraction of the program. Later, it is lowered into **low-level IR**, which is closer to the architecture of the target machine, specifically designed to fit the constraints of an accumulator-based HRM architecture.

## Intermediate Representations

### 1. **High-Level IR (HIR)**

The High-Level IR (HIR) is generated from the Abstract Syntax Tree (AST) and is designed to be a target-independent representation of the program. The HIR uses a rich set of instructions, allowing for more expressive control flow, arithmetic, and logical operations.

Key features of HIR include:
- **Three-Address Code (TAC)** format for most operations.
- **Comprehensive control flow instructions**: Conditional branches like `je` (jump if equal), `jne` (jump if not equal), `jgt` (jump if greater), etc.
- **Arithmetic operations**: Operations like `mul` (multiply), `div` (divide), and `mod` (modulus) are represented, even if the target architecture lacks direct support for them.

HIR allows for powerful optimizations such as:
- **Control flow analysis and optimization** (e.g., dead code elimination, branch simplification).
- **Algebraic optimizations** (constant folding, strength reduction, etc.).

### 2. **Low-Level IR (LIR)**

After generating HIR and applying high-level optimizations, the IR is **lowered** into **Low-Level IR (LIR)**. This representation is closer to the actual instruction set of the target architecture, which is an accumulator-based machine with limited branching and arithmetic capabilities.

Key aspects of LIR:
- **Accumulator-based**: All operations assume an accumulator-based instruction set where most arithmetic and memory operations involve the accumulator.
- **Translation of complex operations**: Operations like `mul`, `div`, and `mod` are broken down into sequences of addition and subtraction.
- **Simple branching**: High-level conditional jumps are translated into basic jump instructions (`jump`, `jumpn`, `jumpz`).

## IR Structure

### High-Level IR (HIR) Instructions

#### 1. **Arithmetic Operations**
These operations handle basic arithmetic, represented in the TAC format.

| Operation | Description                                 | Example          |
|-----------|---------------------------------------------|------------------|
| `add`     | Adds `src1` and `src2`, stores in `dest`    | `add a, b, c`    |
| `sub`     | Subtracts `src2` from `src1`, stores in `dest` | `sub a, b, c`    |
| `mul`     | Multiplies `src1` by `src2`, stores in `dest` | `mul a, b, c`    |
| `div`     | Divides `src1` by `src2`, stores in `dest`  | `div a, b, c`    |
| `mod`     | Modulo operation, `src1 % src2`, stores in `dest` | `mod a, b, c`    |
| `neg`     | Negates `src`, stores result in `dest`      | `neg a, b`       |

*Note*: `mul`, `div`, and `mod` may involve more complex lowering to handle without native hardware support.

#### 2. **Data Movement Operations**
Operations for moving data between registers or from immediate values.

| Operation | Description                                     | Example           |
|-----------|-------------------------------------------------|-------------------|
| `mov`     | Copies the value from `src` to `dest`           | `mov a, b`        |
| `load`    | Loads a value from memory at `[addr]` or from address in `src`  | `load a, [100]` or `load a, (r1)` |
| `store`   | Stores a value to memory at `[addr]` or to address in `src`     | `store a, [100]` or `store a, (r1)` |
| `loadi`   | Loads an immediate constant into `dest`                         | `loadi a, 10`          |


*Note*: In an accumulator architecture, `mov` may be used to load values into the accumulator from memory.

#### 3. **Logical Operations**
These operations deal with boolean and bitwise logic, useful for conditions and low-level manipulation.

| Operation  | Description                                   | Example            |
|------------|-----------------------------------------------|--------------------|
| `and`      | Bitwise AND between `src1` and `src2`         | `and a, b, c`      |
| `or`       | Bitwise OR between `src1` and `src2`          | `or a, b, c`       |
| `not`      | Bitwise NOT on `src`, stores in `dest`        | `not a, b`         |

#### 4. **Control Flow Operations**
Includes branching, jumps, and conditional branches, allowing for structured control flow in the program.

| Operation  | Description                                                | Example               |
|------------|------------------------------------------------------------|-----------------------|
| `je`       | Jump if `src1 == src2`                                     | `je a, b, label`      |
| `jne`      | Jump if `src1 != src2`                                     | `jne a, b, label`     |
| `jgt`      | Jump if `src1 > src2`                                      | `jgt a, b, label`     |
| `jlt`      | Jump if `src1 < src2`                                      | `jlt a, b, label`     |
| `jge`      | Jump if `src1 >= src2`                                     | `jge a, b, label`     |
| `jle`      | Jump if `src1 <= src2`                                     | `jle a, b, label`     |
| `jz`       | Jump if `src == 0`                                         | `jz a, label`         |
| `jnz`      | Jump if `src != 0`                                         | `jnz a, label`        |
| `jmp`      | Unconditional jump to `label`                              | `jmp label`           |
| `call`     | Call subroutine at `label` with param b, and saves to a    | `call label, b, a`    |
| `enter`    | Marks entry to a function and load the param into a        | `enter a`             |
| `ret`      | Return from subroutine                                     | `ret a`               |


#### 5. **Special Operations**
These operations handle input/output and other specialized tasks, including system-level instructions.

| Operation  | Description                                                | Example               |
|------------|------------------------------------------------------------|-----------------------|
| `input`    | Reads input into `dest`                                    | `input a`             |
| `output`   | Writes the value of `src` to the output                    | `output a`            |
| `nop`      | No operation, placeholder instruction                      | `nop`                 |
| `halt`     | Halts the program                                          | `halt`                |

### Example Usage

```text
; Example: Summing two numbers and outputting the result
input a             ; Read input into a
input b             ; Read input into b
add c, a, b         ; c = a + b
output c            ; Output result stored in c
halt                ; End program
```

### Design Considerations

- **Accumulator Usage**: Since this is an accumulator-based architecture, the actual machine code will likely involve frequent use of the accumulator to perform intermediate calculations. For example, `add` and `sub` will involve loading one operand into the accumulator, performing the operation, and storing the result.
- **No Stack**: Without a stack, subroutine calls (`call`) will likely involve fixed memory locations for return addresses and parameters.
- **Lowering Complexity**: Certain operations like `mul`, `div`, `mod`, and comparison instructions might require lowering to simpler instructions depending on what the hardware natively supports. 

### Low-Level IR (LIR) Instructions

TBD

## Control Flow Graph (CFG) and SSA

### Control Flow Graph (CFG)
The **Control Flow Graph (CFG)** is generated during IR generation to represent the possible paths of execution within a function. Each node in the CFG corresponds to a basic block, and edges represent possible control flow transitions (e.g., jumps, branches).

### Static Single Assignment (SSA)
During the IR generation process, we also convert the HIR into **Static Single Assignment (SSA) form**. In SSA, each variable is assigned exactly once, which simplifies many optimizations like constant propagation, dead code elimination, and common subexpression elimination.

SSA transformation involves:
- **Renaming variables**: Every time a variable is assigned, it gets a new name.
- **Φ-functions**: At points where multiple control flow paths converge, **φ-functions** are introduced to select the correct variable value based on the execution path.

## Workflow

[x] **AST to HIR**: The first step translates the Abstract Syntax Tree (AST) generated by the parser into High-Level IR (HIR).
[ ] **CFG and SSA**: During HIR generation, we also build the Control Flow Graph (CFG) and convert the IR into Static Single Assignment (SSA) form for optimization.
[ ] **HIR Optimizations**: Apply high-level optimizations such as constant folding, dead code elimination, and algebraic simplifications.

## Codegen tasks
[ ] **HIR to LIR Lowering (Tentative)**: The HIR is then lowered into Low-Level IR (LIR), closer to the accumulator-based machine.
[ ] **Code Generation**: Finally, the LIR is translated into target-specific assembly or machine code, expanding complex operations like multiplication and handling limited branching instructions.
