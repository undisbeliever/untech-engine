// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// This break should not be invoked
RegisterBreakId(ASSERT_FAILURE)


namespace InteractiveTests {
namespace EntityValidation {

// Tests:
//  * Entity.ValidateEntityLists

constant cursorIndex = Test.dpTmp + 0
constant menuLineIndex = Test.dpTmp + 2


constant N_MENU_ITEMS = 11


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _Init() {
    assert16a()
    assert16i()

    jsr     EntityTest._SetupCounterTest
    bcc     Fail

    jsr     EnableDisplay_Full
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _PrintMenu() {
    assert16a()
    assert16i()

    Text.Console.SetCursor(0, 4)

    ldx.w   #0

    Loop:
        stx.b   menuLineIndex

        cpx.b   cursorIndex
        bne     +
            ldx.w   #CursorString
            bra     ++
        +
            ldx.w   #NoCursorString
        +
        lda.w   #CursorString >> 16
        jsr     Text.Console.PrintString

        ldx.b   menuLineIndex
        lda.l   MenuTexts,x
        tax
        lda.w   #CursorString >> 16
        jsr     Text.Console.PrintString

        jsr     Text.Console.NewLine

        ldx.b   menuLineIndex
        inx
        inx
        cpx.w   #N_MENU_ITEMS * 2
        bcc     Loop
}


namespace Test {
Fail:
    clc
    rts
}

a16()
i16()
code()
Test.add("Entity Validation")
function Test {

    _Init()


    ldx.w   #0

    PrintMenuAndLoop:
        stx.b   cursorIndex

        _PrintMenu()

        Loop:
            jsr     WaitFrame

            Controller.UpdateControllerState()

            lda.w   Controller.Joy1.pressed
            beq     Loop


            ldx.b   cursorIndex

            bit.w   #JOY.up
            beq     +
                dex
                dex
                bpl     +
                    ldx.w   #(N_MENU_ITEMS - 1) * 2
            +
            // A = Joy1
            bit.w   #JOY.down
            beq     +
                inx
                inx
                cpx.w   #N_MENU_ITEMS * 2
                bcc     +
                    ldx.w   #0
            +

            // A = Joy1
            // X = new cursorIndex
            bit.w   #JOY.buttons
            bne     EndLoop

            jmp     PrintMenuAndLoop

EndLoop:
    // button pressed
    // X = new cursorIndex
    jsr     (MenuRoutines,x)

    jsr     Entity.ValidateEntityLists

    sec
    rts
}


rodata(rom0)
CursorString:
    db  " >", 0

NoCursorString:
    db  "  ", 0

rodata(rom0)
MenuTexts:
    evaluate n = 0
    while {n} < N_MENU_ITEMS {
        dw  MenuText{n}
        evaluate n = {n} + 1
    }

code()
MenuRoutines:
    evaluate n = 0
    while {n} < N_MENU_ITEMS {
        dw  MenuRoutine{n}
        evaluate n = {n} + 1
    }


rodata(rom0)
MenuText0:
    db  "None", 0
a16()
i16()
code()
function MenuRoutine0 {
    rts
}


rodata(rom0)
MenuText1:
    db  "InfiniteLoop (1st enemy)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine1 {
    ldx.w   {Entity.listAddress(ENEMIES)}
    beq     Fail

    jmp     _CreateInfiniteLoop_Y

Fail:
    // enemy list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText2:
    db  "InfiniteLoop (2nd enemy)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine2 {
    ldx.w   {Entity.listAddress(ENEMIES)}
    beq     Fail

    ldy.b   BaseEntity.next,x
    beq     Fail

    jmp     _CreateInfiniteLoop_Y

Fail:
    // unexpected null pointer
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText3:
    db  "InfiniteLoop (free list)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine3 {
    ldy.w   Entity.lists.free

    jmp     _CreateInfiniteLoop_Y
}


rodata(rom0)
MenuText4:
    db  "Drop Entity (enemy)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine4 {
    // enemy->first = enemy->first->next

    ldx.w   {Entity.listAddress(ENEMIES)}
    beq     Fail

    lda.b   BaseEntity.next,x
    sta.w   {Entity.listAddress(ENEMIES)}

    rts

Fail:
    // enemy list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText5:
    db  "Drop Entity (deactivated)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine5 {
    // deactivated->first = deactivated->first->next

    ldx.w   Entity.lists.deactivated
    beq     Fail

    lda.b   BaseEntity.next,x
    sta.w   Entity.lists.deactivated

    rts

Fail:
    // deactivated list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText6:
    db  "Deactivated in boss list", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine6 {
    sta.w   {Entity.listAddress(BOSSES)}
    beq     Fail

    // bosses->first = deactivated->first
    // deactivated->first = null

    lda.w   Entity.lists.deactivated
    sta.w   {Entity.listAddress(BOSSES)}

    stz.w   Entity.lists.deactivated

    rts

Fail:
    // boss list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText7:
    db  "Acti in activateNextFrame", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine7 {
    // test activateNextFrame empty
    // swap activateNextFrame and enemy lists

    // I want to test the list before the free list
    assert(Entity.lists.activateNextFrame == Entity.lists.free - 2)

    ldy.w   Entity.lists.activateNextFrame
    bne     Fail

    lda.w   {Entity.listAddress(ENEMIES)}
    sty.w   {Entity.listAddress(ENEMIES)}

    sta.w   Entity.lists.activateNextFrame

    rts

Fail:
    // unexpected null pointer
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText8:
    db  "Invalidate enemy listId", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine8 {
    // enemies->next->listId = EntityLists.PLAYERS

    ldy.w   {Entity.listAddress(PLAYERS)}
    beq     Fail

    ldx.b   BaseEntity.next,y
    beq     Fail

    // X = second entity in enemy list

    sep     #$20
a8()
    lda.b   #EntityLists.PLAYERS
    sta.b   BaseEntity.listId,x

    rep     #$30
a16()

    rts

Fail:
    // unexpected null pointer
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText9:
    db  "Activated in free list", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine9 {
    // free->last->next = enemies->first
    // enemies->first = null

    ldy.w   Entity.lists.free
    beq     Fail

    Loop:
        tyx
        ldy.b   BaseEntity.next,x
        bne     Loop

    // X = last item in free list
    lda.w   {Entity.listAddress(ENEMIES)}
    sta.b   BaseEntity.next,x

    stz.w   {Entity.listAddress(ENEMIES)}

    rts

Fail:
    // free list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText10:
    db  "Invalid free functionTable", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine10 {
    // free.first->next->next.functionTable = 0x1337

    ldx.w   Entity.lists.free
    beq     Fail

    ldy.b   BaseEntity.next,x
    beq     Fail

    lda.w   #0x1337
    sta.w   BaseEntity.functionTable,y

    rts

Fail:
    // unexpected null pointer
    break(ASSERT_FAILURE)
}



// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y = entity in list
a16()
i16()
code()
function _CreateInfiniteLoop_Y {
    tya
    beq     Fail

        Loop:
            tyx
            ldy.b   BaseEntity.next,x
            bne     Loop

        // A = parameter entity
        // X = last entity in list
        sta.b   BaseEntity.next,x

    rts

Fail:
    // unexpected null pointer
    break(ASSERT_FAILURE)
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

