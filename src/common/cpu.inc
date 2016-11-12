// cpu.inc
// =======
//
// Provides a mechanism for managing the processor register sizes
//

arch snes.cpu

evaluate ::cpu.aSize(0)
evaluate ::cpu.iSize(0)

macro sep(evaluate v) {
    if ({v} & 0x20) {
        global evaluate ::cpu.aSize(8)
    }
    if ({v} & 0x10) {
        global evaluate ::cpu.iSize(8)
    }

    sep #{v}
}

macro rep(evaluate v) {
    if ({v} & 0x20) {
        global evaluate ::cpu.aSize(16)
    }
    if ({v} & 0x10) {
        global evaluate ::cpu.iSize(16)
    }

    rep #{v}
}

macro a8() {
    global evaluate ::cpu.aSize(8)
}

macro a16() {
    global evaluate ::cpu.aSize(16)
}

macro au() {
    global evaluate ::cpu.aSize(0)
}

macro i8() {
    global evaluate ::cpu.iSize(8)
}

macro i16() {
    global evaluate ::cpu.iSize(16)
}

macro iu() {
    global evaluate ::cpu.iSize(0)
}

macro punknown() {
    global evaluate ::cpu.aSize(0)
    global evaluate ::cpu.iSize(0)
}

macro assert8a() {
    if {::cpu.aSize} != 8 {
        error "require an 8 bit A"
    }
}

macro assert16a() {
    if {::cpu.aSize} != 16 {
        error "require a 16 bit A"
    }
}

macro assert8i() {
    if {::cpu.iSize} != 8 {
        error "require an 8 bit Index register"
    }
}

macro assert16i() {
    if {::cpu.iSize} != 16 {
        error "require a 16 bit Index register"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

