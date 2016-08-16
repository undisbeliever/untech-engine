
scope EntitiesTest {

scope ChangeToParticleEntity {

    buildFunctionTable(BaseEntityFunctionTable, ChangeToParticleEntity)
    Entities.useDefaultRenderer()

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
    lda.w   #Entities.lists.particles
    sta.w   Entities.changeList

    rts
}

macro _repeat(evaluate n) {
    if {n} < Entities.lists.N_LISTS {
        rodata(EN_RomData)
        scope RomData{n}: {
            dw  FunctionTable
            db  {n} * 2         // initial List
            dw  0               // frameSetId
            db  0               // defaultPalette
        }

        rodata(EN_RomDataList)
        constant EntityId_{n}((pc() - Entities.Data.List) / 2)
            dw  RomData{n}

        _repeat({n} + 1)
    }
}
_repeat(0)

code()
}
}

// vim: ft=asm ts=4 sw=4 et:
