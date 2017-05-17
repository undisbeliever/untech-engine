// common/assert.inc
// =================
//
// Macros that break assembly if a test fails.


// Asserts than an expression is true
// Breaks assembly if expression if false
macro assert(test) {
    if !({test}) {
        error "{test}"
    }
}

// Asserts than an expression is a power of two
// Breaks assembly if expression if false
macro assertPowerOfTwo(evaluate test) {
    if ({test} & ({test} - 1)) != 0 {
        error "{test} is not a power of 2"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

