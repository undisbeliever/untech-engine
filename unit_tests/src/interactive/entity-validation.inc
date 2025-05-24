// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// This break should not be invoked
RegisterBreakId(ASSERT_FAILURE)


namespace InteractiveTests {
namespace EntityValidation {

// Tests:
//  * Entity.ValidateEntityLists

constant cursorIndex = Test.dpTmp + 0
constant menuLineIndex = Test.dpTmp + 2


constant N_MENU_ITEMS = 17


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


a16()
i16()
code()
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
            jsl     WaitFrame__far

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

    jsl     Entity.ValidateEntityLists__far

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

// Test last list
define ENTITY_LIST_ID = {EntityTest._SetupCounterTest.ENTITY_LIST_ID}

// Test ENTITY_LIST_ID is the last list
assert(EntityLists.{ENTITY_LIST_ID} == Entity.N_ENTITY_LISTS - 1)


rodata(rom0)
MenuText1:
    db  "InfiniteLoop (1st in list", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine1 {
    ldx.w   {Entity.listAddress({ENTITY_LIST_ID})}
    beq     Fail

    jmp     _CreateInfiniteLoop_Y

Fail:
    // list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText2:
    db  "InfiniteLoop (2nd in list", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine2 {
    ldx.w   {Entity.listAddress({ENTITY_LIST_ID})}
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
    ldy.w   Entity.specialLists.free

    jmp     _CreateInfiniteLoop_Y
}


rodata(rom0)
MenuText4:
    db  "Drop Entity", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine4 {
    // entityList->first = entityList->first->next

    ldx.w   {Entity.listAddress({ENTITY_LIST_ID})}
    beq     Fail

    lda.b   BaseEntity.next,x
    sta.w   {Entity.listAddress({ENTITY_LIST_ID})}

    rts

Fail:
    // list empty
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

    ldx.w   Entity.specialLists.deactivated
    beq     Fail

    lda.b   BaseEntity.next,x
    sta.w   Entity.specialLists.deactivated

    rts

Fail:
    // deactivated list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText6:
    db  "Deactivated in list 0", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine6 {
    sta.w   {Entity.listAddress(EntityList_0)}
    beq     Fail

    // bosses->first = deactivated->first
    // deactivated->first = null

    lda.w   Entity.specialLists.deactivated
    sta.w   {Entity.listAddress(EntityList_0)}

    stz.w   Entity.specialLists.deactivated

    rts

Fail:
    // boss list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText7:
    db  "Actv in activateNextFrame", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine7 {
    // test activateNextFrame empty
    // swap activateNextFrame and ENTITY_LIST_ID entity list

    // I want to test the list before the free list
    assert(Entity.specialLists.activateNextFrame == Entity.specialLists.free - 2)

    ldy.w   Entity.specialLists.activateNextFrame
    bne     Fail

    lda.w   {Entity.listAddress({ENTITY_LIST_ID})}
    sty.w   {Entity.listAddress({ENTITY_LIST_ID})}

    sta.w   Entity.specialLists.activateNextFrame

    rts

Fail:
    // unexpected null pointer
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText8:
    db  "Invalidate listId", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine8 {
    // list->next->listId = EntityLists.EntityList_4

    ldy.w   {Entity.listAddress({ENTITY_LIST_ID})}
    beq     Fail

    ldx.b   BaseEntity.next,y
    beq     Fail

    // X = second entity in list

    assert(EntityLists.{ENTITY_LIST_ID} != EntityLists.EntityList_4)
    sep     #$20
a8()
    lda.b   #EntityLists.EntityList_4
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
    // free->last->next = entityList->first
    // enemies->first = null

    ldy.w   Entity.specialLists.free
    beq     Fail

    Loop:
        tyx
        ldy.b   BaseEntity.next,x
        bne     Loop

    // X = last item in free list
    lda.w   {Entity.listAddress({ENTITY_LIST_ID})}
    sta.b   BaseEntity.next,x

    stz.w   {Entity.listAddress({ENTITY_LIST_ID})}

    rts

Fail:
    // free list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText10:
    db  "Invalid free functionTabl", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine10 {
    // free.first->next->next.functionTable = 0x1337

    ldx.w   Entity.specialLists.free
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


rodata(rom0)
MenuText11:
    db  "Corrupt instanceIdTable[0", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine11 {
    // Entity.instanceIdTable[0] = Entity.entityPool + ENTITY_SIZE * 20

    assert(Entity.N_ENTITIES > 21)
    constant entity = Entity.entityPool + Entity.ENTITY_SIZE * 20

    // Cell 0 should always be _unusedEntity

    lda.w   #entity
    sta.w   Entity.instanceIdTable

    // Ensure the instanceId -> entity mapping test passes
    stz.w   entity + BaseEntity.instanceId

    rts
}


rodata(rom0)
MenuText12:
    db  "Corrupt instanceIdTable", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine12 {
    // Entity.instanceIdTable[6] = 0x1337

    lda.w   #0x1337
    sta.w   Entity.instanceIdTable + 6 * 2

    rts
}


rodata(rom0)
MenuText13:
    db  "Dup instanceIdTable cell", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine13 {
    // assert(instanceIdTable[120] < 0x8000)
    // assert(instanceIdTable[124] < 0x8000)
    // instanceIdTable[120] = instanceIdTable[124]

    assert(124 * 2 < Entity.MAX_INSTANCE_ID)
    constant OFFSET_A = 120 * 2
    constant OFFSET_B = 124 * 2

    lda.w   Entity.instanceIdTable + OFFSET_A
    bmi     Fail

    ldy.w   Entity.instanceIdTable + OFFSET_B
    bmi     Fail

    sta.w   Entity.instanceIdTable + OFFSET_B

    rts

Fail:
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText14:
    db  "Zero instanceIdTable cell", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine14 {
    // assert(instanceIdTable[123] != _unusedEntity)
    // instanceIdTable[123] = _unusedEntity

    constant OFFSET = 123 * 2

    lda.w   #Entity._unusedEntity
    cmp.w   Entity.instanceIdTable + OFFSET
    beq     Fail
    sta.w   Entity.instanceIdTable + OFFSET

    rts

Fail:
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText15:
    db  "Corrupt instanceId", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine15 {
    // entityList.first->next->instanceId++

    ldy.w   {Entity.listAddress({ENTITY_LIST_ID})}
    beq     Fail

    ldx.w   BaseEntity.next,y
    beq     Fail

    sep     #$20
a8()
    inc.w   BaseEntity.instanceId,x
    rep     #$30
a16()

    rts

Fail:
    // boss list empty
    break(ASSERT_FAILURE)
}


rodata(rom0)
MenuText16:
    db  "Two entities with same id", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine16 {
    // entityList.first->next->instanceId = entityList.first->instanceId

    ldx.w   {Entity.listAddress({ENTITY_LIST_ID})}
    beq     Fail

    ldy.w   BaseEntity.next,x
    beq     Fail

    sep     #$20
a8()
    lda.w   BaseEntity.instanceId,x
    sta.w   BaseEntity.instanceId,y
    rep     #$30
a16()

    rts

Fail:
    // boss list empty
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

