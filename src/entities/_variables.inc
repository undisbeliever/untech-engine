
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

// vim: ft=asm ts=4 sw=4 et:

