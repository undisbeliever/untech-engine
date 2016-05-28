// Test tmp words

arch snes.cpu

define MEMORY_MAP(LOROM)
define ROM_SIZE(1)

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"
include "../../../src/common/tmp_words.inc"

createRamBlock(dp,     0x000000, 0x000100)
createRamBlock(shadow, 0x7e0100, 0x7e1f80)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)


scope test {
    // test requesting a tmp before it's marked

    allocateTmpWord(tmp0)
    allocateTmpWord(tmp1)

    lda     tmp0
    sta     tmp1
}

allocate(dataBlock, shadow, 0x200)

markTmpWord(dataBlock + 0x0e)
markTmpWord(dataBlock + 0x1e)
markTmpWord(dataBlock + 0x2e)
markTmpWord(dataBlock + 0x3e)
markTmpWord(dataBlock + 0x4e)

assert(test.tmp0 == dataBlock + 0x0e)
assert(test.tmp1 == dataBlock + 0x1e)

scope test {
    // test requesting a tmp after it's marked

    allocateTmpWord(tmp2)
    allocateTmpWord(tmp3)

    lda     tmp2
    sta     tmp3
}

assert(test.tmp2 == dataBlock + 0x2e)
assert(test.tmp3 == dataBlock + 0x3e)


// vim: ft=asm ts=4 sw=4 et:

