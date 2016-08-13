
scope EntitiesTest {

scope BlankEntity {

    buildFunctionTable(BaseEntityFunctionTable, BlankEntity)
    Entities.useDefaultRenderer()

allocateTmpWord(parameterSum)

// IN: A = parameter
a16()
i16()
code()
scope Init: {
    clc
    adc.w   parameterSum
    sta.w   parameterSum

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
    rts
}


rodata(EN_RomData)
scope RomData: {
    dw  FunctionTable
    db  0               // initial List
    db  0               // frameSetId
    db  0               // defaultPalette
}

rodata(EN_RomDataList)
    dw  RomData

constant EntityId((pc() - Entities.Data.List) / 2 - 1)

code()
}
}

// vim: ft=asm ts=4 sw=4 et:

