# Design

Based on the design of Human Resource Machine (HRM), we define the LLVM target as following:

1. The machine is 16-bit width. All operations are defined as 16b in our target. In HRM, the max int is 999 and the min int is -999.
2. There is only one register `r0`. In HRM, arithmetic operations are performed either in one register or in one register with `mem`.
3. There are two special instructions `inbox` and `outbox`, loading and storing value from/to the magic queue and `r0`.
4. There are four arithmetic instructions. `add` and `sub` operate on `r0` and `mem`. `bump+` and `bump-` operate on `r0`.
5. `COPYFROM` loads from `mem` to `r0`. It takes either an `imm16` or an indirect address by `imm16`. So does `COPYTO`
6. `JUMPZ` branches if `r0` is 0. `JUMPN` branches if `r0` is less than 0. `JUMP` branches unconditionally.

There's no function call in this game. We're defining a calling convention though.

1. Arguments are passed with `r0` then stack.
2. Return value is passed with `r0`.
3. Return address is stored on the stack.

We ask the functions to be inlined during the compilation, to eliminate the use of stack. We are not assembling the code to machine code anyway.



