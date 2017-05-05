
scope InteractiveTests {
scope BreakOnSelect {

// Tests:
//  * Break Handler


a16()
i16()
code()
Test.add("Break on Select")
scope Test: {
    Text.Console.SetCursor(0, 4)
    Text.Console.PrintConstString("Press SELECT to trigger BRK\n")
    Text.Console.PrintConstString("Press A/B/X/Y to skip BRK")

    Loop:
        jsr     WaitFrame

        lda.l   JOY1
        bit.w   #JOY.select
        beq     +
            brk     #$42
        +

        // A = JOY1
        bit.w   #JOY.buttons
        beq     Loop

    jmp     Test.ResetPpuState
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

