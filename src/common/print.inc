// common/print.inc
// ================
//
// Macros for extended printing to the screen.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Prints a decimal integer number with padding
//
// PARAMS:
//  * width - the printing width
//  * value - the value to print
macro print_padded(evaluate width, evaluate value) {
    variable w = {width} - 1
    variable v = {value}

    if v < 0 {
        v = 0 - v
        w = w - 1
    }

    while w > 0 {
        if v < 10 {
            print " "
        }
        w = w - 1
        v = v / 10
    }

    print {value}
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

        evaluate d = {value} % 16
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

