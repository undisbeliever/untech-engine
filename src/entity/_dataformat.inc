// entity/_dataformat.inc
// ======================
//
// Entity ROM Data Format.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Data format may be subject to changes
if Entity.Data.ENTITY_FORMAT_VERSION != 2 {
    error "Invalid UnTech Entity Format"
}

namespace Entity {
namespace Data {
    // List of entities
    // Word indexes within the EntityRomData data
    constant EntityRomDataList = Project.EntityRomDataList

    // Number of entities in the EntityRomDataList
    constant EntityRomDataListCount = Project.EntityIds.count

    // List of projectiles
    // Word indexes within the EntityRomData data
    constant ProjectileRomDataList = Project.ProjectileRomDataList

    // Number of entities in the ProjectileRomDataList
    constant ProjectileRomDataListCount = Project.ProjectileIds.count
}
}

// Read only data used by the entity.
//
// These structs are extendible (by untech-editor) and will be used to
// store entity parameters.
namespace BaseEntityRomStruct {
    basestruct_offset(Project.EntityRomData)
        // Location of the entity's `functionPtr` table
        // (word addr - code bank)
        field(functionTable, 2)

        // Initial list to store the entity in
        // (see `BaseEntity.listId`)
        // (byte)
        field(initialListId, 1)

        // Initial palette of the entity
        // (byte)
        field(defaultPalette, 1)

        // MS FrameSet Id
        // (word)
        field(frameSetId, 2)
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
        //    * Entity will be moved into the `activateNextFrame` entity
        //      list if carry set or the entity is inside the active window.
        //
        // NOTE: If the entity is spawned by `SpawnEnemyProjectileFromActionPoint`
        //       or `SpawnPlayerProjectileFromActionPoint` then it will always
        //       be activated, regardless of the Init routine output.
        //
        // NOTE: If the entity was spawned from an action point, then
        //       the variable `MetaSprite.ActionPoint.positionPair` will
        //       contain the action point position as a int_ms8 pair
        //       (which is useful for choosing the entity's direction).
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        //         A: parameter
        //         n: set if parameter is 0
        // OUTPUT: carry: set: the entity will be activated on the next frame
        //                clear: the entity will be activated if it is inside the active window.
        field(Init, 2)

        // Entity destructor routine
        //
        // This function is called after cleanup and the MetaSprite has been
        // de-allocated.
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        field(Destructor, 2)

        // Entity deactivated callback routine
        //
        // This function is called after the entity's MetaSprites have
        // been deactivated.
        //
        // The deactivated routine should reset the entity's state so
        // increase consistency when the player backtracks and
        // reactivates the entity.
        //
        // There is no "Activated" entity function.  If the developer
        // wishes to know when the entity is reactivated then they can
        // add a deactivated entity state/flag to the process routine.
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        field(Deactivated, 2)

        // Process one frame of the entity loop.
        //
        // This routine MUST NOT invoke RTS.
        // Instead this routine MUST JUMP to the `Entity.GotoNextEntity`
        // routine.
        //
        // The untech-engine provides a number of routines that change the
        // entity's state and eventually jump to `GotoNextEntity`. These
        // routines have been tagged with `@EndEntityProcess`
        //
        // This routine is responsible for rendering the entity, it is
        // recommended you use one of the `*DrawAndGotoNextEntity`
        // @EndEntityProcess routines to do so.
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp: entity
        // RETURN: Entity.GotoNextEntity or a `@EndEntityProcess` tagged routine.
        field(Process, 2)


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

