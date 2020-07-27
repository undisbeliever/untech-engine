// metatiles/crumbling-tiles.inc
// =============================
//
// Crumbling and (optionally) re-spawning tiles
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


Warnings.Register(CRUMBLING_LIST_FULL, 16, 16, 20)
RegisterBreakId(DRAW_MAP_CELL_BUFFER_OVERRUN)


namespace MetaTiles {
namespace CrumblingTiles {


// Initialize the crumbling tiles subsystem.
//
// ASSUMES: MetaTile Tileset is loaded into memory
//
// REGISTERS: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _Init() {
    assert16a()
    assert16i()

// ::TODO unify animationSpeed::
constant animationSpeed = Resources.animationSpeed


    // Reset time
    stz.w   CrumblingTiles.time

    lda.w   #CrumblingTiles.ChainA.Queue.LAST_INDEX | (CrumblingTiles.ChainA.Queue.LAST_INDEX << 8)
    assert(CrumblingTiles.ChainA.Queue.writerPos + 1 == CrumblingTiles.ChainA.Queue.firstPos)
    sta.w   CrumblingTiles.ChainA.Queue.writerPos
    assert(CrumblingTiles.ChainA.Queue.secondPos + 1 == CrumblingTiles.ChainA.Queue.thirdPos)
    sta.w   CrumblingTiles.ChainA.Queue.secondPos

    assert(CrumblingTiles.ChainA.Queue.LAST_INDEX == CrumblingTiles.ChainB.Queue.LAST_INDEX)
    assert(CrumblingTiles.ChainB.Queue.writerPos + 1 == CrumblingTiles.ChainB.Queue.firstPos)
    sta.w   CrumblingTiles.ChainB.Queue.writerPos
    assert(CrumblingTiles.ChainB.Queue.secondPos + 1 == CrumblingTiles.ChainB.Queue.thirdPos)
    sta.w   CrumblingTiles.ChainB.Queue.secondPos


    sep     #$20
a8()
    // Convert first transition delay from ticks to frames
    ldy.w   tilesetFooter.CrumblingTilesChainA.firstDelay
    lda.w   animationSpeed
    jsr     Math.Divide.u16y_u8a
    sty.w   CrumblingTiles.ChainA.firstWait

    ldy.w   tilesetFooter.CrumblingTilesChainB.firstDelay
    lda.w   animationSpeed
    jsr     Math.Divide.u16y_u8a
    sty.w   CrumblingTiles.ChainB.firstWait


    // Convert second transition delay from ticks to frames
    // (unless the delay is 0xffff)
    ldy.w   tilesetFooter.CrumblingTilesChainA.secondDelay
    cpy.w   #0xffff
    beq     +
        lda.w   animationSpeed
        jsr     Math.Divide.u16y_u8a
    +
    sty.w   CrumblingTiles.ChainA.secondWait

    ldy.w   tilesetFooter.CrumblingTilesChainB.secondDelay
    cpy.w   #0xffff
    beq     +
        lda.w   animationSpeed
        jsr     Math.Divide.u16y_u8a
    +
    sty.w   CrumblingTiles.ChainB.secondWait


    rep     #$30
a16()
}



// Advance the circular buffer index
//
// REQUIRES: 8 bit Index
// DEFINE: {ns} = ChainA or ChainB
// INPUT: X = queue index
// OUTPUT: X = index of the next item in the queue
// KEEP: carry
inline __nextQueueIndex_X() {
    assert8i()

    // MUST NOT CHANGE CARRY

    dex
    dex
    bpl     NoUnderflow_{#}
        ldx.b   #{ns}.Queue.LAST_INDEX
    NoUnderflow_{#}:
}
assert(ChainA.Queue.INDEX_INCREMENT == 2)
assert(ChainA.Queue.LAST_INDEX < 0x80)
assert(ChainB.Queue.LAST_INDEX < 0x80)

// Advance the circular buffer index
//
// REQUIRES: 8 bit A
// DEFINE: {ns} = ChainA or ChainB
// INPUT: A = queue index
// OUTPUT: A = index of the next item in the queue
// KEEP: carry
inline __nextQueueIndex_A() {
    assert8a()

    // MUST NOT CHANGE CARRY

    dec
    dec
    bpl     NoUnderflow_{#}
        lda.b   #{ns}.Queue.LAST_INDEX
    NoUnderflow_{#}:
}
assert(ChainA.Queue.INDEX_INCREMENT == 2)
assert(ChainA.Queue.LAST_INDEX < 0x80)
assert(ChainB.Queue.LAST_INDEX < 0x80)



// Activate a crumbling tile.
//
// NOTE: This function will be called by the interactive tiles system
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// UNKNOWN: DP
// PARAM: ns = ChainA or ChainB
// INPUT: Y = map data index
i16()
a16()
code()
inline __Activate(ns) {
    assert16a()
    assert16i()
    // DP unknown


	// Map bounds check
    cpy.w   map.dataSize
    bcs     DoNotAddToQueue

	sep		#$20
a8()
    // If the tile has already been changed then ignore it
    lda.w   {ns}.firstTileId
    cmp.w   map.data,y
    beq     DoNotAddToQueue


	lda.w	{ns}.Queue.writerPos
	tax

	// Advance writerPos and check if the buffer is full
	// (buffer is full if `__nextQueueIndex(writerPos)` == `thirdPos`)
	__nextQueueIndex_A()
	cmp.w	{ns}.Queue.thirdPos
	beq		BufferFull
	sta.w	{ns}.Queue.writerPos

	// X = old (non-incremented) writerPos (NOTICE: high byte is invalid)
	// Y = map data index

    // Change the map tile to prevent a crumbling tile from being activated twice
    lda.w   {ns}.firstTileId
    sta.w   map.data,y


	// Store map data index in queue
    rep     #$20
a16()
    tya

	// Must use an 8 bit index as the hi-byte of X is unknown.
	// (`sep` must be done after the `tya`)
    sep     #$10
i8()
    sta.w   {ns}.Queue.mapDataIndex,x

au()
iu()
DoNotAddToQueue:
    rep     #$30
a16()
i16()
    rts

a8()
i16()
BufferFull:
    Warnings.Warn(CRUMBLING_LIST_FULL)

    rep     #$30
a16()
i16()
    rts
}


// Activate a crumbling tile
//
// NOTE: This function will be called by the interactive tiles system
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// UNKNOWN: DP
// INPUT: Y = map data index
a16()
i16()
code()
function ActivateChainA {
    __Activate(ChainA)
}

// Activate a crumbling tile.
//
// NOTE: This function will be called by the interactive tiles system
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// UNKNOWN: DP
// INPUT: Y = map data index
a16()
i16()
code()
function ActivateChainB {
    __Activate(ChainB)
}



// Process a crumbling tile chain.
//
// REQUIRES: 8 bit Index, DB = 0x7e, DP = 0
// PARAM: ns = ChainA or ChainB
// MODIFIES: 16 bit A, 8 bit Index
inline __Process(ns) {
    assert8i()

    // Do not process crumbling tiles if the drawMapCellBuffer is full
    ldx.w   drawMapCellBufferIndex
    cpx.b   #drawMapCellBufferIndex.END
    bcs     Return



// Process the first queue
// -----------------------
    rep     #$20
a16()

    ldx.w   {ns}.Queue.firstPos
    cpx.w   {ns}.Queue.writerPos
    beq     ProcessSecondQueue

a16()
i8()
    FirstLoop:
        // X = queue index

        // Schedule second transition
        lda.w   time
    	clc
        adc.w   {ns}.firstWait
        sta.w   {ns}.Queue.time,x


        // Find and draw map cell
        lda.w   {ns}.Queue.mapDataIndex,x
        rep     #$30
    i16()
        tax
        lda.w   {ns}.firstTileId

        // No need to set `map.data,x`, already set in `__Activate` macro

        jsr     _DrawMapCell
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        // Goto next item in queue
        ldx.w   {ns}.Queue.firstPos
        __nextQueueIndex_X()         // does not modify carry
        stx.w   {ns}.Queue.firstPos

        // End loop if drawMapCellBuffer is full
        bcs     EndFirstLoop

        // Loop if not at end of first queue
        cpx.w   {ns}.Queue.writerPos
        bne     FirstLoop
    EndFirstLoop:

    sep     #$30
a8()
i8()

    // Return early
    // If I remove this I MUST CHECK if the SetMapCell buffer is full
    // And I must place a `rts` directly after the `Return` label
Return:
    rts



// Process the second queue
// ------------------------
a16()
i8()
ProcessSecondQueue:

    ldx.w   {ns}.Queue.secondPos
    cpx.w   {ns}.Queue.firstPos
    beq     ProcessThirdQueue

    lda.w   {ns}.Queue.time,x
    cmp.w   time
    bcs     ProcessThirdQueue
    // Carry clear

a16()
i8()
    SecondLoop:
        // carry clear
        // X = queue index

        // Schedule third transition
        lda.w   time
        // carry clear
        adc.w   {ns}.secondWait
        sta.w   {ns}.Queue.time,x


        // Find map cell
        lda.w   {ns}.Queue.mapDataIndex,x
        rep     #$30
    i16()
        tax

        // Set map cell
        sep     #$20
    a8()
        lda.w   {ns}.secondTileId
        sta.w   map.data,x

        rep     #$30
    a16()
        jsr     _DrawMapCell
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        // Goto next item in queue
        ldx.w   {ns}.Queue.secondPos
        __nextQueueIndex_X()         // does not modify carry
        stx.w   {ns}.Queue.secondPos

        // End loop if drawMapCellBuffer is full
        bcs     EndSecondLoop

        // Exit loop if at end of the second queue
        cpx.w   {ns}.Queue.firstPos
        beq     EndSecondLoop

        // Continue loop if time has expired
        lda.w   {ns}.Queue.time,x
        cmp.w   time
        bcc     SecondLoop
    EndSecondLoop:


    // X = Queue.secondPos
a16()
i8()
    ldy.w   {ns}.secondWait + 1
    bpl     +
        // No third transition if secondWait is negative
        stx.w   {ns}.Queue.thirdPos
    +

    // Return early
    // If I remove this I MUST CHECK if the SetMapCell buffer is full
    rts



// Process the third queue
// ------------------------
a16()
i8()
ProcessThirdQueue:

    ldx.w   {ns}.Queue.thirdPos
    cpx.w   {ns}.Queue.secondPos
    beq     SkipThirdQueue

    lda.w   {ns}.Queue.time,x
    cmp.w   time
    bcs     SkipThirdQueue


a16()
i8()
    ThirdLoop:
        // X = queue index

        // Last transition - no more transitions to schedule

        // Find map cell
        lda.w   {ns}.Queue.mapDataIndex,x
        rep     #$10
    i16()
        tax

        // Set map cell
        sep     #$20
    a8()
        lda.w   {ns}.thirdTileId
        sta.w   map.data,x

        rep     #$30
    a16()
        jsr     _DrawMapCell
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        // Goto next item in queue
        ldx.w   {ns}.Queue.thirdPos
        __nextQueueIndex_X()         // does not modify carry
        stx.w   {ns}.Queue.thirdPos

        // End loop if drawMapCellBuffer is full
        bcs     EndThirdLoop

        // Exit loop if at end of the third queue
        cpx.w   {ns}.Queue.secondPos
        beq     EndThirdLoop

        // Continue loop if time has expired
        lda.w   {ns}.Queue.time,x
        cmp.w   time
        bcc     ThirdLoop
    EndThirdLoop:

a16()
i8()
SkipThirdQueue:
    rts
}

au()
i8()
code()
function __Process_ChainA {
    __Process(ChainA)
}
function __Process_ChainB {
    __Process(ChainB)
}



// Process Crumbling Tiles
//
// ASSUMES: There is enough DMA time in `dma.transfersLeft` to fill `drawMapCellBuffer`.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
macro _Process() {
    assert16a()
    assert16i()


    lda.w   CrumblingTiles.time
    inc
    bpl     TimeValid

TimeValid:
    sta.w   CrumblingTiles.time

    sep     #$30
a8()
i8()
    // Alternate between doing chainA and chainB first.
    // It should make it more fair to the system
    lsr
    bcs     OddFrame
        jsr     CrumblingTiles.__Process_ChainA
    a16()
        jsr     CrumblingTiles.__Process_ChainB

        bra     EndIf

    a8()
    i8()
    OddFrame:
        jsr     CrumblingTiles.__Process_ChainB
    a16()
        jsr     CrumblingTiles.__Process_ChainA
a16()
i8()
EndIf:

    rep     #$30
a16()
i16()
}



namespace _DrawMapCell {

a16()
i16()
code()
DrawMapCell_Offscreen:
    // buffer is unchanged so it will never be full
    clc
    rts

au()
iu()
DrawMapCell_BufferFull:
    break(DRAW_MAP_CELL_BUFFER_OVERRUN)
}

// Add a map cell to the drawMapCellBuffer
//
// ASSUMES: The `drawMapCellBuffer` is not full.
//
// INPUT: A = tileId
// INPUT: X = map.data index
// RETURN: carry set if buffer full
a16()
i16()
code()
function _DrawMapCell {
    Render._DrawMapCell()
    cpx.w   #drawMapCellBufferIndex.END
    // carry set if drawMapCellBuffer is full
    rts
}

}
}

// A blank (unused) interactive tile function
namespace InteractiveTiles {

inline __CreateCrumblingTileConstants(chainId) {
    namespace CrumblingTiles_{chainId} {
        constant EntityCollision          = MetaTiles.CrumblingTiles.ActivateChain{chainId}
        constant PlayerOriginCollision    = MetaTiles.CrumblingTiles.ActivateChain{chainId}
        constant PlayerLeftRightCollision = MetaTiles.CrumblingTiles.ActivateChain{chainId}

        constant EntityAirCollision       = NoTileInteraction.NullFunction
        constant PlayerAirCollision       = NoTileInteraction.NullFunction
    }
    namespace CrumblingTiles_{chainId}_PlayerOnly {
        constant PlayerOriginCollision    = MetaTiles.CrumblingTiles.ActivateChain{chainId}
        constant PlayerLeftRightCollision = MetaTiles.CrumblingTiles.ActivateChain{chainId}

        constant EntityCollision          = NoTileInteraction.NullFunction
        constant EntityAirCollision       = NoTileInteraction.NullFunction
        constant PlayerAirCollision       = NoTileInteraction.NullFunction
    }
}
__CreateCrumblingTileConstants(A)
__CreateCrumblingTileConstants(B)

}

// vim: ft=bass-65816 ts=4 sw=4 et:

