
scope InteractiveTests {
scope ActionPointSpawnerTest {

// Tests:
//  * EntityLoop
//  * Entity.SpawnPlayerProjectileFromActionPoint
//  * Entity.SpawnEnemyProjectileFromActionPoint
//  * Entity.SpawnAtActionPoint
//  * Entity.DeleteOffscreenEntities
//  * Entity.CountEntitiesInList


scope ProjectileEntity {
    buildFunctionTable(BaseEntityFunctionTable, ProjectileEntity)
    Entity.useDefaultRenderer()

    constant VELOCITY(0x018000)
    constant DIAGONAL_VELOCITY(0x010f87)  // sqrt(0x018000 ** 2 / 2)

    scope EntityStruct {
        childstruct(BaseEntity)
            field(tmp, 3)
        endstruct()
    }

    a16()
    i16()
    code()
    scope Init: {
        sep     #$20
    a8()

        ldx.w   MetaSprite.ActionPoint.address

        // Determine if projectile is diagonal or orthogonal
        scope TestIfDiagonal {
            lda.l   MetaSprite.Format.ActionPoints.xPos,x
            cmp.b   #INT_MS8_OFFSET
            beq     Orthogonal

            lda.l   MetaSprite.Format.ActionPoints.yPos,x
            cmp.b   #INT_MS8_OFFSET
            beq     Orthogonal

                // Projectile is Diagonal
                ldy.w   #DIAGONAL_VELOCITY
                lda.b   #DIAGONAL_VELOCITY >> 16
                bra     EndIf

            Orthogonal:
                // Projectile is Orthogonal
                ldy.w   #VELOCITY
                lda.b   #VELOCITY >> 16

        EndIf:
            sty.b   EntityStruct.tmp
            sta.b   EntityStruct.tmp + 2
        }

        macro _SetVelocity(c) {
            scope SetVelocity_{c} {
                // Set Projectile Velocity
                lda.l   MetaSprite.Format.ActionPoints.{c}Pos,x
                cmp.b   #INT_MS8_OFFSET
                beq     Zero
                bcs     Positive

                Negative:
                    // load negative velocity
                    rep     #$31
                a16()
                    lda.w   #1
                    // c clear
                    sbc.b   EntityStruct.tmp
                    tay

                    sep     #$20
                a8()
                    lda.b   #0
                    sbc.b   EntityStruct.tmp + 2
                    bra     EndIf

                Positive:
                    ldy.b   EntityStruct.tmp
                    lda.b   EntityStruct.tmp + 2
                    bra     EndIf

                Zero:
                    ldy.w   #0
                    lda.b   #0

            EndIf:
                sty.b   EntityStruct.{c}Vecl
                sta.b   EntityStruct.{c}Vecl + 2
            }
        }

        _SetVelocity(x)
        _SetVelocity(y)


        rep     #$30
    a16()
        lda.w   #0
        jmp     MetaSprite.SetFrame
    }

    a16()
    i16()
    code()
    Destructor:
    TakeDamageFromEntity:
    CollideWithBody:
    CollideWithShield:
    BlockWithShield:
    CalculateAttackValue:
    scope Null: {
        rts
    }

    a16()
    i16()
    code()
    Process: {
        macro _addVelocity(c) {
            clc
            lda.b   BaseEntity.{c}Vecl
            adc.b   BaseEntity.{c}Pos
            sta.b   BaseEntity.{c}Pos

            lda.b   BaseEntity.{c}Vecl + 1
            xba
            bpl     +
                ora     #$ff00
                bra     ++
            +
                and     #$00ff
            +
            adc.b   BaseEntity.{c}Pos + 2
            sta.b   BaseEntity.{c}Pos + 2
        }
        _addVelocity(x)
        _addVelocity(y)

        rts
    }

    rodata(EN_RomData)
    scope RomData: {
        dw  FunctionTable
        db  Entity.ListId.NPC   // initial List
        dw  MSFS.Projectile     // frameSetId
        db  0                   // defaultPalette
    }

    rodata(EN_RomDataList)
    constant ENTITY_ID((pc() - Entity.Data.List) / 2)
        dw  RomData
}



scope SpawnerEntity {
    buildFunctionTable(BaseEntityFunctionTable, SpawnerEntity)
    Entity.useDefaultRenderer()

    constant FRAME_DELAY(5)
    constant N_SPAWN_POINTS(8)

    scope EntityStruct {
        childstruct(BaseEntity)
            field(frameDelay, 2)
            field(spawnPoint, 2)
            field(spawnRoutine, 2)
        endstruct()
    }

    code()
    scope SpawnRoutineTable: {
        dw  Entity.SpawnPlayerProjectileFromActionPoint
        dw  Entity.SpawnEnemyProjectileFromActionPoint
        dw  Entity.SpawnAtActionPoint
    }
    constant N_SPAWN_ROUTINES((pc() - SpawnRoutineTable) / 2)

    a16()
    i16()
    code()
    scope Init: {
        stz.b   EntityStruct.frameDelay
        stz.b   EntityStruct.spawnPoint
        stz.b   EntityStruct.spawnRoutine

        lda.w   #0
        jmp     MetaSprite.SetFrame
    }

    a16()
    i16()
    code()
    Destructor:
    TakeDamageFromEntity:
    CollideWithBody:
    CollideWithShield:
    BlockWithShield:
    CalculateAttackValue:
    scope Null: {
        rts
    }

    // Spawn the projectile using alternating action points and
    // spawn routines.
    a16()
    i16()
    code()
    scope Process: {
        lda.b   EntityStruct.frameDelay
        dec
        bpl     SkipSpawn

            assertPowerOfTwo(N_SPAWN_POINTS)

            lda.b   EntityStruct.spawnPoint
            inc

            ldy.w   #ProjectileEntity.ENTITY_ID

            ldx.b   EntityStruct.spawnRoutine
            jsr     (SpawnRoutineTable,x)


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

    SkipSpawn:
        sta.b   EntityStruct.frameDelay

        rts
    }

    rodata(EN_RomData)
    scope RomData: {
        dw  FunctionTable
        db  Entity.ListId.BOSS      // initial List
        dw  MSFS.ActionPointSpawner // frameSetId
        db  0                       // defaultPalette
    }

    rodata(EN_RomDataList)
    constant ENTITY_ID((pc() - Entity.Data.List) / 2)
        dw  RomData
}



// Prints the list statistics to screen
macro PrintListSizes() {
    assert16a()
    assert16i()

    Text.Console.SetCursor(0, 20)
    jsr     Text.Console.NewLine

    ldx.w   #PlayerProjectilesString
    lda.w   #PlayerProjectilesString >> 16
    jsr     Text.Console.PrintString

    Entity.CountEntitiesInList(playerProjectiles)
    jsr     Text.Console.PrintU16A

    jsr     Text.Console.NewLine

    ldx.w   #EnemyProjectilesString
    lda.w   #EnemyProjectilesString >> 16
    jsr     Text.Console.PrintString

    Entity.CountEntitiesInList(enemyProjectiles)
    jsr     Text.Console.PrintU16A

    jsr     Text.Console.NewLine

    ldx.w   #NpcsString
    lda.w   #NpcsString >> 16
    jsr     Text.Console.PrintString

    Entity.CountEntitiesInList(npcs)
    jsr     Text.Console.PrintU16A
}


// The Test
a16()
i16()
code()
Test.add("Spawn At Action Point Tests")
scope Test: {
    jsr     EntityLoop.Init

    // Load the entity

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_HEIGHT / 2
    sta.w   Entity.Spawn.yPos

    lda.w   #SpawnerEntity.ENTITY_ID

    jsr     Entity.Spawn
    bcc     Fail


    // Process the Entity Loop
    // -----------------------

    GameLoop:
        jsr     EntityLoop.ProcessFrame
        jsr     Entity.DeleteDeactivatedEntities

        PrintListSizes()

        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        beq     GameLoop


    jmp     EntityLoop.Cleanup

Fail:
    clc
    rts
}


rodata(rom0)
PlayerProjectilesString:
    db "Player Projectiles: ", 0

EnemyProjectilesString:
    db "Enemy Projectiles:  ", 0

NpcsString:
    db "NPC Entities:       ", 0

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

