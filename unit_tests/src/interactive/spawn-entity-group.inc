// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//   * Spawn_Entity_Group bytecode instruction
//   * Sleep_AnimationTicks bytecode instruction


namespace InteractiveTests {
namespace SpawnEntityGroup {

constant GRAVITY = 10000

constant FRAMES_PER_SECOND = 60

constant ENEMIES_SPAWNED_PER_SECOND = 4

constant N_ENEMIES_SPAWNED = 5 * ENEMIES_SPAWNED_PER_SECOND


a16()
i16()
code()
Test.add("Spawn_Entity_Group bytecode test")
function Test {

// Number of frames to wait between entity count tests
// (uint16)
constant _enemyCountTestDelay  = Test.dpTmp + 0

// Expected number of enemies for the next enemy count test
// (uint16)
constant _expectedEnemyCount   = Test.dpTmp + 2

assert(Test.TMP_SIZE > 2)


    lda.w   #Project.RoomList.SpawnEntityGroupTest
    ldx.w   #Project.PlayerIds.InteractiveTilesTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx

    // Force NTSC timing
    lda.w   #Resources.ANIMATION_TICKS_PER_SECOND / FRAMES_PER_SECOND
    sta.w   Resources.animationSpeed

    lda.w   #ENEMIES_SPAWNED_PER_SECOND
    sta.b   _expectedEnemyCount

    lda.w   #FRAMES_PER_SECOND / 2
    sta.b   _enemyCountTestDelay



    jsr     Test.PrintCurrentTestName


    jsr     Resources.TransferToPpu

    jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far

    sep     #$20
a8()

    jsr     EnableDisplay_Full

    rep     #$30
a16()


    Loop:
        jsr     WaitFrame_PrintFreeCycles

        jsr     GameLoop.ProcessFrame


        // Every second, check the number of enemies has increased
        dec.b   _enemyCountTestDelay
        bne     ++
            Entity.CountEntitiesInList(Enemies)
            cmp.b   _expectedEnemyCount
            bne     Fail

            lda.b   _expectedEnemyCount
            clc
            adc.w   #ENEMIES_SPAWNED_PER_SECOND
            cmp.w   #N_ENEMIES_SPAWNED
            bcc     +
                lda.w   #N_ENEMIES_SPAWNED
            +
            sta.b   _expectedEnemyCount

            lda.w   #FRAMES_PER_SECOND
            sta.b   _enemyCountTestDelay
        +

        // Loop until the script ends
        ldx.w   Scripting.activeScriptsEnd
        bne     Loop


    Entity.CountEntitiesInList(Enemies)
    cmp.w   #N_ENEMIES_SPAWNED
    bne     Fail


    // Test an invalid entity groups will set `_entityAddr` to 0.
    sep     #$20
a8()
    ldx.w   #0xffff
    stx.w   Room.EntityGroups.Spawn_Entity_Group__BgTask._entityAddr

    // Test spawning an empty entity group
    lda.b   #Room.N_ENTITY_GROUPS - 1
    ldy.w   #0x100
    sta.w   Scripting.scriptData,y
    jsr     Room.EntityGroups.Spawn_Entity_Group__BgTask.SetupBgTask___EntityGroup

    ldx.w   Room.EntityGroups.Spawn_Entity_Group__BgTask._entityAddr
    bne     Fail


    // Test out of bounds entity group id
    ldx.w   #0xffff
    stx.w   Room.EntityGroups.Spawn_Entity_Group__BgTask._entityAddr

    constant OOB_GROUP_ID = 28
    lda.b   #OOB_GROUP_ID
    ldy.w   #0x100
    sta.w   Scripting.scriptData,y

    // Ensure entityGroup value for this out of bounds read is non-zero
    sta.w   Room.Format.Header.entityGroups + OOB_GROUP_ID

    jsr     Room.EntityGroups.Spawn_Entity_Group__BgTask.SetupBgTask___EntityGroup

    ldx.w   Room.EntityGroups.Spawn_Entity_Group__BgTask._entityAddr
    bne     Fail


    rep     #$30
a16()

    // Test passes
    Resources.AnimatedTileset.DisableAnimation()
    Resources.Palette.DisableAnimation()

    jsl     Entity.ValidateEntityLists__far

    jmp     Test.ResetPpuState


Fail:
    clc
    rts
}


}
}

