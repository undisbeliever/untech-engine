
scope Entities {

scope Data {
    storeBlockStart(List,  EN_RomDataList)

    rodata(EN_RomData)
        constant Offset(pc() & 0xff0000)

    code()
}

// Read only data used by the entity.
//
// These structs are extendible and will be used to store entity
// parameters.
scope BaseEntityData {
    basestruct-offset(BaseEntityData, Data.Offset)
        // Location of the entity's `functionPtr` table
        // (word addr - code bank)
        field(functionTable, 2)

        // Initial list to store the entity in
        // (byte index)
        field(initialList, 1)

        // MS FrameSet Id
        // (word)
        field(frameSetId, 2)

        // Initial palette of the entity
        // (byte)
        field(defaultPalette, 1)
    endstruct()
}

// Base Entity Function Table
//
// The data for this must exist in the code data block
scope BaseEntityFunctionTable {
    basestruct(BaseEntityFunctionTable)
        // Initialise entity variables.
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        field(Init, 2)

        // Process 1 frame of the entity loop
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        field(Process, 2)

        // Render entity to screen
        //
        // Should point to `MetaSprite.Render.RenderEntity` by default
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        //        MetaSprite.xPos: entity xPos - INT_MS8_OFFSET
        //        MetaSprote.yPos: entity yPos - INT_MS8_OFFSET
        field(Render, 2)
    endstruct()
}

}

// vim: ft=asm ts=4 sw=4 et:
