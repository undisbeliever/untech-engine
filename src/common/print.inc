// print.inc
// =========
//
// Macros for extended printing to the screen


// Prints a decimal integer number with padding
//
// PARAMS:
//  * width - the printing width
//  * value - the value to print
macro print_padded(evaluate width, evaluate value) {
    __print_padded_spaces_loop({width} - 1, {value})
    print {value}
}

macro __print_padded_spaces_loop(evaluate w, evaluate i) {
    if {i} < 0 {
        __print_padded_spaces_loop({w} - 1, 0 - {i})
    } else if {w} > 0 {
        if {i} < 10 {
            print " "
        }
        __print_padded_spaces_loop({w} - 1, {i} / 10)
    }
}

// Prints a hexadecimal number with padding
//
// NOTE: cannot print negative numbers
//
// PARAMS:
//  * width - the printing width
//  * value - the value to print
macro print_hex(evaluate width, evaluate value) {
    if {width} > 0 || {value} > 0 {
        print_hex({width} - 1, {value} / 16)

        evaluate d({value} % 16)
        if {d} < 10 {
            print {d}
        } else if {d} == 10 {
            print "a"
        } else if {d} == 11 {
            print "b"
        } else if {d} == 12 {
            print "c"
        } else if {d} == 13 {
            print "d"
        } else if {d} == 14 {
            print "e"
        } else if {d} == 15 {
            print "f"
        }
    }
}

macro print_hex(evaluate value) {
    print_hex(1, {value})
}

// vim: ft=bass-65816 ts=4 sw=4 et:

