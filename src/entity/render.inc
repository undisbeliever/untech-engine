// entity/render.inc
// =================
//
// Entity Render-Loop.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {

// Processes the render loop
//
// Renders all entities to screen
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
// MODIFIES: DP
a16()
i16()
code()
function RenderLoop {
    macro _jsr_RenderList(name) {
        lda.w   lists.{name}
        jsr     RenderList
    }

    _jsr_RenderList(particles)
    _jsr_RenderList(playerProjectiles)
    _jsr_RenderList(enemyProjectiles)
    _jsr_RenderList(players)
    _jsr_RenderList(bosses)
    _jsr_RenderList(enemies)
    _jsr_RenderList(npcs)

    lda.w   lists.platforms
End:
    // RenderList
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
function RenderList {
assert(pc() == RenderLoop.End)

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

// vim: ft=bass-65816 ts=4 sw=4 et:

