
scope Entities {
    assert(Entities.N_ENTITIES >= 10)

    // assert ENTITY_SIZE is a power of 2
    assert(Entities.ENTITY_SIZE > 0)
    assert(Entities.ENTITY_SIZE & (Entities.ENTITY_SIZE - 1) == 0)

    allocate(entityPool, entityBlock, ENTITY_SIZE * N_ENTITIES)

    // assert that some entities will exist at the start of a page.
    // Entities that start on a page are faster than those that don't.
    assert(entityPool & (Entities.ENTITY_SIZE - 1) == 0)
}

scope BaseEntity {
    basestruct(BaseEntity)
        struct_maxsize(Entities.ENTITY_SIZE)

        // address of next entity in current list
        // (word addr)
        field(next, 2)

        // address of the function table of the entity
        //
        // If entity is in deactivated list, this address is reused as the
        // address of the entity list to move into when reactivated.
        // (word addr)
        field(functionTable, 2)

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
        // (2 x 1:7:16 fixed point - pixels/display frame)
        // to save CPU time velocity comparisons will be preformed
        // using the upper 16 bits (1:7:8 fixed point)
        field(xVecl, 1)
        field(xVecl.hw, 2)
        field(yVecl, 1)
        field(yVecl.hw, 2)

        // invincibility frames.
        // NOTE: iframes are ignored in projectiles and particles
        // (byte - frame count)
        field(iframes, 1)
    endstruct()
}

scope Entities {
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

        // Entities that are to be deleted
        //  - not processed
        //  - no collisions
        //  - not displayed on screen
        //  - will be destructed and move to free list at the end of frame
        allocate(toDelete, wram7e, 2)

        // this is in rendering order

    constant FIRST(players)
        // Player entity and allies
        //  - collides with: platforms, enemies, bosses, enemyProjectiles
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

        // Player projectiles
        //  - will be deleted when moved outside active window
        allocate(playerProjectiles, wram7e, 2)

        // Enemy projectiles
        //  - deactivated when moved outside active window
        allocate(enemyProjectiles, wram7e, 2)

        // NPCs
        //  - no entity collisions
        //  - deactivated when moved outside active window
        allocate(npcs, wram7e, 2)

        // Platforms
        //  - collides with: players, enemies (using the tileHitbox)
        //  - deactivated when moved outside active window
        allocate(platforms, wram7e, 2)

        // Particles
        //  - no entity collisions
        //  - will never be deactivated
        allocate(particles, wram7e, 2)

    constant LAST(particles)
    constant N_LISTS((LAST - FIRST) / 2 + 1)
    }
}


scope Entities {
    // Current list address being processed
    allocateTmpWord(currentList)

    // Change the list of the entity after processing it
    //
    // If 0 then no list change occurs
    //
    // (word address of list)
    allocateTmpWord(changeList)
}

// vim: ft=asm ts=4 sw=4 et:

