Below are the revised math problems and corresponding code snippets tailored to fit within compiler's limitations.

---

### 1. **Factorial of a Number**

**Problem:** Compute the factorial of a given non-negative integer `n`.  
**Known Solution:** `n! = n × (n-1) × ... × 1`  
**Adjusted Example:** `6! = 720` (Since `7! = 5040` exceeds `999`)

```c
// Factorial of a number
function factorial(n) {
    let result = 1;
    let i;
    for (i = 1; i <= n; i = i + 1) {
        result = result * i;
    }
    return result;
}

function main() {
    let number = 6;
    let fact = factorial(number);
    // Expected output: 720
    return fact;
}
```

**Notes:**
- **Input Constraint:** Use `n` such that `n! <= 999`. For example, `n` can be up to `6` since `6! = 720` and `7! = 5040` exceeds the limit.

---

### 2. **Fibonacci Sequence**

**Problem:** Compute the `n`-th Fibonacci number.  
**Known Solution:** `F(n) = F(n-1) + F(n-2)` with `F(0)=0` and `F(1)=1`  
**Adjusted Example:** `F(16) = 987` (Since `F(17) = 1597` exceeds `999`)

```c
// Fibonacci sequence
function fibonacci(n) {
    if (n == 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    let a = 0;
    let b = 1;
    let c;
    let i;
    for (i = 2; i <= n; i = i + 1) {
        c = a + b;
        a = b;
        b = c;
    }
    return b;
}

function main() {
    let number = 16;
    let fib = fibonacci(number);
    // Expected output: 987
    return fib;
}
```

**Notes:**
- **Input Constraint:** Use `n` up to `16` to ensure `F(n)` does not exceed `999`.

---

### 3. **Prime Number Checker**

**Problem:** Determine whether a given integer `n` is a prime number.  
**Known Solution:** A prime number is greater than `1` and has no positive divisors other than `1` and itself.  
**Example:** `7` is prime, `8` is not.

```c
// Prime number checker
function isPrime(n) {
    if (n <= 1) {
        return 0; // Not prime
    }
    let i;
    for (i = 2; i * i <= n; i = i + 1) {
        if (n % i == 0) {
            return 0; // Not prime
        }
    }
    return 1; // Prime
}

function main() {
    let number = 7;
    let prime = isPrime(number);
    // Expected output: 1 (true)
    return prime;
}
```

**Notes:**
- **Range Compliance:** Any `n` within `-999` to `+999` can be tested. Typically, primes are positive integers greater than `1`.

---

### 4. **Greatest Common Divisor (GCD)**

**Problem:** Find the greatest common divisor of two integers `a` and `b`.  
**Known Solution:** Using the Euclidean algorithm.  
**Adjusted Example:** `GCD(48, 18) = 6`

```c
// GCD using Euclidean algorithm
function gcd(a, b) {
    while (b != 0) {
        let temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

function main() {
    let num1 = 48;
    let num2 = 18;
    let result = gcd(num1, num2);
    // Expected output: 6
    return result;
}
```

**Notes:**
- **Input Constraint:** Ensure that `a` and `b` are within `-999` to `+999`. The GCD function works correctly with positive integers; if negatives are used, the absolute values are typically considered.

---

### 5. **Sum of First N Natural Numbers**

**Problem:** Calculate the sum of the first `n` natural numbers.  
**Known Solution:** `Sum = n*(n+1)/2`  
**Adjusted Example:** Sum of first `44` natural numbers is `990` (Since `45*46/2 = 1035` exceeds `999`)

```c
// Sum of first n natural numbers
function sumOfNatural(n) {
    let sum = 0;
    let i;
    for (i = 1; i <= n; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}

function main() {
    let number = 44;
    let sum = sumOfNatural(number);
    // Expected output: 990
    return sum;
}
```

**Notes:**
- **Input Constraint:** Use `n` such that `n*(n+1)/2 <= 999`. For example, `n` can be up to `44`.

---

### 6. **Reverse a Number**

**Problem:** Reverse the digits of an integer `n`.  
**Known Solution:** Reversing the digits of `12345` yields `54321`  
**Adjusted Example:** `Reverse(123) = 321`

```c
// Reverse a number
function reverseNumber(n) {
    let reversed = 0;
    while (n > 0) {
        let digit = n % 10;
        reversed = reversed * 10 + digit;
        n = n / 10;
    }
    return reversed;
}

function main() {
    let number = 123;
    let reversed = reverseNumber(number);
    // Expected output: 321
    return reversed;
}
```

**Notes:**
- **Input Constraint:** Ensure that the reversed number does not exceed `999`. For example, `999` reverses to `999`, which is within the limit.

---

### 7. **Greatest Common Divisor (GCD) using Recursive Approach**

**Problem:** Find the GCD of two integers `a` and `b` using recursion.  
**Known Solution:** Euclidean algorithm recursively.  
**Adjusted Example:** `GCD(54, 24) = 6`

```c
// Recursive GCD
function gcdRecursive(a, b) {
    if (b == 0) {
        return a;
    }
    return gcdRecursive(b, a % b);
}

function main() {
    let num1 = 54;
    let num2 = 24;
    let result = gcdRecursive(num1, num2);
    // Expected output: 6
    return result;
}
```

