
// Turns the current scope into a struct.
//
// usage:
//    scope point {
//      struct()
//        struct_maxsize(4)  // optional
//
//        field(xPos, 2)
//        field(yPos, 2)
//
//      endstruct()
//    }
//
//
// Basic struct inheritance can be achieved with the `basestruct` and
// `childstruct` macros.
//
// NOTE: Inner scopes will not work with struct inheritance.
//
// usage:
//    scope super {
//      basestruct(super)
//        struct_maxsize(16)  // optional
//        field(a, 2)
//      endstruct()
//    }
//    scope child {
//      childstruct(super)
//        field(b, 2)
//      endstruct()
//    }
//
// Child scope will contain `a` and `b` fields.
//

evaluate ::struct(0)
evaluate ::struct.inBase(0)

macro struct(evaluate start) {
    if {defined __struct} {
        error "scope is already a struct"
    }
    if {::struct} != 0 {
        error "Cannot nest structs"
    }

    global evaluate ::struct(0x1337)
    global evaluate ::struct.inBase(0)
    global evaluate ::struct.pos({start})
    global evaluate ::struct.size(0)
    global evaluate ::struct.maxSize(0)
}
macro struct() {
    struct(0)
}

macro basestruct(name) {
    if {defined ::struct.bases.{name}.size} {
        error "basestruct {name} already exists."
    }

    struct()

    global evaluate ::struct.inBase(1)
    global define ::struct.currentBase({name})

    global evaluate ::struct.bases.{name}.size(0)
}

macro basestruct(name, parent) {
    if {defined ::struct.bases.{name}.size} {
        error "basestruct `{name}` already exists."
    }

    childstruct({parent})

    global evaluate ::struct.inBase(1)
    global define ::struct.currentBase({name})

    global evaluate ::struct.bases.{name}.size(0)
    global define ::struct.bases.{name}.parent({parent})
}

macro childstruct(parent) {
    if !{defined ::struct.bases.{parent}.size} {
        error "basestruct `{parent}` does not exist"
    }
    struct()
    _childstruct_parents({parent})
}

macro _childstruct_parents(parent) {
    if {defined ::struct.bases.{parent}.parent} {
        _childstruct_parents({::struct.bases.{parent}.parent})
    }
    if {defined ::struct.bases.{parent}.maxSize} {
        struct_maxsize({::struct.bases.{parent}.maxSize})
    }
    _childstruct_fields({parent}, 0)
}

macro _childstruct_fields(parent, evaluate n) {
    if {n} < {::struct.bases.{parent}.size} {
        field({::struct.bases.{parent}.var{n}}, {::struct.bases.{parent}.var{n}size})

        _childstruct_fields({parent}, {n} + 1)
    }
}

// Creates a field in a struct of a given size.
macro field(name, evaluate size) {
    if {::struct} != 0x1337 {
        error "Not in a struct"
    }
    if {::struct.inBase} {
        evaluate s({::struct.bases.{::struct.currentBase}.size})
        global define ::struct.bases.{::struct.currentBase}.var{s}({name})
        global evaluate ::struct.bases.{::struct.currentBase}.var{s}size({size})
        global evaluate ::struct.bases.{::struct.currentBase}.size({s} + 1)
    }

    assert({size} > 0)

    constant {name}({::struct.pos})
    global evaluate ::struct.pos({::struct.pos} + {size})
    global evaluate ::struct.size({::struct.size} + {size})
}

// Limits the size of the current struct to `maxSize` bytes
macro struct_maxsize(evaluate maxSize) {
    assert({maxSize} > 0)

    if {::struct} != 0x1337 {
        error "Not in a struct"
    }
    if {::struct.maxSize} > 0 {
        error "struct is already size limited"
    }
    if {::struct.inBase} {
        global evaluate ::struct.bases.{::struct.currentBase}.maxSize({maxSize})
    }
    global evaluate ::struct.maxSize({maxSize})
}

// Ends the struct, will create the `size` field.
// Also errors out if the struct is too large
macro endstruct() {
    if {::struct} != 0x1337 {
        error "Not in a struct"
    }

    if {::struct.maxSize} > 0 && {::struct.size} > {::struct.maxSize} {
        error "struct is too large, (size {::struct.size}, max {::struct.maxSize})"
    }

    global define __struct(1)
    global evaluate ::struct(0)
    constant size({::struct.size})
}

// vim: ft=asm ts=4 sw=4 et:

