// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace ActionPointSpawnerTest {

// Tests:
//  * EntityLoop
//  * Entity.SpawnPlayerProjectileFromActionPoint
//  * Entity.SpawnEnemyProjectileFromActionPoint
//  * Entity.SpawnAtActionPoint
//  * Entity.DeleteIfOutsideAndGotoNextEntity
//  * Entity.CountEntitiesInList
//  * ENTITY_POOL_EMPTY warning


namespace ProjectileEntity {
    buildFunctionTable(BaseEntityFunctionTable, ProjectileEntity)

    // Slow speed to exhaust entity pool.
    constant VELOCITY = 0x8000
    constant DIAGONAL_VELOCITY = 0x5a82 // sqrt(0x8000 ** 2 / 2

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(tmp, 4)
        endstruct()
    }

    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        sep     #$20
    a8()

        ldx.w   MetaSprite.ActionPoint.address

        // Determine if projectile is diagonal or orthogonal
        lda.l   MetaSprite.Format.ActionPoints.xPos,x
        cmp.b   #INT_MS8_OFFSET
        beq     Orthogonal

        lda.l   MetaSprite.Format.ActionPoints.yPos,x
        cmp.b   #INT_MS8_OFFSET
        beq     Orthogonal

            // Projectile is Diagonal
            ldy.w   #DIAGONAL_VELOCITY
            sty.b   EntityStruct.tmp
            ldy.w   #DIAGONAL_VELOCITY >> 16
            bra     EndIf

        Orthogonal:
            // Projectile is Orthogonal
            ldy.w   #VELOCITY
            sty.b   EntityStruct.tmp
            ldy.w   #VELOCITY >> 16

    EndIf:
        sty.b   EntityStruct.tmp + 2


        rep     #$30
    a16()

        macro _SetVelocity(c) {
            namespace {c} {
                // Set Projectile Velocity
                lda.l   MetaSprite.Format.ActionPoints.{c}Pos,x
                and.w   #0xff
                cmp.w   #INT_MS8_OFFSET
                beq     Zero
                bcs     Positive

                Negative:
                    // load negative velocity
                    sec
                    lda.w   #0
                    sbc.b   EntityStruct.tmp
                    sta.b   EntityStruct.{c}Vecl

                    lda.w   #0
                    sbc.b   EntityStruct.tmp + 2
                    sta.b   EntityStruct.{c}Vecl + 2

                    bra     EndIf

                Positive:
                    lda.b   EntityStruct.tmp
                    sta.b   EntityStruct.{c}Vecl
                    lda.b   EntityStruct.tmp + 2
                    sta.b   EntityStruct.{c}Vecl + 2

                    bra     EndIf

                Zero:
                    stz.b   EntityStruct.{c}Vecl
                    stz.b   EntityStruct.{c}Vecl + 2

            EndIf:
            }
        }

        _SetVelocity(x)
        _SetVelocity(y)


        lda.w   #0
        jsr     MetaSprite.SetFrame

        // always activate
        sec
        rts
    }

    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
    function Null {
        rts
    }

    a16()
    i16()
    code()
    Process: {
        macro _addVelocity(c) {
            clc
            lda.b   BaseEntity.{c}Vecl
            adc.b   BaseEntity.{c}Pos.sx
            sta.b   BaseEntity.{c}Pos.sx

            lda.b   BaseEntity.{c}Vecl + 1
            xba
            bpl     +
                ora.w   #$ff00
                bra     ++
            +
                and.w   #$00ff
            +
            adc.b   BaseEntity.{c}Pos.px
            sta.b   BaseEntity.{c}Pos.px
        }
        _addVelocity(x)
        _addVelocity(y)

        jmp     Entity.DeleteIfOutsideOrDrawAndGotoNextEntity
    }

    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  EntityLists.Particles   // initialListId
        db  0                       // defaultPalette
        dw  MSFS.Projectile         // frameSetId

    rodata(EN_ProjectileList)
    constant PROJECTILE_ID = (pc() - Entity.Data.ProjectileList) / 2
        dw  RomData

    rodata(EN_EntityDataList)
    constant ENTITY_DATA_ID = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData
}



namespace SpawnerEntity {
    buildFunctionTable(BaseEntityFunctionTable, SpawnerEntity)

    constant FRAME_DELAY = 5
    constant N_SPAWN_POINTS = 8

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(frameDelay, 2)
            field(spawnPoint, 2)
            field(spawnRoutine, 2)

            // used to test if an entity spawned or not
            field(_freeListValue, 2)
        endstruct()
    }

    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        stz.b   EntityStruct.frameDelay
        stz.b   EntityStruct.spawnPoint
        stz.b   EntityStruct.spawnRoutine

        lda.w   #0
        jsr     MetaSprite.SetFrame

        // always activate
        sec
        rts
    }

    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
    function Null {
        rts
    }

    // Spawn the projectile using alternating action points and
    // spawn routines.
    // If no entity was spawned then try again on the next frame.
    a16()
    i16()
    code()
    function Process {
        assertPowerOfTwo(N_SPAWN_POINTS)

        lda.b   EntityStruct.frameDelay
        dec
        bpl     SkipSpawn

            lda.w   Entity.specialLists.free
            sta.b   EntityStruct._freeListValue


            lda.b   EntityStruct.spawnPoint
            inc

            ldx.b   EntityStruct.spawnRoutine
            jsr     (SpawnRoutineTable,x)


            lda.w   Entity.specialLists.free
            cmp.b   EntityStruct._freeListValue
            beq     NoEntitySpawned

                lda.b   EntityStruct.spawnPoint
                inc
                and.w   #N_SPAWN_POINTS - 1
                sta.b   EntityStruct.spawnPoint
                bne     SkipChangeSpawner

                    ldx.b   EntityStruct.spawnRoutine
                    inx
                    inx
                    cpx.w   #N_SPAWN_ROUTINES * 2
                    bcc     +
                        ldx.w   #0
                    +
                    stx.b   EntityStruct.spawnRoutine

            SkipChangeSpawner:
                lda.w   #FRAME_DELAY - 1

        NoEntitySpawned:

    SkipSpawn:
        sta.b   EntityStruct.frameDelay

        jmp     Entity.DrawAndGotoNextEntity
    }


    // INPUT: A = spawn point
    //        DP = entity
    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    code()
    SpawnRoutineTable:
        dw  SpawnPlayerProjectileAtAp
        dw  SpawnEnemyProjectileAtAp
        dw  SpawnEntityAtAp
    constant N_SPAWN_ROUTINES = (pc() - SpawnRoutineTable) / 2

    a16()
    i16()
    code()
    function SpawnPlayerProjectileAtAp {
        ldy.w   #ProjectileEntity.PROJECTILE_ID
        jmp     Entity.SpawnPlayerProjectileFromActionPoint
    }

    a16()
    i16()
    code()
    function SpawnEnemyProjectileAtAp {
        ldy.w   #ProjectileEntity.PROJECTILE_ID
        jmp     Entity.SpawnEnemyProjectileFromActionPoint
    }

    a16()
    i16()
    code()
    function SpawnEntityAtAp {
        ldy.w   #ProjectileEntity.ENTITY_DATA_ID
        jmp     Entity.SpawnAtActionPoint
    }


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  EntityLists.Enemies     // initialListId
        db  0                       // defaultPalette
        dw  MSFS.ActionPointSpawner // frameSetId

    rodata(EN_EntityDataList)
    constant ENTITY_DATA_ID = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData
}



// Prints the list statistics to screen
macro PrintListSizes() {
    assert16a()
    assert16i()

    Text.Console.SetCursor(0, 20)
    Text.Console.PrintConstString("\nPlayer Projectiles: ")

    Entity.CountEntitiesInList(PlayerProjectiles)
    jsr     Text.Console.PrintU16A

    Text.Console.PrintConstString("\nEnemy Projectiles:  ")

    Entity.CountEntitiesInList(EnemyProjectiles)
    jsr     Text.Console.PrintU16A

    Text.Console.PrintConstString("\nParticle Entities:  ")

    Entity.CountEntitiesInList(Particles)
    jsr     Text.Console.PrintU16A
}


// The Test
a16()
i16()
code()
Test.add("Spawn At Action Point Test")
function Test {
    Text.Console.SetCursor(0, 4)
    Text.Console.PrintConstString("Also tests:\n  DeleteAndGotoNextEntity")

    jsr     EntityLoop.Init

    jsr     EnableDisplay_Full


    // Load the entity

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_HEIGHT / 2
    sta.w   Entity.Spawn.yPos

    lda.w   #SpawnerEntity.ENTITY_DATA_ID

    jsr     Entity.Spawn
    bcc     Fail


    // Process the Entity Loop
    // -----------------------

    GameLoop:
        jsr     EntityLoop.ProcessFrame

        PrintListSizes()

        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        beq     GameLoop


    jmp     EntityLoop.Cleanup

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

