// Marks unused words as a temporary variable to be accessed later.

evaluate ::tmpWord.nMarks(0)
evaluate ::tmpWord.nAllocations(0)

// Mark an unused word as a temporary variable that can be accessed
// later.
//
// var must be in DB 0x7e
macro markTmpWord(evaluate var) {
    assert(({var} >> 16) == 0x7e)

    constant ::tmpWords.{::tmpWord.nMarks}({var})

    global evaluate ::tmpWord.nMarks({::tmpWord.nMarks} + 1)
}

// Allocates a tmp Word to a given variable name.
//
// The variable will be located somewhere in DB 0x7e
macro allocateTmpWord(name) {
    constant {name}(::tmpWords.{::tmpWord.nAllocations})

    global evaluate ::tmpWord.nAllocations({::tmpWord.nAllocations} + 1)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

