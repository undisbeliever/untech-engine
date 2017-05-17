// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

scope EntityTest {

a16()
i16()
code()
Test.add("Entity.Spawn")
scope Spawn: {
    jsr     Entity.Init

    stz.w   Entities.BlankEntity.parameterSum

    lda.w   #0x1234
    sta.w   Entity.Spawn.xPos
    lda.w   #0x5678
    sta.w   Entity.Spawn.yPos

    lda.w   Entities.BlankEntity.EntityId_0
    ldy.w   #0x1337
    jsr     Entity.Spawn
    bcc     Fail

    // test x/y pos
    lda.w   BaseEntity.xPos.px,y
    cmp.w   #0x1234
    bne     Fail

    lda.w   BaseEntity.yPos.px,y
    cmp.w   #0x5678
    bne     Fail

    // test parameter worked
    lda.w   Entities.BlankEntity.parameterSum
    cmp.w   #0x1337
    bne     Fail

    // test list decreased by one
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - 1
    bne     Fail

    // Test first list length
    jsr     _FirstListLength
    cmp.w   #1
    bne     Fail

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Entity.Spawn (Overflow)")
scope Spawn_Overflow: {
constant tmp(Test.tmp)

    jsr     Entity.Init

    stz.w   Entities.BlankEntity.parameterSum

    ldx.w   #Entity.N_ENTITIES

    Loop:
        stx.w   tmp

        lda.w   Entities.BlankEntity.EntityId_0
        ldy.w   #5
        jsr     Entity.Spawn
        bcc     Fail

        ldx.w   tmp
        dex
        bne     Loop


    // Test cannot make new entity
    lda.w   Entities.BlankEntity.EntityId_0
    jsr     Entity.Spawn
    bcs     Fail

    // Test free list is 0
    jsr     _FreeListLength
    bne     Fail

    // Test parameter Sum is correct value
    lda.w   Entities.BlankEntity.parameterSum
    cmp.w   #5 * Entity.N_ENTITIES
    bne     Fail

    // Test first list length
    jsr     _FirstListLength
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
Test.add("Entity.Spawn (Multiple Lists)")
scope Spawn_MultipleLists: {
constant tmp(Test.tmp)

    jsr     Entity.Init

    stz.w   Entities.BlankEntity.parameterSum

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    assert(Entity.N_ENTITIES >= Entity.lists.N_LISTS * 2)

    // spawn 2 entities per list
    lda.w   #Entities.BlankEntity.EntityId_0
    Loop:
        sta.w   tmp

        ldy.w   #1
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #1
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #Entities.BlankEntity.EntityId_0 + Entity.lists.N_LISTS
        bcc     Loop


    // Test parameter Sum is correct value
    lda.w   Entities.BlankEntity.parameterSum
    cmp.w   #2 * Entity.lists.N_LISTS
    bne     Fail


    // test list decreased by N_LISTS
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 2
    bne     Fail

    // Test each list length
    ldy.w   #0
    ListLoop:
        phy
            jsr     _ListLength_Y
        ply

        cmp.w   #2
        bne     Fail

        iny
        cpy.w   #Entity.lists.N_LISTS
        bcc     ListLoop

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entity.Spawn (Offscreen)")
scope Spawn_Offscreen: {
constant tmp_listId(Test.tmp)

constant ONSCREEN(0x1080)

    jsr     Entity.Init

    // spawn 2 entities on-screen and 4 entities off-screen per entity list

    assert(Entity.N_ENTITIES >= Entity.lists.N_LISTS * 6)

    lda.w   #Entities.BlankEntity.EntityId_0
    Loop:
        sta.w   tmp_listId

        macro spawn(xPos, yPos) {
            ldx.w   #{xPos}
            ldy.w   #{yPos}
            jsr     SpawnEntity_XY
            bcc     Fail
        }

        spawn(ONSCREEN, ONSCREEN)
        spawn(ONSCREEN + 0x200, ONSCREEN)
        spawn(ONSCREEN, ONSCREEN + 0x200)
        spawn(ONSCREEN, ONSCREEN)
        spawn(ONSCREEN - 0x200, ONSCREEN)
        spawn(ONSCREEN, ONSCREEN - 0x200)

        lda.w   tmp_listId
        inc
        cmp.w   #Entities.BlankEntity.EntityId_0 + Entity.lists.N_LISTS
        bcc     Loop

    // Verify list sizes

    jsr     _FreeListLength
    cmp.w   #ExpectedSizes.FREE
    bne     Fail

    jsr     _DeactivatedListLength
    cmp.w   #ExpectedSizes.DEACTIVATED
    bne     Fail


    ldx.w   #ExpectedSizes
    jsr     _TestListLengthTable
    bcc     Fail


    jmp     _TestEntityListsValid

Fail:
    clc
    rts

scope SpawnEntity_XY: {
    stx.w   Entity.Spawn.xPos
    sty.w   Entity.Spawn.yPos
    lda.w   tmp_listId
    jmp     Entity.Spawn
}

rodata(rom0)
scope ExpectedSizes: {
// NOTE: projectiles are always spawned, even if offscreen
    dw  6   // player
    dw  6   // boss
    dw  2   // enemy
    dw  2   // npc
    dw  2   // platform
    dw  6   // particle

    constant ALIVE(6 * 3 + 2 * 3)
    constant DEACTIVATED(4 * 3)
    constant FREE(Entity.N_ENTITIES - ALIVE - DEACTIVATED)

    constant size(pc() - ExpectedSizes)
    assert(size / 2 == Entity.lists.N_LISTS)
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

