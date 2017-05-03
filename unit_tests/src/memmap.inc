
createCodeBlock(code,                   0xc08000, 0xc0ffaf)
createDataBlock(testTableBlock,         0xc00000, 0xc07fff)

createDataBlock(MS_FrameSetList,        0xc10000, 0xc100ff)
createDataBlock(MS_PaletteList,         0xc10100, 0xc101ff)
createDataBlock(MS_FrameList,           0xc10200, 0xc102ff)
createDataBlock(MS_AnimationList,       0xc10300, 0xc103ff)

createDataBlock(MS_FrameSetData,        0xc10400, 0xc10fff)
createDataBlock(MS_FrameData,           0xc11000, 0xc117ff)
createDataBlock(MS_AnimationData,       0xc11800, 0xc11fff)
createDataBlock(MS_FrameObjectsData,    0xc12000, 0xc127ff)
createDataBlock(MS_TileHitboxData,      0xc12800, 0xc12fff)
createDataBlock(MS_EntityHitboxData,    0xc13000, 0xc137ff)
createDataBlock(MS_ActionPointsData,    0xc13800, 0xc13fff)

createDataBlock(MS_PaletteData,         0xc14000, 0xc147ff)

createDataBlock(DMA_Tile16Data,         0xc14800, 0xc14fff)

createDataBlock(MS_TileBlock_0,         0xc15000, 0xc16fff)
createDataBlock(MS_TileBlock_1,         0xc17000, 0xc18fff)

createDataBlock(EN_EntityList,          0xc19000, 0xc1907f)
createDataBlock(EN_ProjectileList,      0xc19080, 0xc190ff)
createDataBlock(EN_RomData,             0xc19100, 0xc19fff)

createDataBlock(rom0,                   0xc1a000, 0xc1cfff)
createDataBlock(testNameBlock,          0xc1d000, 0xc1ffff)


createRamBlock(dp,          0x000000, 0x0000ff)
createRamBlock(entityBlock, 0x7e0100, 0x7e10ff)
createRamBlock(shadow,      0x7e1100, 0x7e1f7f)
createRamBlock(stack,       0x7e1f80, 0x7e1fff)
createRamBlock(wram7e,      0x7e2000, 0x7effff)



// Temporary Variables
// ===================
allocate(dpTmp0, dp, 2)
allocate(dpTmp1, dp, 2)
allocate(dpTmp2, dp, 2)
allocate(dpTmp3, dp, 2)
allocate(dpTmp4, dp, 2)
allocate(dpTmp5, dp, 2)
allocate(dpTmp6, dp, 2)
allocate(dpTmp7, dp, 2)



// Entity Layout
// =============
scope Entity {
    constant ENTITY_SIZE(64)
    constant N_ENTITIES(64)
}

// Allow tests to access entity pool
scope EntityPool {
    evaluate n(0)
    while {n} < Entity.N_ENTITIES {
        constant entity{n}(Entity.entityPool + {n} * Entity.ENTITY_SIZE)
        evaluate n({n} + 1)
    }
}



// VRAM Layout
// ===========
constant VRAM_OBJ_WADDR(0x6000)
constant VRAM_CONSOLE_TILES_WADDR(0x1000)
constant VRAM_CONSOLE_MAP_WADDR(0x0000)


// vim: ft=bass-65816 ts=4 sw=4 et:
