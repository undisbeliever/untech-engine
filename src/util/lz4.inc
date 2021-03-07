// util/lz4.inc
// ============
//
// LZ4 block decompressor.
//
// NOTE:
//   This decompressor uses a modified block format, define
//   USE_STD_LZ4_BLOCK_FRAME in order for this decompressor to use
//   the standard format.
//
//   Block format:
//      <uint16 decompressed size> <lz4 compressed block>
//
//   This decompressor does not process lz4 frames.
//
//
// Literal moves are done using `mvn` instructions instead of DMA so
// the decompressor can be interrupted safely.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(LZ4_OUTPUT_SIZE_INVALID)
RegisterBreakId(LZ4_FAILED)

namespace LZ4 {

code()
evaluate BLOCK_MOVE_IN_PB = ((pc() >> 16) & 0x7e) <= 0x3f


// Pointer the compressed data in ROM
// (dp - faraddr)
allocate(ptr, dp, 3)

// The end of the output buffer
// (dp - addr)
allocate(outputBufferEnd, dp, 2)

allocate(_tmp1, dp, 2)
allocate(_tmp2, dp, 2)
allocate(_tmp3, dp, 2)



// RAM location to store the block move code to
if {BLOCK_MOVE_IN_PB} {
    allocate(blockMoveLiteral, shadow, 6)
    allocate(blockMoveMatch, shadow, 6)
} else {
    allocate(blockMoveLiteral, wram7e, 7)
    allocate(blockMoveMatch, wram7e, 7)
}


// Block move code
// To be copied and executed in RAM
rodata(rom0)
function BlockMoveData {
    if {BLOCK_MOVE_IN_PB} {
        define JMP_INS = jmp
    } else {
        define JMP_INS = jml
    }

    mvn     0=0
    {JMP_INS} Decompress.ReturnFromLiteralMove

    mvn     0=0
    {JMP_INS} Decompress.ReturnFromMatchMove

constant size = pc() - BlockMoveData
assert(blockMoveMatch == blockMoveLiteral + size / 2)

constant destBank = 1
constant sourceBank = 2
}


// Reads the length bytes from an lz4 stream
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0
//
// INPUT: DB:X = lz4 stream ptr
//        A = length from token block
// MODIFIES: _lengthTmp
// KEEP: Y
// OUTPUT: DB:X = lz4 stream ptr after length bytes
//         A = length
macro _ReadLength() {
    assert16a()
    assert16i()

    //  if len == 0xf:
    //      repeat:
    //          a = *(x++) & 0xff
    //          len += a
    //      until a != 0xff

    cmp.w   #0xf
    bne     NoLengthToken{#}
    Loop{#}:
        sta.b   _lengthTmp

        lda.w   0,x
        inx
        and.w   #0xff
        cmp.w   #0xff
        bne     EndLoop{#}
            clc
            adc.b   _lengthTmp
            sta.b   _lengthTmp
            bra     Loop{#}

    EndLoop{#}:
        clc
        adc.b   _lengthTmp

    NoLengthToken{#}:
}


// Decompress an LZ4 block into memory
//
// Will break with error if:
//   * The output size is 0, or
//   * The output size is greater than the maximum allowed, or
//   * The decompressed output size does not match the size in the header
//
// REQUIRES: 16 bit Index, DP = 0
//
// INPUT:
//      ptr: address of compressed source
//      A:X: address to decompress to (MUST not be inside Direct Page)
//      Y: maximum allowed decompressed size
//
// OUTPUT:
//      ptr: end of compressed source
//      outputBufferEnd: the end of the output buffer
//      X: the end of the compressed source
//      Y: the end of the output buffer
au()
i16()
code()
function Decompress {
// only used in setup/header
// (dp)
constant _maxDecompressedSize = _tmp1
constant _dest = _tmp2
constant _destBank = _tmp3

// only used in loop
// (dp)
constant _lengthTmp = _tmp1
constant _token = _tmp2
constant _matchStart = _tmp3

assertZeroPage(ptr)
assertZeroPage(outputBufferEnd)
assertZeroPage(_tmp1)
assertZeroPage(_tmp2)
assertZeroPage(_tmp3)


    php
    phb

    sty.b   _maxDecompressedSize

    sep     #$20
a8()
    stx.b   _dest
    sta.b   _destBank


    // Copy block move code in shadow RAM
    ldy.w   #blockMoveLiteral
    ldx.w   #BlockMoveData
    lda.b   #0
    xba
    lda.b   #BlockMoveData.size - 1

    mvn     blockMoveLiteral>>16=BlockMoveData>>16


    // Modify source/dest banks of block move code
    // DB: blockMoveLiteral bank
    lda.b   ptr + 2
    sta.w   blockMoveLiteral + BlockMoveData.sourceBank

    lda.b   _destBank
    sta.w   blockMoveLiteral + BlockMoveData.destBank
    sta.w   blockMoveMatch + BlockMoveData.sourceBank
    sta.w   blockMoveMatch + BlockMoveData.destBank


    // set DB:X to ptr
    lda.b   ptr + 2
    pha
    plb

    rep     #$30
a16()
i16()
    ldx.b   ptr


    // Read output size from header and calculate outputBufferEnd

    lda.w   0,x                     // output size
    beq     OutputSizeInvalid       // output size == 0 is invalid
    cmp.b   _maxDecompressedSize    // don't decompress data larger than maxDecompressedSize
    bcc     +
    beq     +
OutputSizeInvalid:
        break(LZ4_OUTPUT_SIZE_INVALID)
    +
    clc
    adc.b   _dest
    bcs     OutputSizeInvalid       // prevent bank wrapping
    sta.b   outputBufferEnd

    if {defined USE_STD_LZ4_BLOCK_FRAME} {
        lda.w   2,x
        bne     OutputSizeInvalid   // only compress data < 64KiB in size
        inx
        inx
    }
    inx
    inx


    ldy.b   _dest
    Loop:
        // DB:X = compressed source
        //    Y = output destination

        lda.w   0,x                 // token
        sta.b   _token
        inx

        and.w   #0xf0
        beq     SkipLiteral

            // Optional Literal Block
            lsr
            lsr
            lsr
            lsr

            _ReadLength()

            dec

            phb
            {BlockMoveData.JMP_INS} blockMoveLiteral
        ReturnFromLiteralMove:
            plb
    SkipLiteral:


        // Check if at end of buffer
        cpy.b   outputBufferEnd
        bcs     EndLoop


        // Match Block

        tya
        sec
        sbc.w   0,x                 // match offset
        sta.b   _matchStart
        inx
        inx

        lda.b   _token
        and.w   #0xf
        _ReadLength()
        clc
        adc.w   #4 - 1              // minimum match length is 4, -1 for mvn

        stx.b   ptr

        ldx.b   _matchStart

        phb
        {BlockMoveData.JMP_INS} blockMoveMatch
    ReturnFromMatchMove:
        plb

        ldx.b   ptr


        cpy.b   outputBufferEnd
        bcc     Loop


EndLoop:
    // n clear if y != outputBufferEnd
    bne     Fail

    stx.b   ptr

    plb
    plp
    rts


Fail:
    break(LZ4_FAILED)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