**Notes:**
- **Input Constraint:** Similar to the iterative GCD, ensure `a` and `b` are within `-999` to `+999`.

---

### 8. **Binary Search**

**Problem:** Perform binary search on a sorted array to find the index of a target value.  
**Known Solution:** Binary search algorithm returns the index if found, else `-1`.  
**Adjusted Example:** Searching for `5` in `[1, 3, 5, 7, 9]` returns index `2`

```c
// Binary search
function binarySearch(arr, size, target) {
    let left = 0;
    let right = size - 1;
    while (left <= right) {
        let mid = left + (right - left) / 2;
        if (arr[mid] == target) {
            return mid;
        }
        if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1; // Not found
}

function main() {
    let arr[5];
    arr[0] = 1;
    arr[1] = 3;
    arr[2] = 5;
    arr[3] = 7;
    arr[4] = 9;
    let size = 5;
    let target = 5;
    let index = binarySearch(arr, size, target);
    // Expected output: 2
    return index;
}
```

**Notes:**
- **Array Constraints:** Ensure all array elements and the target are within `-999` to `+999`.
- **Division Handling:** The calculation `mid = left + (right - left) / 2` should perform integer division. If your compiler truncates decimals, it's fine; otherwise, adjust accordingly.

---

### 9. **Bubble Sort**

**Problem:** Sort an array of integers in ascending order using the Bubble Sort algorithm.  
**Known Solution:** Repeatedly swapping adjacent elements if they are in the wrong order.  
**Adjusted Example:** Sorting `[5, 3, 8, 4, 2]` results in `[2, 3, 4, 5, 8]`

```c
// Bubble Sort
function bubbleSort(arr, size) {
    let i, j;
    for (i = 0; i < size - 1; i = i + 1) {
        for (j = 0; j < size - i - 1; j = j + 1) {
            if (arr[j] > arr[j + 1]) {
                // Swap arr[j] and arr[j + 1]
                let temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

function main() {
    let arr[5];
    arr[0] = 5;
    arr[1] = 3;
    arr[2] = 8;
    arr[3] = 4;
    arr[4] = 2;
    let size = 5;
    bubbleSort(arr, size);
    // Expected sorted array: [2, 3, 4, 5, 8]
    return arr[0]; // Return first element to verify sorting
}
```

**Notes:**
- **Array Constraints:** Ensure all elements are within `-999` to `+999`.
- **Verification:** Since returning the entire array may not be supported, returning the first element (`arr[0]`) allows you to verify if the sort was successful (`2` in this example).

---

### 10. **Sum of Digits**

**Problem:** Calculate the sum of the digits of an integer `n`.  
**Known Solution:** Sum of digits of `1234` is `10`  
**Adjusted Example:** `SumDigits(432) = 9`

```c
// Sum of digits
function sumDigits(n) {
    let sum = 0;
    while (n > 0) {
        let digit = n % 10;
        sum = sum + digit;
        n = n / 10;
    }
    return sum;
}

function main() {
    let number = 432;
    let sum = sumDigits(number);
    // Expected output: 9
    return sum;
}
```

**Notes:**
- **Input Constraint:** Ensure `n` is within `-999` to `+999`. The function as written handles positive integers. If negative numbers are to be handled, consider using absolute values.

---

### Additional Testing Tips Considering the Integer Range:

1. **Input Limits:** Always use inputs that, after computation, stay within `-999` to `+999`. For example:
   - **Factorial:** Limit `n` to `6`.
   - **Fibonacci:** Limit `n` to `16`.
   - **Sum of Natural Numbers:** Limit `n` to `44`.
   
2. **Overflow Handling:** Test scenarios where intermediate calculations might approach the limits. For example, in the `reverseNumber` function, reversing `999` yields `999`, which is within range, but ensure that larger inputs aren't used.

3. **Negative Numbers:**
   - If your compiler supports negative numbers, test functions like `isPrime`, `gcd`, and `sumDigits` with negative inputs to see how they behave.
   - Adjust functions if necessary to handle negatives appropriately, such as taking absolute values where needed.

4. **Edge Cases:**
   - **Zero:** Test functions with `0` as input where applicable, e.g., `factorial(0)`, `fibonacci(0)`, `gcd(a, 0)`, etc.
   - **Maximum and Minimum Values:** Test with `999` and `-999` to ensure no overflow occurs and that the compiler correctly handles these extremes.

5. **Control Structures:**
   - Verify that loops (`for`, `while`) and conditionals (`if`) handle boundary conditions correctly. For example, a loop that should run `n` times with `n = 0`, `n = 1`, `n = 999`, etc.

6. **Function Calls and Recursion:**
   - Ensure that recursive functions like `gcdRecursive` do not exceed the maximum recursion depth due to the input size limitations.
   - Test nested function calls to verify that the call stack is managed correctly within the compiler's constraints.

7. **Variable Scope:**
   - Test the differentiation between local and global variables by creating scenarios where variables with the same name exist in different scopes.
