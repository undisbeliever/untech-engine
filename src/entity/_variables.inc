// entity/_variables.inc
// =====================
//
// Variables used by the Entity module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {
    assert(N_ENTITIES >= 10)

    assert(ENTITY_SIZE > 0)
    assertPowerOfTwo(ENTITY_SIZE)

    allocate(entityPool, entityBlock, ENTITY_SIZE * N_ENTITIES)

    // assert that some entities will exist at the start of a page.
    // Entities that start on a page are faster than those that don't.
    assert(entityPool & (ENTITY_SIZE - 1) == 0)
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

        // pointer to the ROM data used by the entity code
        // (word addr)
        field(romPtr, 2)

        // Entity List Id for the current entity
        //
        // Set to `BaseEntityData.initialListId` before entity's
        // `BaseEntityFunctionTable.Init` call.  The entity is ONLY ALLOWED to
        // modify the `listId` variable in the entity's `Init` method.
        //
        // This variable MUST NOT BE MODIFIED after the `Init` method returns.
        // To change the `listId` after construction use the
        // `ChangeEntityListIdAndGotoNextEntity` routine.
        //
        // (byte)
        field(listId, 1)

        MetaSprite.EntityData()

        // position of entities relative to map
        // (2 x 0:16:16 fixed point)
        field(xPos.sx, 2)   // subpixel position
        field(xPos.px, 2)   // pixel position
        field(yPos.sx, 2)   // subpixel position
        field(yPos.px, 2)   // pixel position

        // velocity of entity
        // (2 x 1:15:16 fixed point - pixels/display frame)
        field(xVecl, 4)
        field(yVecl, 4)
    endstruct()
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
}

// vim: ft=bass-65816 ts=4 sw=4 et:

