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
        // (2 x 0:16:16 fixed point - pixels)
        field(xPos, 2)
        field(xPos.px, 2)
        field(yPos, 2)
        field(yPos.px, 2)

        // velocity of entity
        // (2 x 1:15:16 fixed point - pixels/display frame)
        field(xVecl, 4)
        field(yVecl, 4)
    endstruct()
}

namespace Entity {
    // A linked list of entity types
    // (word addr)
    namespace lists {
        // Player entity and allies
        //  - collides with: enemies, bosses, enemyProjectiles
        allocate(players, wram7e, 2)

        // Bosses
        //  - collides with: playerProjectiles
        allocate(bosses, wram7e, 2)

        // Enemies
        //  - collides with: playerProjectiles
        allocate(enemies, wram7e, 2)

        // NPCs
        //  - no entity collisions
        allocate(npcs, wram7e, 2)

        // Platforms
        //  - no collision detection in entity gameloop.
        //     * The process routine of the player entities is is
        //       responsible for collision detection between entity and
        //       platforms as some entities won't have collision with
        //       platforms
        //  - deactivated when moved outside active window
        allocate(platforms, wram7e, 2)

        // Particles
        //  - no entity collisions
        allocate(particles, wram7e, 2)

        // Player projectiles
        //  - collides with: enemies, bosses
        allocate(playerProjectiles, wram7e, 2)

        // Enemy projectiles
        //  - collides with: players
        allocate(enemyProjectiles, wram7e, 2)


    constant FIRST = players
    constant LAST = enemyProjectiles
    constant N_LISTS = (LAST - FIRST) / 2 + 1

        // Special lists
        // =============
        // These lists MUST NOT BE accessed by the subproject

        // Deactivated entities
        //  - will not be processed
        //  - will be reactivated when the camera moves the entity inside
        //  active window
        allocate(deactivated, wram7e, 2)
        constant _DEACTIVATED_INDEX = deactivated - FIRST

        // List of entities that are to be activated at the start of the
        // next frame.
        //
        // This list contains either:
        //  A) Entities that could not be activated in the previous
        //  frame, and
        //  B) Newly spawned active entities.
        allocate(activateNextFrame, wram7e, 2)
        constant _ACTIVATE_NEXT_FRAME_INDEX = activateNextFrame - FIRST

        // Unallocated Entities
        allocate(free, wram7e, 2)
        constant _FREE_INDEX = free - FIRST


        constant _FIRST_SPECIAL_LIST_INDEX = _DEACTIVATED_INDEX
        constant _LAST_SPECIAL_LIST_INDEX = _FREE_INDEX
    }

    // The last entity in the free list.
    // If there are no free entities left then `lastFreeEntity` is 0.
    // (word addr)
    allocate(lastFreeEntity, wram7e, 2)

    // Entity list Id used for the BaseEntityData struct
    namespace ListId {
        createEnum()
            enum(PLAYER)
            enum(BOSS)
            enum(ENEMY)
            enum(NPC)
            enum(PLATFORM)
            enum(PARTICLE)
            enum(PLAYER_PROJECTILE)
            enum(ENEMY_PROJECTILE)

        assert({__ENUM__.current} == lists.N_LISTS)
    }


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

