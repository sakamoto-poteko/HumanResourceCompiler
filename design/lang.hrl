import lang_mod;

// only single line comments allowed
init floor[1] = 1; // init floor[id] sets the initial value for a box on the floor
init floor[0] = 0;
init floor_max = 10;

// A function is defined, which takes an argument. 
// Only one or zero argument is allowed.
function countdown(a) {
    for (let i = a; i >= 0; --i) {
        outbox(i);
    }

    return a;
}

// sub marks a no-return function
sub no_return_func() {
    // and it does nothing
}

// the program starts here
sub start() {
    // a loop construct
    // int(true) = 1, int(false) = 0, boolean and int are basically the same thing
    // bool(1) = true, bool(0) = false, bool(anything except 0) = true
    while (true) {
        // inbox and outbox
        let a = inbox();
        let b = inbox();
        if (a > b) {
            outbox(a);
        } else {
            outbox(b);
        }
        let c = a * b;
        let num = countdown(a);
        floor[2] = num;
        floor[3] = floor[b] + c;
        outbox(c);
    }
}
