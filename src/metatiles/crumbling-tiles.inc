// metatiles/crumbling-tiles.inc
// =============================
//
// Crumbling and (optionally) re-spawning tiles
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


Warnings.Register(CRUMBLING_LIST_FULL, 16, 16, 20)
RegisterBreakId(DRAW_MAP_CELL_BUFFER_OVERRUN)


namespace MetaTiles {
namespace CrumblingTiles {


// Reset the crumbling tiles subsystem queues and time
//
// REGISTERS: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _ResetQueues() {
    assert16a()
    assert16i()

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
}



// Calculate the transition delays for the crumbling tiles chains
//
// ASSUMES: MetaTile Tileset is loaded into memory
//
// REGISTERS: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _CalculateTransitionDelays() {
    assert8a()
    assert16i()

// ::TODO unify animationSpeed::
constant animationSpeed = Resources.animationSpeed


    // Convert first transition delay from ticks to frames
    ldy.w   Tileset.Footer.CrumblingTilesChainA.firstDelay
    lda.w   animationSpeed
    jsr     Math.Divide.u16y_u8a
    sty.w   CrumblingTiles.ChainA.firstWait

    ldy.w   Tileset.Footer.CrumblingTilesChainB.firstDelay
    lda.w   animationSpeed
    jsr     Math.Divide.u16y_u8a
    sty.w   CrumblingTiles.ChainB.firstWait


    // Convert second transition delay from ticks to frames
    // (unless the delay is 0xffff)
    ldy.w   Tileset.Footer.CrumblingTilesChainA.secondDelay
    cpy.w   #0xffff
    beq     +
        lda.w   animationSpeed
        jsr     Math.Divide.u16y_u8a
    +
    sty.w   CrumblingTiles.ChainA.secondWait

    ldy.w   Tileset.Footer.CrumblingTilesChainB.secondDelay
    cpy.w   #0xffff
    beq     +
        lda.w   animationSpeed
        jsr     Math.Divide.u16y_u8a
    +
    sty.w   CrumblingTiles.ChainB.secondWait
}



// Advance the circular buffer index
//
// REQUIRES: 8 bit Index
// DEFINE: {ns} = ChainA or ChainB
// INPUT: Y = queue index
// OUTPUT: Y = index of the next item in the queue
// KEEP: carry
inline __nextQueueIndex_Y() {
    assert8i()

    // MUST NOT CHANGE CARRY

    dey
    dey
    bpl     NoUnderflow_{#}
        ldy.b   #{ns}.Queue.LAST_INDEX
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



// Increment the crumbling tiles time, rolling back time if necessary.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
macro _IncrementTime() {
    assert16a()
    assert8i()

    constant DECREMENT_AMOUNT = 0x4000
    assert(0xffff / 5 < DECREMENT_AMOUNT)
    assert(0x8000 - DECREMENT_AMOUNT + 0xffff / 5 < 0x8000)

    inc.w   CrumblingTiles.time
    bpl     TimeValid
        // Update time in both queues using a single loop
        assert(CrumblingTiles.N_CRUMBLING_BLOCK_CHAINS == 2)
        assert(CrumblingTiles.ChainA.Queue.time + CrumblingTiles.ChainA.Queue.N_ELEMENTS * 2 == CrumblingTiles.ChainB.Queue.time)
        assert(CrumblingTiles.N_ELEMENTS_IN_BOTH_QUEUES * 2 < 0xff)
        ldy.b   #CrumblingTiles.N_ELEMENTS_IN_BOTH_QUEUES * 2
        Loop:
            lda.w   CrumblingTiles._timeBuffer - 2,y
            sec
            sbc.w   #DECREMENT_AMOUNT
            bcs     +
                lda.w   #0
            +
            sta.w   CrumblingTiles._timeBuffer - 2,y
            dey
            dey
            bne     Loop

        lda.w   CrumblingTiles.time
        sec
        sbc.w   #DECREMENT_AMOUNT
        sta.w   CrumblingTiles.time

TimeValid:
}



// Process a crumbling tile chain.
//
// You MUST NOT call this function if the drawMapCellBuffer is full.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
// REQUIRES: drawMapCellBuffer not full
// PARAM: ns = ChainA or ChainB
inline __Process(ns) {
    assert16a()
    assert8i()


// Process the first queue
// -----------------------
    ldy.w   {ns}.Queue.firstPos
    cpy.w   {ns}.Queue.writerPos
    beq     ProcessSecondQueue


a16()
i8()
    FirstLoop:
        // Y = queue index

        // Schedule second transition
        lda.w   time
    	clc
        adc.w   {ns}.firstWait
        sta.w   {ns}.Queue.time,y


        // Find and draw map cell
        rep     #$30
    i16()
        ldx.w   {ns}.Queue.mapDataIndex,y
        lda.w   {ns}.firstTileId

        // No need to set `map.data,x`, already set in `__Activate` macro

        jsr     Render._DrawMapCell
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        // Goto next item in queue
        ldy.w   {ns}.Queue.firstPos
        __nextQueueIndex_Y()         // does not modify carry
        sty.w   {ns}.Queue.firstPos

        // End loop if drawMapCellBuffer is full
        bcs     EndFirstLoop

        // Loop if not at end of first queue
        cpy.w   {ns}.Queue.writerPos
        bne     FirstLoop
    EndFirstLoop:

a16()
i8()
    // Return early
    // If I remove this I MUST CHECK if the drawMapCellBuffer is full
    rts



// Process the second queue
// ------------------------
a16()
i8()
ProcessSecondQueue:

    ldy.w   {ns}.Queue.secondPos
    cpy.w   {ns}.Queue.firstPos
    beq     ProcessThirdQueue

    lda.w   {ns}.Queue.time,y
    cmp.w   time
    bcs     ProcessThirdQueue
    // Carry clear

a16()
i8()
    SecondLoop:
        // carry clear
        // Y = queue index

        // Schedule third transition
        lda.w   time
        // carry clear
        adc.w   {ns}.secondWait
        sta.w   {ns}.Queue.time,y


        // Set map cell
        rep     #$30
        sep     #$20
    i16()
    a8()
        ldx.w   {ns}.Queue.mapDataIndex,y
        lda.w   {ns}.secondTileId
        sta.w   map.data,x

        rep     #$30
    a16()
        jsr     Render._DrawMapCell
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        // Goto next item in queue
        ldy.w   {ns}.Queue.secondPos
        __nextQueueIndex_Y()         // does not modify carry
        sty.w   {ns}.Queue.secondPos

        // End loop if drawMapCellBuffer is full
        bcs     EndSecondLoop

        // Exit loop if at end of the second queue
        cpy.w   {ns}.Queue.firstPos
        beq     EndSecondLoop

        // Continue loop if time has expired
        lda.w   {ns}.Queue.time,y
        cmp.w   time
        bcc     SecondLoop
    EndSecondLoop:


    // Y = Queue.secondPos
a16()
i8()
    ldx.w   {ns}.secondWait + 1
    bpl     +
        // No third transition if secondWait is negative
        sty.w   {ns}.Queue.thirdPos
    +

    // Return early
    // If I remove this I MUST CHECK if the drawMapCellBuffer is full
    rts



// Process the third queue
// ------------------------
a16()
i8()
ProcessThirdQueue:

    ldy.w   {ns}.Queue.thirdPos
    cpy.w   {ns}.Queue.secondPos
    beq     SkipThirdQueue

    lda.w   {ns}.Queue.time,y
    cmp.w   time
    bcs     SkipThirdQueue


a16()
i8()
    ThirdLoop:
        // Y = queue index

        // Last transition - no more transitions to schedule

        // Set map cell
        rep     #$10
        sep     #$20
    a8()
    i16()
        ldx.w   {ns}.Queue.mapDataIndex,y
        lda.w   {ns}.thirdTileId
        sta.w   map.data,x

        rep     #$30
    a16()
        jsr     Render._DrawMapCell
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        // Goto next item in queue
        ldy.w   {ns}.Queue.thirdPos
        __nextQueueIndex_Y()         // does not modify carry
        sty.w   {ns}.Queue.thirdPos

        // End loop if drawMapCellBuffer is full
        bcs     EndThirdLoop

        // Exit loop if at end of the third queue
        cpy.w   {ns}.Queue.secondPos
        beq     EndThirdLoop

        // Continue loop if time has expired
        lda.w   {ns}.Queue.time,y
        cmp.w   time
        bcc     ThirdLoop
    EndThirdLoop:

a16()
i8()
SkipThirdQueue:
    rts
}
a16()
i8()
farCode()
function _Process_ChainA {
    __Process(ChainA)
}
a16()
i8()
farCode()
function _Process_ChainB {
    __Process(ChainB)
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

