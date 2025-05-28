// common/struct.inc
// =================
//
// Turns the current scope into a struct.
//
// usage:
//    namespace point {
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
//    namespace super {
//      basestruct()
//        struct_maxsize(16)  // optional
//        field(a, 2)
//      endstruct()
//    }
//    namespace child {
//      childstruct(super)
//        field(b, 2)
//      endstruct()
//    }
//
// Child scope will contain `a` and `b` fields.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


inline struct(evaluate start) {
    if {defined __STRUCT__} {
        error "scope is already a struct"
    }

    define __STRUCT__

    constant __STRUCT__.start = {start}
    variable __STRUCT__.inStruct = 0x1337
    variable __STRUCT__.isBaseStruct = 0
    variable __STRUCT__.pos = {start}
    variable __STRUCT__.size = 0
    variable __STRUCT__.maxSize = 0
}
inline struct() {
    struct(0)
}

inline basestruct_offset(evaluate start) {
    struct({start})

    __STRUCT__.isBaseStruct = 1
    define __STRUCT__.fields.size = 0
}
inline basestruct() {
    basestruct_offset(0)
}

// raises an error is the struct is no a basestruct
macro validateStructIsBase(struct) {
    if !{defined {struct}.__STRUCT__} {
        error "struct `{struct}` does not exist"
    }
    if {struct}.__STRUCT__.isBaseStruct == 0 {
        error "struct `{struct}` is not a basestruct"
    }
    if {struct}.__STRUCT__.inStruct {
        error "basestruct `{struct}` is not closed"
    }
}

inline basestruct(parent) {
    validateStructIsBase({parent})

    basestruct_offset({parent}.__STRUCT__.start)
    _childstruct_fields({parent})
}

inline childstruct(parent) {
    validateStructIsBase({parent})

    struct({parent}.__STRUCT__.start)
    _childstruct_fields({parent})
}

inline _childstruct_fields(parent) {
    __STRUCT__.maxSize = {parent}.__STRUCT__.maxSize

    evaluate n = 0
    while {n} < {{parent}.__STRUCT__.fields.size} {
        field({{parent}.__STRUCT__.fields.{n}.name}, {{parent}.__STRUCT__.fields.{n}.size})

        evaluate n = {n} + 1
    }

    assert(__STRUCT__.pos == {parent}.__STRUCT__.pos)
}

// Creates a field in a struct of a given size.
inline field(name, evaluate size) {
    if __STRUCT__.inStruct != 0x1337 {
        error "Not in a struct"
    }
    if __STRUCT__.isBaseStruct {
        define __STRUCT__.fields.{__STRUCT__.fields.size}.name = {name}
        evaluate __STRUCT__.fields.{__STRUCT__.fields.size}.size = {size}
        evaluate __STRUCT__.fields.size = {__STRUCT__.fields.size} + 1
    }

    assert({size} > 0)

    constant {name} = __STRUCT__.pos
    __STRUCT__.pos = __STRUCT__.pos + {size}
    __STRUCT__.size = __STRUCT__.size + {size}

    if __STRUCT__.maxSize > 0 && __STRUCT__.size > __STRUCT__.maxSize {
        evaluate size = __STRUCT__.size
        evaluate maxSize = __STRUCT__.maxSize
        error "struct is too large, (size {size}, max {maxSize})"
    }
}

// Limits the size of the current struct to `maxSize` bytes
inline struct_maxsize(evaluate maxSize) {
    assert({maxSize} > 0)

    if __STRUCT__.inStruct != 0x1337 {
        error "Not in a struct"
    }
    if __STRUCT__.maxSize > 0 {
        error "struct is already size limited"
    }
    __STRUCT__.maxSize = {maxSize}
}

// Ends the struct, will create the `size` field.
// Also errors out if the struct is too large
inline endstruct() {
    if __STRUCT__.inStruct != 0x1337 {
        error "Not in a struct"
    }

    if __STRUCT__.maxSize > 0 && __STRUCT__.size > __STRUCT__.maxSize {
        evaluate size = __STRUCT__.size
        evaluate maxSize = __STRUCT__.maxSize
        error "struct is too large, (size {size}, max {maxSize})"
    }

    __STRUCT__.inStruct = 0
    define __STRUCT__ = 1

    constant size = __STRUCT__.size
}

// vim: ft=bass-65816 ts=4 sw=4 et:

