
scope Entities {

// Processes the render loop
//
// Renders all entities to screen
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
// MODIFIES: DP
a16()
i16()
code()
scope RenderLoop: {
    // using `lists.N_LISTS` directly causes unrecognized variable
    // errors in bass, comparing value manually
    assert(lists.N_LISTS == 8)

    macro _repeat(evaluate n) {
        if {n} < 8 {
            scope _{n} {
                lda.w   lists.FIRST + {n} * 2
                jsr RenderList
            }
            _repeat({n} + 1)
        }
    }
    _repeat(0)

    rts
}

// Render a list of entities
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A - address of first entity in list
// INPUT: z - set if list is empty
// MODIFIES: DP
a16()
i16()
code()
scope RenderList: {
    beq     Skip

    Loop:
        tcd
        ldx.b   BaseEntity.functionTable
        beq     Continue

            lda.b   BaseEntity.xPos.px
            sec
            sbc.w   #INT_MS8_OFFSET
            sec
            sbc.w   Camera.xPos
            sta.w   MetaSprite.Render.xPos

            lda.b   BaseEntity.yPos.px
            sec
            sbc.w   #INT_MS8_OFFSET
            sec
            sbc.w   Camera.yPos
            sta.w   MetaSprite.Render.yPos

            jsr     (BaseEntityFunctionTable.Render,x)

    Continue:
        lda.b   BaseEntity.next
        bne     Loop
Skip:
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

