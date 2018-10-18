// entity/_dataformat.inc
// ======================
//
// Entity ROM Data Format.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {

namespace Data {
    // List of entities.
    // Addresses point to the BaseEntityData struct in the EN_RomData.
    // (list of addr)
    storeBlockStart(EntityList,  EN_EntityList)
    storeBlockCount(EntityListCount, EN_EntityList, 2)

    // List of projectiles.
    // Addresses point to the BaseEntityData struct in the EN_RomData.
    // (list of addr)
    storeBlockStart(ProjectileList,  EN_ProjectileList)
    storeBlockCount(ProjectileListCount, EN_ProjectileList, 2)

    rodata(EN_RomData)
        constant Offset = pc() & 0xff0000

    code()
}

// Uses the default renderer in the function table
inline useDefaultRenderer() {
    constant Render = MetaSprite.Render.RenderEntity
}

}

// Read only data used by the entity.
//
// These structs are extendible and will be used to store entity
// parameters.
namespace BaseEntityData {
    basestruct_offset(Entity.Data.Offset)
        // Location of the entity's `functionPtr` table
        // (word addr - code bank)
        field(functionTable, 2)

        // Initial list to store the entity in
        // (see Entity.ListId enum)
        // (byte)
        field(initialListId, 1)

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
namespace BaseEntityFunctionTable {
    basestruct()
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

        // Process one frame of the entity loop
        //
        // This routine MUST NOT invoke RTS.
        // Instead this routine MUST JUMP to the `Entity.GotoNextEntity`
        // routine.
        //
        // The untech-engine provides a number of routines that change the
        // entity's state and eventually jump to `GotoNextEntity`. These
        // routines have been tagged with `@EndEntityProcess`
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        // RETURN: Entity.GotoNextEntity or a `@EndEntityProcess` tagged routine.
        field(Process, 2)


        // Render entity to screen
        //
        // Should point to `MetaSprite.Render.RenderEntity` by default
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        field(Render, 2)


        // Process a collision between this entity and an other entity.
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        //
        // INPUT: dp: entity
        //         A: collision bits
        //         Y: otherEntity address
        //         Metasprite.Collision.bits: collision bits (MUST NOT BE MODIFIED)
        //         Metasprite.Collision.otherEntity: the entity collided with (MUST NOT BE MODIFIED)
        //         Metasprite.Collision.xPos: the x-coordinate of the collision centre
        //         Metasprite.Collision.yPos: the y-coordinate of the collision centre
        // Collision bits format: (this hitboxType << 4) | other hitboxType
        field(ProcessCollision, 2)
    endstruct()
}

// vim: ft=bass-65816 ts=4 sw=4 et:

