// cpu.inc
// =======
//
// Provides a mechanism for managing the processor register sizes
//

arch snes.cpu

evaluate ::cpu.pSet(0)
evaluate ::cpu.pClear(0)

scope private {
    macro setPFlags(evaluate v) {
        global evaluate ::cpu.pSet({::cpu.pSet} | {v})
        global evaluate ::cpu.pClear({::cpu.pClear} & (~{v}))
    }

    macro clearPFlags(evaluate v) {
        global evaluate ::cpu.pSet({::cpu.pSet} & (~{v}))
        global evaluate ::cpu.pClear({::cpu.pClear} | {v})
    }

    macro unknownPFlags(evaluate v) {
        global evaluate ::cpu.pSet({::cpu.pSet} & (~{v}))
        global evaluate ::cpu.pClear({::cpu.pClear} & (~{v}))
    }
}

macro sep(evaluate v) {
    private.setPFlags({v})

    sep #{v}
}

macro rep(evaluate v) {
    private.clearPFlags({v})

    rep #{v}
}

macro a8() {
    private.setPFlags($20)
}

macro a16() {
    private.clearPFlags($20)
}

macro au() {
    private.unknownPFlags($20)
}

macro i8() {
    private.setPFlags($10)
}

macro i16() {
    private.clearPFlags($10)
}

macro iu() {
    private.unknownPFlags($10)
}

macro punknown() {
    private.unknownPFlags($FF)
}

macro assert8a() {
    if !({::cpu.pSet} & $20) || ({::cpu.pClear} & $20) {
        error "require an 8 bit A"
    }
}

macro assert16a() {
    if ({::cpu.pSet} & $20) || !({::cpu.pClear} & $20) {
        error "require a 16 bit A"
    }
}

macro assert8i() {
    if !({::cpu.pSet} & $10) || ({::cpu.pClear} & $10) {
        error "require an 8 bit Index register"
    }
}

macro assert16i() {
    if ({::cpu.pSet} & $10) || !({::cpu.pClear} & $10) {
        error "require a 16 bit Index register"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

