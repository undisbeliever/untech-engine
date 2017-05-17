
scope Entity {
    assert(N_ENTITIES >= 10)

    assert(ENTITY_SIZE > 0)
    assertPowerOfTwo(ENTITY_SIZE)

    allocate(entityPool, entityBlock, ENTITY_SIZE * N_ENTITIES)

    // assert that some entities will exist at the start of a page.
    // Entities that start on a page are faster than those that don't.
    assert(entityPool & (ENTITY_SIZE - 1) == 0)
}

scope BaseEntity {
    basestruct(BaseEntity)
        struct_maxsize(Entity.ENTITY_SIZE)

        // address of next entity in current list
        // (word addr)
        field(next, 2)

        // address of the function table of the entity
        // (word addr)
        field(functionTable, 2)

        // If entity is in deactivated list, `functionTable` is reused as the
        // address of the entity list to move into when reactivated.
        // (word addr - union with functionTable)
        constant _ft_currentList(functionTable)

        // pointer to the ROM data used by the entity code
        // (word addr)
        field(romPtr, 2)

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

scope Entity {
    // A linked list of entity types
    // (word addr)
    scope lists {
        // Unallocated Entities
        allocate(free, wram7e, 2)

        // Deactivated entities
        //  - will not be processed
        //  - will be reactivated when the camera moves the entity inside
        //  active window
        //
        // NOTE: The engine will reuse the variable `BaseEntity.functionTable`
        // to store the original list id.  The functionTable will be retrieved
        // from the entity's `romPtr`
        allocate(deactivated, wram7e, 2)

        // Entities that could not be activated in this frame
        //  - Gameloop will try and reactivate them on next frame
        //
        // NOTE: The engine will reuse the variable `BaseEntity.functionTable`
        // to store the original list id.  The functionTable will be retrieved
        // from the entity's `romPtr`
        allocate(tryToActivateAgain, wram7e, 2)

        // Entities that are to be deleted
        //  - not processed
        //  - no collisions
        //  - not displayed on screen
        //  - will be destructed and move to free list at the end of frame
        allocate(toDelete, wram7e, 2)

        // Player projectiles
        //  - will be deleted when moved outside active window
        allocate(playerProjectiles, wram7e, 2)

        // Enemy projectiles
        //  - will be deleted when moved outside active window
        allocate(enemyProjectiles, wram7e, 2)


        // Player entity and allies
        //  - collides with: enemies, bosses, enemyProjectiles
        //  - will never be deactivated
        allocate(players, wram7e, 2)

        // Bosses
        //  - collides with: playerProjectiles
        //  - will never be deactivated
        allocate(bosses, wram7e, 2)

        // Enemies
        //  - collides with: playerProjectiles
        //  - deactivated when moved outside active window
        allocate(enemies, wram7e, 2)

        // NPCs
        //  - no entity collisions
        //  - deactivated when moved outside active window
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
        //  - will never be deactivated
        allocate(particles, wram7e, 2)


    constant FIRST(players)
    constant LAST(particles)
    constant N_LISTS((LAST - FIRST) / 2 + 1)
    }

    // Entity list Id used for the BaseEntityData struct
    scope ListId {
        createEnum(0, 2)
            enum(PLAYER)
            enum(BOSS)
            enum(ENEMY)
            enum(NPC)
            enum(PLATFORM)
            enum(PARTICLE)

        assert({__enum.current} == lists.N_LISTS * 2)
    }


    // Current list address being processed
    allocateTmpWord(currentList)

    // Change the list of the entity after processing it
    //
    // If 0 then no list change occurs
    //
    // (word address of list)
    allocateTmpWord(changeList)




    // Active window size (beyond camera display)
    // inactive entities inside this window will become active.
    constant ACTIVE_WINDOW_EXTEND_HORIZONTAL(96)
    constant ACTIVE_WINDOW_EXTEND_VERTICAL(96)

    // Inactive window size (beyond camera display)
    // Active entities that leave the window will deactivate.
    constant INACTIVE_WINDOW_EXTEND_HORIZONTAL(128)
    constant INACTIVE_WINDOW_EXTEND_VERTICAL(128)


    // Activate window
    // (4x uint16)
    scope activeWindow {
        allocate(top, wram7e, 2)
        allocate(bottom, wram7e, 2)
        allocate(left, wram7e, 2)
        allocate(right, wram7e, 2)
    }

    // Inactivate window
    // (4x uint16)
    scope inactiveWindow {
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
    constant TEST_ACTIVE_WINDOW_MASK(~3)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

