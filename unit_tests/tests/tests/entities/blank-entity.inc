
scope EntitiesTest {

scope BlankEntity {
    buildFunctionTable(BaseEntityFunctionTable, BlankEntity)
    Entities.useDefaultRenderer()

allocateTmpWord(nInits)

a16()
i16()
code()
scope Init: {
    inc.w   nInits
    rts
}

a16()
i16()
code()
scope Process: {
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

