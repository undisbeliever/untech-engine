// entity/_dataformat.inc
// ======================
//
// Entity ROM Data Format.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Data format may be subject to changes
if Entity.Data.ENTITY_FORMAT_VERSION != 6 {
    error "Invalid UnTech Entity Format"
}

namespace Entity {
namespace Data {
    // Number of entities in the EntityRomDataList
    constant EntityRomDataListCount = Project.EntityIds.count

    // Number of projectiles in the ProjectileRomDataList
    constant ProjectileRomDataListCount = Project.ProjectileIds.count

    // Number of players in the ProjectileRomDataList
    constant PlayerRomDataListCount = Project.PlayerIds.count


    // The offset between the playerId and the RomData tables
    constant PlayerRomDataListOffset = EntityRomDataListCount + ProjectileRomDataListCount


    // Entity function table address for each entity and projectile
    //
    // Contains entity, projectile and player FunctionTables.
    // Projectile data is stored directly after entity data.
    // Player data is stored directly after projectile data.
    //
    // (word addr array, points to `BaseEntityFunctionTable` in code bank)
    constant FunctionTables = Project.EntityFunctionTables
    assert(Project.EntityFunctionTables.size == (EntityRomDataListCount + ProjectileRomDataListCount + PlayerRomDataListCount) * 2)


    // EntityRomData table
    //
    // Data contains entity and projectile EntityRomData.
    //
    // Indexes for `EntityRomDataList` match the `FunctionTables` array.
    //
    // (word index array - points EntityRomData data)
    constant EntityRomDataList = Project.EntityRomDataList
}
}

// Read only data used by the entity.
//
// These structs are extendible (by untech-editor) and will be used to
// store entity parameters.
namespace BaseEntityRomStruct {
    basestruct_offset(Project.EntityRomData)
        // Initial palette of the entity
        // (byte)
        field(defaultPalette, 1)

        // Initial projetileId for the entity
        // (Should be set to 0xff if the entity has no initial projectile)
        // (byte)
        field(initialProjectileId, 1)

        // Initial list to store the entity in
        // (Ignored if the entity is spawned as a projectile)
        // (see `BaseEntity.listId`)
        // (byte)
        field(initialListId, 1)

        // MS FrameSet Id
        // (word)
        field(frameSetId, 2)
    endstruct()

    // Ensure untech-editor BaseEntityRomStruct matches untech-engine BaseEntityRomStruct
    assert(BaseEntityRomStruct.defaultPalette == Project.EntityRomStructs.BaseEntityRomStruct.defaultPalette)
    assert(BaseEntityRomStruct.initialProjectileId == Project.EntityRomStructs.BaseEntityRomStruct.initialProjectileId)
    assert(BaseEntityRomStruct.initialListId == Project.EntityRomStructs.BaseEntityRomStruct.initialListId)
    assert(BaseEntityRomStruct.frameSetId == Project.EntityRomStructs.BaseEntityRomStruct.frameSetId)
    assert(BaseEntityRomStruct.size == Project.EntityRomStructs.BaseEntityRomStruct.size)
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
        //    * Entity momentum set to 0
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

namespace BasePlayerFunctionTable {
    basestruct()
        // Initialise player variables.
        //
        // When this method is called the following is already set:
        //    * Player position
        //    * Player movementState
        //    * Player momentum reset to 0
        //    * MetaSprite frameSet and palette initialized
        //
        // After this method is called:
        //    * The MetaSprite is activated
        //    * Camera is updated
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp = platerEntity
        field(Init, 2)

        // Room About to Change hook.
        //
        // This method is called just before a new room is loaded.
        //
        // REGS: 16 bit A, 16 bit Index, DB = $7e
        // INPUT: dp = playerEntity
        field(RoomAboutToChange, 2)
        // ::TODO call RoomAboutToChange::

        // Process the player entity.
        //
        // This method is responsible for setting the player's MetaSprite frame.
        // There should be a call to either `MetaSprite.Animation.Process` or
        // `MetaSprite.SetFrame` within this method.
        //
        // NOTE: This method MUST NOT draw any MetaSprites,
        //       it is called outside the MetaSprites render loop.
        //
        // INPUT: dp = playerEntity
        field(Process, 2)

        // Draw the player entity.
        //
        // This method will be called after the camera has been updated and
        // inside the Metasprite RenderLoop.
        //
        // REGS: 16 bit A, 16 bit Index, DB = 0x7e
        //
        // INPUIT: dp = playerEntity
        field(DrawPlayer, 2)

        // Process a collision between this entity and an other entity.
        //
        // This routine is the same as `BaseEntityFunctionTable.ProcessCollision`
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

        assert(Init == BaseEntityFunctionTable.Init)
        assert(ProcessCollision == BaseEntityFunctionTable.ProcessCollision)
    endstruct()
}

// vim: ft=bass-65816 ts=4 sw=4 et:

