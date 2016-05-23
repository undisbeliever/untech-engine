
// Asserts than an expression is true
// Breaks assembly if expression if false
macro assert(test) {
    if !({test}) {
        error "{test}"
    }
}

// vim: ft=asm ts=4 sw=4 et:

