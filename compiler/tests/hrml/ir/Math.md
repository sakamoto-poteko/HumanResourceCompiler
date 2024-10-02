Below are the revised math problems and corresponding code snippets tailored to fit within compiler's limitations.

---

### 1. **Factorial of a Number**

**Problem:** Compute the factorial of a given non-negative integer `n`.  
**Known Solution:** `n! = n × (n-1) × ... × 1`  
**Example:** `5! = 120`

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
    let number = 5;
    let fact = factorial(number);
    // Expected output: 120
    return fact;
}
```

**Notes:**
- **Input Constraint:** Ensure `n` is such that `n!` does not exceed `999`. For example, `n` can be up to `6` since `6! = 720` and `7! = 5040` exceeds the limit.

---

### 2. **Fibonacci Sequence**

**Problem:** Compute the `n`-th Fibonacci number.  
**Known Solution:** `F(n) = F(n-1) + F(n-2)` with `F(0)=0` and `F(1)=1`  
**Example:** `F(10) = 55`

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
    let number = 10;
    let fib = fibonacci(number);
    // Expected output: 55
    return fib;
}
```

**Notes:**
- **Input Constraint:** Use `n` up to `16` to ensure `F(n)` does not exceed `999` (since `F(16) = 987` and `F(17) = 1597` exceeds the limit).

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
- **Range Compliance:** Any `n` within `-999` to `+999` can be tested, but typically primes are positive integers greater than `1`.

---

### 4. **Greatest Common Divisor (GCD)**

**Problem:** Find the greatest common divisor of two integers `a` and `b`.  
**Known Solution:** Using the Euclidean algorithm.  
**Example:** `GCD(48, 18) = 6`

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
- **Input Constraint:** Ensure that `a` and `b` are within `-999` to `+999`. The GCD function works correctly with positive integers; if negatives are used, consider taking absolute values before processing.

---

### 5. **Sum of First N Natural Numbers**

**Problem:** Calculate the sum of the first `n` natural numbers.  
**Known Solution:** `Sum = n*(n+1)/2`  
**Example:** Sum of first `44` natural numbers is `990`

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
**Known Solution:** Reversing the digits of `1234` yields `4321`  
**Example:** `Reverse(123) = 321`

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

### 7. **Sum of Digits**

**Problem:** Calculate the sum of the digits of an integer `n`.  
**Known Solution:** Sum of digits of `432` is `9`  
**Example:** `SumDigits(432) = 9`

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

### 8. **Bubble Sort (Sorting Five Numbers Manually)**

**Problem:** Sort five integers in ascending order using the Bubble Sort algorithm.  
**Known Solution:** Repeatedly swapping adjacent elements if they are in the wrong order.  
**Example:** Sorting `5, 3, 8, 4, 2` results in `2, 3, 4, 5, 8`

**Note:** Since arrays are not supported, we'll handle five separate variables (`a`, `b`, `c`, `d`, `e`) and perform swaps manually.

```c
// Bubble Sort for five variables without using arrays
function bubbleSortFive(a, b, c, d, e) {
    let swapped;
    do {
        swapped = 0;
        // Compare and swap a and b
        if (a > b) {
            let temp = a;
            a = b;
            b = temp;
            swapped = 1;
        }
        // Compare and swap b and c
        if (b > c) {
            let temp = b;
            b = c;
            c = temp;
            swapped = 1;
        }
        // Compare and swap c and d
        if (c > d) {
            let temp = c;
            c = d;
            d = temp;
            swapped = 1;
        }
        // Compare and swap d and e
        if (d > e) {
            let temp = d;
            d = e;
            e = temp;
            swapped = 1;
        }
    } while (swapped == 1);
    return a; // Returning the first element as a representative
}

function main() {
    let a = 5;
    let b = 3;
    let c = 8;
    let d = 4;
    let e = 2;
    let first = bubbleSortFive(a, b, c, d, e);
    // Expected first element after sorting: 2
    return first;
}
```

**Explanation:**
- The `bubbleSortFive` function manually swaps the variables `a`, `b`, `c`, `d`, and `e` if they are out of order.
- The `do-while` loop continues until no swaps are needed, indicating that the numbers are sorted.
- The function returns the first element (`a`) after sorting, which should be the smallest number (`2` in the example).

**Notes:**
- **Scalability:** This approach is limited to sorting exactly five numbers. For more numbers, additional variables and swap conditions would need to be added.
- **Verification:** Since returning multiple values isn't straightforward without arrays, you can modify the function to return one or more sorted values or implement additional return statements as needed.

---

### 9. **Binary Search (Without Arrays)**

**Problem:** Since arrays are not supported, implementing a binary search isn't feasible. Instead, we can perform a simple search for a target value among individual variables.

**Alternative Example:** **Find the Maximum of Five Numbers**

**Known Solution:** Identify the largest number among five given integers.

**Example:** Maximum of `5, 3, 8, 4, 2` is `8`

```c
// Find the maximum of five numbers
function findMax(a, b, c, d, e) {
    let max = a;
    if (b > max) {
        max = b;
    }
    if (c > max) {
        max = c;
    }
    if (d > max) {
        max = d;
    }
    if (e > max) {
        max = e;
    }
    return max;
}

function main() {
    let a = 5;
    let b = 3;
    let c = 8;
    let d = 4;
    let e = 2;
    let maximum = findMax(a, b, c, d, e);
    // Expected output: 8
    return maximum;
}
```

**Notes:**
- **Alternative to Binary Search:** Since arrays aren't available, searching within individual variables is limited. However, finding the maximum or minimum can still be tested.

---

### 10. **Palindrome Checker (Without Arrays)**

**Problem:** Determine whether a given integer `n` is a palindrome (reads the same backward as forward).  
**Known Solution:** Reverse the number and compare it with the original.

**Example:** `121` is a palindrome, `123` is not.

```c
// Palindrome checker
function isPalindrome(n) {
    let original = n;
    let reversed = 0;
    while (n > 0) {
        let digit = n % 10;
        reversed = reversed * 10 + digit;
        n = n / 10;
    }
    if (original == reversed) {
        return 1; // True
    } else {
        return 0; // False
    }
}

function main() {
    let number = 121;
    let palindrome = isPalindrome(number);
    // Expected output: 1 (true)
    return palindrome;
}
```

**Notes:**
- **Input Constraint:** Ensure `n` is within `-999` to `+999`. The function handles positive integers. For negative numbers, you might want to take the absolute value before processing.

---

### Tips for Testing Without Arrays:

1. **Multiple Variables Handling:** When testing functions that traditionally use arrays (like sorting), manually handle a fixed number of variables. This helps verify your compiler's ability to manage multiple variables and nested control structures.

2. **Edge Cases:** Test with edge cases such as `0`, `1`, and `999` to ensure correct handling within the specified range.

3. **Control Structures:** Verify the correct implementation of `if`, `for`, and `while` by designing test cases that heavily utilize these constructs.

4. **Function Calls and Recursion:** Test nested function calls and recursive functions to ensure proper call stack management. For example, the recursive GCD function is a good test case.

5. **Variable Scope:** Create scenarios with variables of the same name in different scopes (global vs. local) to ensure that variable scope is correctly handled.

6. **Arithmetic Operations:** Ensure that all basic arithmetic operations (`+`, `-`, `*`, `/`, `%`) work as expected within the integer range.

7. **Overflow Handling:** Since the integer range is limited, test scenarios that approach the boundaries (e.g., operations resulting in `999` or `-999`) to verify that overflow is handled appropriately.
