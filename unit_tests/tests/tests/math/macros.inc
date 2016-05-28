
scope MathTest {

code()
a16()
i16()
Test.add("neg.b")
scope neg.b: {
    sep     #$20
a8()

    lda.b   #42
    Math.neg.b()

    cmp.b   #-42
    bne     Fail

    lda.b   #-120
    Math.neg.b()

    cmp.b   #120
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}



code()
a16()
i16()
Test.add("neg.w")
scope neg.w: {
    lda.w   #0x1337
    Math.neg.w()

    cmp.w   #-0x1337
    bne     Fail

    lda.w   #-1024
    Math.neg.w()

    cmp.w   #1024
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}



code()
a16()
i16()
Test.add("Negate32xy")
scope Negate32xy: {
    macro test(evaluate num) {
        ldy.w   #{num} & 0xffff
        ldx.w   #{num} >> 16

        Math.Negate32xy()

        cpy.w   #(0-{num}) & 0xffff
        bne     Fail
        cpx.w   #(0-{num}) >> 16
        bne     Fail
    }

    test(0)
    test(-1)
    test(0x1337beef)

    sec
    rts

Fail:
    clc
    rts
}



code()
a16()
i16()
Test.add("Negate32.b")
scope Negate32.b: {
    macro test(evaluate num) {
        lda.w   #{num} & 0xffff
        sta.b   Test.dpTmp
        lda.w   #{num} >> 16
        sta.b   Test.dpTmp + 2

        lda.w   #0

        Math.Negate32.b(Test.dpTmp)

        lda.b   Test.dpTmp
        cmp.w   #(0-{num}) & 0xffff
        bne     Fail

        lda.b   Test.dpTmp + 2
        cmp.w   #(0-{num}) >> 16
        bne     Fail
    }

    test(0)
    test(42)
    test(0xdeadbeef)

    sec
    rts

Fail:
    clc
    rts
}



code()
a16()
i16()
Test.add("Negate32.w")
scope Negate32.w: {
    macro test(evaluate num) {
        lda.w   #{num} & 0xffff
        sta.w   Test.tmp
        lda.w   #{num} >> 16
        sta.w   Test.tmp + 2

        lda.w   #0

        Math.Negate32.w(Test.tmp)

        lda.w   Test.tmp
        cmp.w   #(0-{num}) & 0xffff
        bne     Fail

        lda.w   Test.tmp + 2
        cmp.w   #(0-{num}) >> 16
        bne     Fail
    }

    test(0)
    test(0xffffffff)
    test(123)

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

