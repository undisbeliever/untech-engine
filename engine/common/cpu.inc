// common/cpu.inc
// ==============
//
// Provides a mechanism for managing the processor register sizes.
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

