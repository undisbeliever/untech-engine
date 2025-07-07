// entity/_variables.inc
// =====================
//
// Variables used by the Entity module.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


// NOTE: There are three types of ids used by the entity subsystem:
//
//   * `instanceId`: An 8 bit identifier used to uniquely identify entity
//                   instances.  This value is assigned at Spawn and should
//                   be unique for each stage, but reuse may occur.
//                   Instance Id's can be converted to entity address using
//                   the `GetEntityWithIdOrBranchIfEmpty*` macros.
//
//   * `entityDataId`: An array index (NOT a multiple of two) within
//                     `Entity.Data.EntityRomDataList` that points to a
//                     `BaseEntityRomStruct` block contained in the ROM.
//
//   * `listId`: The `EntityLists` value, which represents the entity list the
//               entity is in.


namespace Entity {
    assert(N_ENTITIES >= 10)

    assert(ENTITY_SIZE > 0)
    assertPowerOfTwo(ENTITY_SIZE)

    // Player entity data
    //
    // NOTE: The player entity is always active.
    //
    // NOTE: The player entity MUST HAVE its `next` field set to NULL (0)
    //       (allows code reuse in entity hitbox collision tests).
    allocate(playerEntity, entityBlock, ENTITY_SIZE)

    // Pool from which all other entities are spawned from
    allocate(entityPool, entityBlock, ENTITY_SIZE * N_ENTITIES)

    // Assert playerEntity is at the start of a page.
    // Entities that start on a page are faster than those that don't.
    assert(playerEntity & 0xff == 0)
}

namespace BaseEntity {
    basestruct()
        struct_maxsize(Entity.ENTITY_SIZE)

        // address of next entity in current list
        // (word addr)
        field(next, 2)

        // address of the function table of the entity
        // (word addr)
        field(functionTable, 2)

        // index to the ROM data used by the entity code
        // (word index)
        field(romPtr, 2)

        // InstanceIdTable identifier for the current entity.
        //
        // If zero then the entity is not in the `instanceIdTable`.
        //
        // NOTE: projectile entities will not be given an instanceId.
        //
        // (byte index)
        field(instanceId, 1)

        // The projectile to spawn when a projectile ActionPoint is encountered.
        //
        // If id >= ProjectileRomDataListCount then no projectile will be spawned
        //
        // NOTE: This variable is between `instanceId` and `listId` so
        //       `Entity.ValidateEntityLists` can easily detect if the developer
        //       accidentally wrote to this variable with a 16 bit value.
        // (byte, project editable)
        field(projectileId, 1)

        // Entity List Id for the current entity
        //
        // Set to `Entity.BaseEntityRomStruct.initialListId` before the entity's
        // `BaseEntityFunctionTable.Init` call.  The entity is ONLY ALLOWED to
        // modify the `listId` variable in the entity's `Init` method.
        //
        // This variable MUST NOT BE MODIFIED after the `Init` method returns.
        // To change the `listId` after construction use the
        // `ChangeEntityListIdAndGotoNextEntity` routine.
        //
        // This variable is dependant on the contents of the `EntityLists` enum,
        // which is project definable.
        //
        // (byte)
        field(listId, 1)

        MetaSprite.EntityData()

        // Position of entities relative to map
        // (2 x unsigned 0:16:16 fixed point, project editable)
        field(xPos.sx, 2)   // subpixel position
        field(xPos.px, 2)   // pixel position
        field(yPos.sx, 2)   // subpixel position
        field(yPos.px, 2)   // pixel position

        // Entity momentum.
        //
        // Despite the name, these variables have nothing to do with
        // mass or Newtonian mechanics.  These values represent the
        // unsigned change in position on each display frame, direction
        // of movement is determined by `movementState`.
        //
        // Nomenclature: The name of these values comes from a Super
        // Metroid Tool-Assisted-Speedrun resource guide
        // (http://tasvideos.org/GameResources/SNES/SuperMetroid.html).
        //
        // (2 x unsigned 0:8:16 fixed point - pixels/display frame, project editable)
        field(xMomentum.sx, 2)
        field(xMomentum.px, 1)
        field(yMomentum.sx, 2)
        field(yMomentum.px, 1)

