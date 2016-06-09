arch snes.cpu

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)
define ROM_SPEED(slow)
define REGION(Australia)
define ROM_NAME("UNTECH UNIT TESTS")
define VERSION(0)

include "../../src/common.inc"

createCodeBlock(code,                   0xc08000, 0xc0ffaf)
createDataBlock(testTableBlock,         0xc00000, 0xc07fff)

createDataBlock(MS_FrameSetList,        0xc10000, 0xc100ff)
createDataBlock(MS_PaletteList,         0xc10100, 0xc101ff)
createDataBlock(MS_FrameList,           0xc10200, 0xc102ff)
createDataBlock(MS_AnimationList,       0xc10300, 0xc103ff)

createDataBlock(MS_FrameSetData,        0xc10400, 0xc10fff)
createDataBlock(MS_FrameData,           0xc11000, 0xc117ff)
createDataBlock(MS_FrameObjectsData,    0xc12000, 0xc127ff)
createDataBlock(MS_TileHitboxData,      0xc12800, 0xc12fff)
createDataBlock(MS_EntityHitboxData,    0xc13000, 0xc137ff)
createDataBlock(MS_ActionPointsData,    0xc13800, 0xc13fff)

createDataBlock(MS_PaletteData,         0xc14000, 0xc147ff)

createDataBlock(DMA_Tile16Data,         0xc14800, 0xc14fff)

createDataBlock(MS_TileBlock_0,         0xc15000, 0xc16fff)
createDataBlock(MS_TileBlock_1,         0xc17000, 0xc18fff)

createDataBlock(rom0,                   0xc19000, 0xc1cfff)
createDataBlock(testNameBlock,          0xc1d000, 0xc1ffff)


createRamBlock(dp,     0x000000, 0x000100)
createRamBlock(shadow, 0x7e0100, 0x7e1f80)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)
createRamBlock(wram7e, 0x7e2000, 0x7effff)



include "../../src/dma.inc"
include "../../src/interrupts.inc"
include "../../src/math.inc"
include "../../src/metasprite.inc"


include "resources/metasprite/metasprites.gen.inc"

// ::DEBUG::
scope tmp {
    allocate(tmp, shadow, 26)
    markTmpWord(tmp + 0)
    markTmpWord(tmp + 2)
    markTmpWord(tmp + 4)
    markTmpWord(tmp + 8)
    markTmpWord(tmp + 10)
    markTmpWord(tmp + 12)
    markTmpWord(tmp + 14)
    markTmpWord(tmp + 16)
    markTmpWord(tmp + 18)
    markTmpWord(tmp + 20)
    markTmpWord(tmp + 22)
    markTmpWord(tmp + 24)
}

// ::DEBUG::
// ::TODO move into src/includes somehow::
scope Entities {
    constant ENTITY_SIZE(64)

    allocate(entity0, shadow, ENTITY_SIZE)
    allocate(entity1, shadow, ENTITY_SIZE)
    allocate(entity2, shadow, ENTITY_SIZE)
    allocate(entity3, shadow, ENTITY_SIZE)
    allocate(entity4, shadow, ENTITY_SIZE)
    allocate(entity5, shadow, ENTITY_SIZE)
    allocate(entity6, shadow, ENTITY_SIZE)
    allocate(entity7, shadow, ENTITY_SIZE)
    allocate(entity8, shadow, ENTITY_SIZE)
    allocate(entity9, shadow, ENTITY_SIZE)
    allocate(entity10, shadow, ENTITY_SIZE)
    allocate(entity11, shadow, ENTITY_SIZE)
    allocate(entity12, shadow, ENTITY_SIZE)
}
scope BaseEntity {
    struct()
        MetaSprite.EntityData()
    endstruct()

    assert(size <= Entities.ENTITY_SIZE)
}


include "includes/test-framework.inc"

// A simple test that always succeeds
code()
Test.add("Success Test")
scope SuccessTest: {
    sec
    rts
}

include "tests/dma.inc"
include "tests/math.inc"
include "tests/metasprite.inc"

Test.finalizeTable()

code(code)
CopHandler:
IrqHandler:
EmptyHandler:
    rti

// vim: ft=asm ts=4 sw=4 et:

