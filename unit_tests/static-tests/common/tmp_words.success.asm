// Test tmp words

// ::TODO validate binary output::

architecture wdc65816

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"
include "../../../src/common/tmp_words.inc"

createRamBlock(dp,     0x000000, 0x000100)
createRamBlock(shadow, 0x7e0100, 0x7e1f80)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)


namespace test {
    // test requesting a tmp before it's marked

    allocateTmpWord(tmp0)
    allocateTmpWord(tmp1)

    lda.w   tmp0
    sta.w   tmp1
}


allocate(dataBlock, shadow, 0x200)

markTmpWord(dataBlock + 0x0e)
markTmpWord(dataBlock + 0x1e)
markTmpWord(dataBlock + 0x2e)
markTmpWord(dataBlock + 0x3e)
markTmpWord(dataBlock + 0x4e)


namespace test {
    // test requesting a tmp after it's marked

    allocateTmpWord(tmp2)
    allocateTmpWord(tmp3)

    lda.w   tmp2
    sta.w   tmp3
}

assert(test.tmp2 == dataBlock + 0x2e)
assert(test.tmp3 == dataBlock + 0x3e)


// vim: ft=bass-65816 ts=4 sw=4 et:

