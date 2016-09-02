
scope MetaSprite {
scope ActionPoint {

// Spawns a given entity on a given ActionPoint type
//
// When entities are spawned the value `MetaSprite.ActionPoint.address`
// will be set to the address of the current Action Point.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// MUST NOT BE CALLED IN ENTITY CONSTRUCTOR
//
// INPUT: DP = entity
//        Y = entityId
//        A = type to match
a16()
i16()
code()
scope SpawnEntityOfType: {
allocateTmpWord(tmp_compare)
allocateTmpWord(tmp_entityToSpawn)

    sta.w   tmp_compare

    ldx.b   BaseEntity.MetaSprite.currentFrame
    beq     Return

    lda.l   Format.Frame.ActionPoints,x
    beq     Return

    tax

    sty.w   tmp_entityToSpawn

    sep     #$20

a8()
    lda.l   Format.ActionPoints.parameter,x
    beq     ReturnRep

    Loop:
        cmp.w   tmp_compare
        bne     SkipSpawn

            rep     #$31
        a16()
            lda.l   Format.ActionPoints.xPos,x
            and.w   #0x00ff
            // carry clear, will set carry on overflow (thus the -1)
            adc.w   #-INT_MS8_OFFSET - 1
            adc.b   BaseEntity.xPos.px
            sta.w   Entity.Spawn.xPos

            clc
            lda.l   Format.ActionPoints.yPos,x
            and.w   #0x00ff
            // carry clear, will set carry on overflow (thus the -1)
            adc.w   #-INT_MS8_OFFSET - 1
            adc.b   BaseEntity.yPos.px
            sta.w   Entity.Spawn.yPos

            stx.w   MetaSprite.ActionPoint.address
            tdc
            tay
            lda.w   tmp_entityToSpawn
            jsr     Entity.Spawn

            ldx.w   MetaSprite.ActionPoint.address

            sep     #$20
        a8()

    SkipSpawn:
        inx
        inx
        inx
        lda.l   Format.ActionPoints.parameter,x
        bne     Loop

ReturnRep:
    rep     #$30
a16()
Return:
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