        // Entity movement state
        // (uint8, Entity.MovementState bitfield, project editable)
        field(movementState, 1)
    endstruct()
}


// Bitfield for the entity movement state
namespace Entity.MovementState {
    // When set entity is facing right, when clear entity is facing left
    constant FACING_RIGHT_FLAG      = %00000001

    // When set entity is moving down, when clear entity is moving up
    constant MOVING_DOWN_FLAG       = %00000010

    // When set entity is in a Standing state, when clear entity is Falling
    constant STANDING_FLAG          = %00000100

    // When set entity experiences upwards gravity, when clear entity experiences downwards gravity
    constant UP_GRAVITY_FLAG        = %00001000

    // Set if the entity is unaffected by gravity
    constant NO_GRAVITY_FLAG        = %00010000

    // When set entity is moving right, when clear entity is moving left
    constant MOVING_RIGHT_FLAG      = %01000000

    // Set if there was a tile collision in the X-axis in the last `MoveEntityAndProcessTileCollisions` call
    constant X_COLLISION_FLAG       = %10000000
}


// The EntityLists enum contains the names of the entity lists
// used by the subproject.
if !{defined EntityLists.__ENUM__.current} {
    error "Expected an enum named EntityLists"
}

// The engine requires that the EntityLists enum be:
//   1. Closed with endEnum()
//   2. With a starting value of 0
//   3. With an increment value of 1
assert({defined EntityLists.__ENUM__.closed})
assert(EntityLists.__ENUM__.first == 0)
assert(EntityLists.__ENUM__.last + 1 == EntityLists.__ENUM__.count)

namespace Entity {
    constant N_ENTITY_LISTS = EntityLists.__ENUM__.count

    // Retrieves the list index for a given entity list
    define listIndex(l) = (EntityLists.{l} * 2)

    // Retrieves the address of a given entity list
    define listAddress(l) = (Entity.lists.FIRST + EntityLists.{l} * 2)

    namespace lists {
        // A linked list containing the entities
        // (word addr * N_ENTITY_LISTS)
        allocate(_entityLists, wram7e, N_ENTITY_LISTS * 2)

        constant FIRST = _entityLists
        constant LAST = FIRST + (N_ENTITY_LISTS - 1) * 2
    }

    // Special lists
    // =============
    // These lists MUST NOT BE accessed by the subproject

    namespace specialLists {
        // Deactivated entities
        //  - will not be processed
        //  - will be reactivated when the camera moves the entity inside
        //  active window
        allocate(deactivated, wram7e, 2)
        constant DEACTIVATED_INDEX = deactivated - lists.FIRST

        // List of entities that are to be activated at the start of the
        // next frame.
        //
        // This list contains either:
        //  A) Entities that could not be activated in the previous
        //  frame, and
        //  B) Newly spawned active entities.
        allocate(activateNextFrame, wram7e, 2)
        constant ACTIVATE_NEXT_FRAME_INDEX = activateNextFrame - lists.FIRST

        // Unallocated Entities
        allocate(free, wram7e, 2)
        constant FREE_INDEX = free - lists.FIRST


        constant FIRST_SPECIAL_LIST_INDEX = DEACTIVATED_INDEX
        constant LAST_SPECIAL_LIST_INDEX = FREE_INDEX
    }

    // The last entity in the free list.
    // If there are no free entities left then `lastFreeEntity` is 0.
    // (word addr)
    allocate(lastFreeEntity, wram7e, 2)

    // Active window size (beyond camera display)
    // inactive entities inside this window will become active.
    constant ACTIVE_WINDOW_EXTEND_HORIZONTAL = 96
    constant ACTIVE_WINDOW_EXTEND_VERTICAL = 96

    // Inactive window size (beyond camera display)
    // Active entities that leave the window will deactivate.
    constant INACTIVE_WINDOW_EXTEND_HORIZONTAL = 128
    constant INACTIVE_WINDOW_EXTEND_VERTICAL = 128


    // Activate window
    // (4x uint16)
    namespace activeWindow {
        allocate(top, wram7e, 2)
        allocate(bottom, wram7e, 2)
        allocate(left, wram7e, 2)
        allocate(right, wram7e, 2)
    }

    // Inactivate window
    // (4x uint16)
    namespace inactiveWindow {
        allocate(top, wram7e, 2)
        allocate(bottom, wram7e, 2)
        allocate(left, wram7e, 2)
        allocate(right, wram7e, 2)
    }


    // Previous window position mask that the active window was tested
    //
    // Is not reset on Init (doesn't have to be)
    //
    // (2x uint16)
    allocate(prevActiveWindowX, wram7e, 2)
    allocate(prevActiveWindowY, wram7e, 2)


    // The mask which the Camera pos is compared to prevActiveWindow.
    constant TEST_ACTIVE_WINDOW_MASK = ~3



    // Instance Id Table
    // =================

    constant INSTANCE_ID_INCREMENT = 2
    constant FIRST_INSTANCE_ID = 2

    // The largest valid instanceId
    // NOTE: bit 0 of this value must be clear
    constant MAX_INSTANCE_ID = 127 * INSTANCE_ID_INCREMENT

    // Mapping of `instanceId`s to addresses
    // If the instanceId does not exist it points to `_emptyEntity`
    // (addr table)
    allocate(instanceIdTable, wram7e, MAX_INSTANCE_ID + 2)

    // Placeholder for the unused instance ids.
    //
    // It exists in ROM space (address > 0x8000) for two reasons:
    //   1) Bad code that writes to an unused instance Id will not clobber
    //      zero-page.
    //   2) Testing if an instanceId is unused can be done with the `n`
    //      status flag.
    allocate(_unusedEntity, wram7e, ENTITY_SIZE)
    assert(_unusedEntity & 0xffff > 0x8000)

    // The cursor position in the instanceIdTable.
    //
    // This value MUST BE:
    //  * Less than or equal to MAX_INSTANCE_ID
    //  * Have bit 0 clear
    // (byte index)
    allocate(currentInstanceId, wram7e, 1)

    // The minimum allowed instanceId when Spawning a new entity using `Entity.Spawn`.
    //
    // This allows the room subsystem to reserve `instanceId`s for the room entities.
    //
    // This value must be:
    //  * Non-zero
    //  * Less than MAX_INSTANCE_ID
    //  * Have bit 0 clear
    //
    // (byte index)
    allocate(minimumInstanceId, wram7e, 1)


    // Player
    // ======
    namespace Player {
        // The position of the player entity
        // (2 x unsigned 0:16:16 fixed point)
        constant xPos.sx = playerEntity + BaseEntity.xPos.sx
        constant xPos.px = playerEntity + BaseEntity.xPos.px
        constant yPos.sx = playerEntity + BaseEntity.yPos.sx
        constant yPos.px = playerEntity + BaseEntity.yPos.px

        // The player's movement state
        // (uint8, Entity.MovementState bitfield)
        constant movementState = playerEntity + BaseEntity.movementState
    }
}

if {defined SHOW_MEMORY_USAGE} {
    namespace SHOW_MEMORY_USAGE {
        evaluate baseEntitySize = BaseEntity.size
        evaluate baseEntityMaxSize = BaseEntity.__STRUCT__.maxSize
        evaluate baseEntityRemaining = BaseEntity.__STRUCT__.maxSize - BaseEntity.size

        print "\nBaseEntity size: {baseEntityMaxSize} bytes ({baseEntitySize} bytes used, {baseEntityRemaining} remaining)\n"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

