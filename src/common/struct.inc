
// Turns the current scope into a struct.
//
// usage:
//    scope point {
//        struct()
//
//        field(xPos, 2)
//        field(yPos, 2)
//
//        endstruct()
//    }
//
// Unfortunately structs cannot be nested.

evaluate ::struct(0)

macro struct() {
    if {defined __struct} {
        error "scope is already a struct"
    }
    if {::struct} != 0 {
        error "Cannot nest structs"
    }

    global evaluate ::struct(0x1337)
    global evaluate ::struct.size(0)
}

// Creates a field in a struct of a given size.
macro field(name, size) {
    if {::struct} != 0x1337 {
        error "Not in a struct"
    }

    constant {name}({::struct.size})
    global evaluate ::struct.size({::struct.size} + {size})
}

// Ends the struct, will create the `size` field.
macro endstruct() {
    if {::struct} != 0x1337 {
        error "Not in a struct"
    }

    global define __struct(1)
    global evaluate ::struct(0)
    constant size({::struct.size})
}

// vim: ft=asm ts=4 sw=4 et:

