
scope EntityTest {
scope Entities {

scope DeleteAfterDelayEntity {

    buildFunctionTable(BaseEntityFunctionTable, DeleteAfterDelayEntity)
    Entity.useDefaultRenderer()

    scope EntityStruct {
        childstruct(BaseEntity)
            field(frames, 2)
        endstruct()
    }

// IN: A = parameter
a16()
i16()
code()
scope Init: {
    sta.b   EntityStruct.frames
    rts
}


a16()
i16()
code()
scope Destructor: {
    rts
}


a16()
i16()
code()
scope Process: {
    dec.b   EntityStruct.frames
    bne     +
        lda.w   #Entity.lists.toDelete
        sta.w   Entity.changeList
    +

    rts
}


a16()
i16()
code()
CalculateAttackValue:
TakeDamageFromEntity:
BlockWithShield:
CollideWithBody:
CollideWithShield:
    rts



macro _repeat(evaluate n) {
    if {n} < Entity.lists.N_LISTS {
        rodata(EN_RomData)
        scope RomData{n}: {
            dw  FunctionTable
            db  {n} * 2         // initial List
            dw  0               // frameSetId
            db  0               // defaultPalette
        }

        rodata(EN_EntityList)
        constant EntityId_{n}((pc() - Entity.Data.EntityList) / 2)
            dw  RomData{n}

        _repeat({n} + 1)
    }
}
_repeat(0)

code()
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

