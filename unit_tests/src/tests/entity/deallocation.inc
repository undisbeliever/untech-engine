// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {


a16()
i16()
code()
Test.add("Entities.DeleteEntitiesInActivateNextFrameList")
function DeleteEntitiesInActivateNextFrameList {
constant tmp = Test.tmp

    jsr     Entity.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft


    // Spawn many entities (none of which would be activated)

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS)

    lda.w   #Entities.BlankEntity.EntityDataId_0
    Loop:
        sta.w   tmp

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #Entities.BlankEntity.EntityDataId_0 + Entity.N_ENTITY_LISTS
        bcc     Loop


    jsr     _ActivateNextFrameListLength
    cmp.w   #Entity.N_ENTITY_LISTS
    bne     Fail


    jsr     Entity.DeleteEntitiesInActivateNextFrameList


    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES
    bne     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.DeleteDeactivatedEntities")
function DeleteDeactivatedEntities {
constant N_ENTITIES_TO_SPAWN = 15
constant counter = Test.tmp

assert(N_ENTITIES_TO_SPAWN < Entity.N_ENTITIES + 2)


    jsr     Entity.Init

    // Spawn N_ENTITIES_TO_SPAWN DeactivateIfOutsideEntity entities offscreen

    lda.w   #0x8000
    sta.w   Camera.xPos
    sta.w   Camera.yPos

    // update inactive window
    jsr     ProcessEntityGameLoop


    lda.w   #N_ENTITIES_TO_SPAWN
    sta.w   counter


    Loop:
        lda.w   #0x1000
        sta.w   Entity.Spawn.xPos
        sta.w   Entity.Spawn.yPos

        lda.w   #Entities.DeactivateIfOutsideEntity.EntityDataId_0
        jsr     Entity.Spawn
        bcc     Fail

        dec.w   counter
        bne     Loop


    jsr     _DeactivatedListLength
    cmp.w   #N_ENTITIES_TO_SPAWN
    bne     Fail


    jsr     Entity.DeleteDeactivatedEntities


    jsr     _DeactivatedListLength
    cmp.w   #0
    bne     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.DeleteEntitiesInList")
function DeleteEntitiesInList {
constant tmp = Test.tmp

constant ENTITIES_PER_LIST = 3
constant TEST_LIST_ID = 2

assert(Entity.N_ENTITY_LISTS * ENTITIES_PER_LIST < Entity.N_ENTITIES + 2)
assert(Entity.N_ENTITY_LISTS > 2)


    jsr     Entity.Init

    // Spawn 3 entities per list

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    lda.w   #Entities.BlankEntity.EntityDataId_0
    Loop:
        sta.w   tmp

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        jsr     Entity.Spawn
        bcc     Fail

        assert(ENTITIES_PER_LIST == 3)

        lda.w   tmp
        inc
        cmp.w   #Entities.BlankEntity.EntityDataId_0 + Entity.N_ENTITY_LISTS
        bcc     Loop


    jsr     ProcessEntityGameLoop


    ldx.w   #ListLengthTable_Initial
    jsr     _TestListLengthTable
    bcc     Fail

    lda.w   #TEST_LIST_ID
    jsr     Entity.DeleteEntitiesInList

    ldx.w   #ListLengthTable_AfterDelete
    jsr     _TestListLengthTable
    bcc     Fail

    // test calling routine on empty list works
    lda.w   #TEST_LIST_ID
    jsr     Entity.DeleteEntitiesInList

    ldx.w   #ListLengthTable_AfterDelete
    jsr     _TestListLengthTable
    bcc     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts


rodata(rom0)
ListLengthTable_Initial:
    evaluate n = 0
    while {n} < Entity.N_ENTITY_LISTS {
        dw  ENTITIES_PER_LIST
        evaluate n = {n} + 1
    }

rodata(rom0)
ListLengthTable_AfterDelete:
    evaluate n = 0
    while {n} < Entity.N_ENTITY_LISTS {
        dw {n} != TEST_LIST_ID ? ENTITIES_PER_LIST : 0
        evaluate n = {n} + 1
    }
}



a16()
i16()
code()
Test.add("Entities.DeleteEntitiesOutsideRect")
function DeleteEntitiesOutsideRect {
constant testIndex   = Test.tmp
constant spawnIndex  = Test.tmp + 2

    lda.w   #0
    TestLoop:
        sta.w   testIndex

        jsr     _SpawnEntities
        bcc     Fail

        // Set Rect
        ldx.w   testIndex
        lda.l   TestTable.left,x
        sta.b   Entity.DeleteEntitiesOutsideRect.left

        lda.l   TestTable.right,x
        sta.b   Entity.DeleteEntitiesOutsideRect.right

        lda.l   TestTable.top,x
        sta.b   Entity.DeleteEntitiesOutsideRect.top

        lda.l   TestTable.bottom,x
        sta.b   Entity.DeleteEntitiesOutsideRect.bottom


        // Invoke DeleteEntitiesOutsideRect
        jsr     Entity.DeleteEntitiesOutsideRect

        jsr     _TestEntityListsValid
        bcc     Fail


        // Test list counts
        jsr     _DeactivatedListLength
        ldx.w   testIndex
        cmp.l   TestTable.nDeactivated,x
        bne     Fail

        txa
        clc
        adc.w   #TestTable.listLengthTable
        tax
        jsr     _TestListLengthTable
        bcc     Fail


        // Goto next element in table
        lda.w   testIndex
        clc
        adc.w   #TestTable.rowSize
        cmp.w   #TestTable.tableSize
        bcc     TestLoop


    jmp     _TestEntityListsValid


Fail:
    clc
    rts


// Spawn all the entities in the SpawnTable
function _SpawnEntities {
    jsr     Entity.Init

    // Move camera outside spawn area
    lda.w   #0x8000
    sta.w   Camera.xPos
    sta.w   Camera.yPos
    jsr     ProcessEntityGameLoop

    // Spawn entities in SpawnTable
    ldx.w   #0
    SpawnLoop:
        stx.w   spawnIndex

        lda.l   SpawnTable.xPos,x
        sta.w   Entity.Spawn.xPos

        lda.l   SpawnTable.yPos,x
        sta.w   Entity.Spawn.yPos

        lda.l   SpawnTable.entityDataId,x

        jsr     Entity.Spawn
        bcc     Fail


        lda.w   spawnIndex
        clc
        adc.w   #SpawnTable.rowSize
        tax
        cpx.w   #SpawnTable.tableSize
        bcc     SpawnLoop


    jsr     ProcessEntityGameLoop

    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - (SpawnTable.tableSize / SpawnTable.rowSize)
    bcc     Fail

    sec
    rts


Fail:
    clc
    rts
}


rodata(rom0)
SpawnTable:
namespace SpawnTable {
    basestruct_offset(SpawnTable)
        field(xPos, 2)
        field(yPos, 2)
        field(entityDataId, 2)
    endstruct()
    constant rowSize = size;

    assert(Entity.N_ENTITY_LISTS > 5)

    evaluate y = 1
    while {y} <= 4 {
        evaluate x = 1
        while {x} <= 4 {
            evaluate listId = {x}
            dw {x} * 0x0100, {y} * 0x0100, Entities.BlankEntity.EntityDataId_{listId}
            dw {x} * 0x0100, {y} * 0x0100, Entities.DeactivateIfOutsideEntity.EntityDataId_{listId}

            evaluate x = {x} + 1
        }
        evaluate y = {y} + 1
    }

    constant tableSize = pc() - SpawnTable
    assert(tableSize % rowSize == 0)
}


rodata(rom0)
TestTable:
namespace TestTable {
    basestruct_offset(TestTable)
        field(left, 2)
        field(right, 2)
        field(top, 2)
        field(bottom, 2)
        field(nDeactivated, 2)
        field(listLengthTable, 2 * Entity.N_ENTITY_LISTS)
    endstruct()
    constant rowSize = size;

    macro row(left, right, top, bottom, evaluate n1, evaluate n2, evaluate n3, evaluate n4) {
        dw  {left}, {right}, {top}, {bottom}        // rect
        dw  {n1} + {n2} + {n3} + {n4}               // nDeactivated
        dw  0, {n1}, {n2}, {n3}, {n4}, 0, 0, 0      // listLengthTable
    }
    assert(Entity.N_ENTITY_LISTS == 8)

    // The Spawn table will spawn the following blank entities
    // in the given position and list Id
    //
    //   yPos/xPos  0x0100  0x0200  0x0300  0x0400
    //   0x0100     list1   list2   list3   list4
    //   0x0200     list1   list2   list3   list4
    //   0x0300     list1   list2   list3   list4
    //   0x0400     list1   list2   list3   list4

    // All entities inside bounds
    row(0x00c0, 0x0c00, 0x00c0, 0x0c00,     4, 4, 4, 4)

    // All entity outside bounds
    // (test X axis)
    row(0x0240, 0x02c0, 0x00c0, 0x0c00,     0, 0, 0, 0)

    // All entity outside bounds
    // (test Y axis)
    row(0x00c0, 0x0c00, 0x0240, 0x02c0,     0, 0, 0, 0)

    // Delete the top/bottom most entities
    // (Keep yPos = 0x0200, 0x0300)
    row(0x00c0, 0x0c00, 0x01c0, 0x03c0,     2, 2, 2, 2)

    // Keep the top row
    // (Keep yPos = 0x0100)
    row(0x00c0, 0x0c00, 0x00c0, 0x01c0,     1, 1, 1, 1)

    // Keep the bottom row
    // (Keep yPos = 0x0300)
    row(0x00c0, 0x0c00, 0x02c0, 0x03c0,     1, 1, 1, 1)

    // Delete the left/right most entities
    // (Keep xPos = 0x0200, 0x0300)
    row(0x01c0, 0x03c0, 0x00c0, 0x0c00,     0, 4, 4, 0)

    // Keep the left-most entities
    row(0x00c0, 0x01c0, 0x00c0, 0x0c00,     4, 0, 0, 0)

    // Keep the right-most entities
    row(0x03c0, 0x04c0, 0x00c0, 0x0c00,     0, 0, 0, 4)

    // Keep the top-left entity
    row(0x00c0, 0x01c0, 0x00c0, 0x01c0,     1, 0, 0, 0)

    // Keep the top-right entity
    row(0x03c0, 0x04c0, 0x00c0, 0x01c0,     0, 0, 0, 1)

    // Keep the bottom-right 2x2 entities
    row(0x02c0, 0x0c00, 0x02c0, 0x0c00,     0, 0, 2, 2)

    // Keep the bottom-right 3x3 entities
    row(0x01c0, 0x0c00, 0x01c0, 0x0c00,     0, 3, 3, 3)

    constant tableSize = pc() - TestTable
    assert(tableSize % rowSize == 0)
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

