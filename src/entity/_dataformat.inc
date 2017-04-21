
scope Entity {

scope Data {
    // List of entities.
    // Addresses point to the BaseEntityData struct in the EN_RomData.
    // (list of addr)
    storeBlockStart(EntityList,  EN_EntityList)

    // List of projectiles.
    // Addresses point to the BaseEntityData struct in the EN_RomData.
    // (list of addr)
    storeBlockStart(ProjectileList,  EN_ProjectileList)

    rodata(EN_RomData)
        constant Offset(pc() & 0xff0000)

    code()
}

// Uses the default renderer in the function table
macro useDefaultRenderer() {
    constant Render(MetaSprite.Render.RenderEntity)
}

}

// Read only data used by the entity.
//
// These structs are extendible and will be used to store entity
// parameters.
scope BaseEntityData {
    basestruct_offset(BaseEntityData, Entity.Data.Offset)
        // Location of the entity's `functionPtr` table
        // (word addr - code bank)
        field(functionTable, 2)

        // Initial list to store the entity in
        // (see Entity.ListId enum)
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
        // When this function is called the following is already set:
        //    * Entity position
        //    * Entity velocity set to 0
        //    * MetaSprite frameSet and palette initialized
        //
        // After this function is called
        //    * Entity will be activated if inside the active window
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        //         A: parameter
        //         n: set if parameter is 0
        field(Init, 2)

        // Entity destructor routine
        //
        // This function is called after cleanup and the MetaSprite has been
        // de-allocated.
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        field(Destructor, 2)

        // Process 1 frame of the entity loop
        //
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        //        `Entity.currentList` (const) current list being processed
        //
        // OPTIONAL OUTPUT:
        //   * set `Entity.changeList` to address of entity loop
        //     to change the entity list.
        //
        // After this function is called:
        //   * MetaSprite.Animation.Process is called
        //   * if `Entity.changeList` is set, the list this entity belongs
        //     to will change
        field(Process, 2)


        // Render entity to screen
        //
        // Should point to `MetaSprite.Render.RenderEntity` by default
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        //        MetaSprite.Render.xPos: entity xPos - INT_MS8_OFFSET
        //        MetaSprite.Render.yPos: entity yPos - INT_MS8_OFFSET
        field(Render, 2)


        // Return the entity's attack value
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp: entity
        //         Y: entity attacking
        //         C: set if the attacker is a ShieldAttack hitbox
        // OUTPUT: A: attack value
        field(CalculateAttackValue, 2)

        // Called when the entity's body hitbox collides with an attack
        // hitbox.
        //
        // This function is expected to reduce enemy HP after factoring
        // in defence / I-frames.
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp: entity
        //         A: attack value (opponent's CalculateAttackValue)
        //         Y: entity collided with
        //         C: set if entity hit weak point
        field(TakeDamageFromEntity, 2)

        // Called when the entity's shield hitbox collides with a
        // body/attack hitbox.
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp: entity
        //         Y: entity collided with
        //         C: set if collide with an attack
        //
        // NOTE: C will not be set if a shield collides with a SHEILD_ATTACK hitbox
        field(BlockWithShield, 2)

        // Called when the entity's body hitbox collides with a body
        // hitbox.
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp: entity
        //         Y: entity collided with
        field(CollideWithBody, 2)

        // Called when the entity's body hitbox collides with a shield
        // hitbox.
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp: entity
        //         Y: entity collided with
        field(CollideWithShield, 2)
    endstruct()
}

// vim: ft=bass-65816 ts=4 sw=4 et:

