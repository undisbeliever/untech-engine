// common/cpu.inc
// ==============
//
// Provides a mechanism for managing the processor register sizes.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


variable __CPU__.aSize = 0
variable __CPU__.iSize = 0

macro sep(evaluate v) {
    if ({v} & 0x20) {
        __CPU__.aSize = 8
    }
    if ({v} & 0x10) {
        __CPU__.iSize = 8
    }

    sep #{v}
}

macro rep(evaluate v) {
    if ({v} & 0x20) {
        __CPU__.aSize = 16
    }
    if ({v} & 0x10) {
        __CPU__.iSize = 16
    }

    rep #{v}
}

macro a8() {
    __CPU__.aSize = 8
}

macro a16() {
    __CPU__.aSize = 16
}

macro au() {
    __CPU__.aSize = 0
}

macro i8() {
    __CPU__.iSize = 8
}

macro i16() {
    __CPU__.iSize = 16
}

macro iu() {
    __CPU__.iSize = 0
}

macro punknown() {
    __CPU__.aSize = 0
    __CPU__.iSize = 0
}

macro assert8a() {
    if __CPU__.aSize != 8 {
        error "require an 8 bit A"
    }
}

macro assert16a() {
    if __CPU__.aSize != 16 {
        error "require a 16 bit A"
    }
}

macro assert8i() {
    if __CPU__.iSize != 8 {
        error "require an 8 bit Index register"
    }
}

macro assert16i() {
    if __CPU__.iSize != 16 {
        error "require a 16 bit Index register"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

