
scope EntityTest {
scope Entities {

scope ChangeToParticleEntity {

    buildFunctionTable(BaseEntityFunctionTable, ChangeToParticleEntity)
    Entity.useDefaultRenderer()

// IN: A = parameter
a16()
i16()
code()
scope Init: {
    rts
}


// IN: A = parameter
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
    // change to particle list
    lda.w   #Entity.lists.particles
    sta.w   Entity.changeList

    rts
}


// Empty hitbox collision hooks
a16()
i16()
code()
CalculateAttackValue:
TakeDamageFromEntity:
BlockWithShield:
CollideWithBody:
CollideWithShield:
    rts



evaluate n(0)
while {n} < Entity.lists.N_LISTS {
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

    evaluate n({n} + 1)
}

code()
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

