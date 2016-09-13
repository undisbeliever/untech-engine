
// Asserts than an expression is true
// Breaks assembly if expression if false
macro assert(test) {
    if !({test}) {
        error "{test}"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

