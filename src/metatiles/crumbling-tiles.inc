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


    sep     #$30
a8()
i8()
    // Initialize the free list loop
    ldx.b   #(CrumblingTiles.N_ELEMENTS - 1) * 2
    stx.w   CrumblingTiles.ChainA.free
    stx.w   CrumblingTiles.ChainB.free

    Loop:
        txa
        dec
        dec
        sta.w   CrumblingTiles.ChainA.Queue.next,x
        sta.w   CrumblingTiles.ChainB.Queue.next,x

        tax
        bne     Loop

    ldx.b   #0xff
    stx.w   CrumblingTiles.ChainA.Queue.next
    stx.w   CrumblingTiles.ChainB.Queue.next


    // Set head/tail to NULL
    rep     #$10
i16()
    assert(CrumblingTiles.N_ELEMENTS * 2 < 0x80)

    assert(CrumblingTiles.ChainA.firstHead  + 1 == CrumblingTiles.ChainA.firstTail)
    assert(CrumblingTiles.ChainA.secondHead + 1 == CrumblingTiles.ChainA.secondTail)
    assert(CrumblingTiles.ChainA.thirdHead  + 1 == CrumblingTiles.ChainA.thirdTail)

    assert(CrumblingTiles.ChainB.firstHead  + 1 == CrumblingTiles.ChainB.firstTail)
    assert(CrumblingTiles.ChainB.secondHead + 1 == CrumblingTiles.ChainB.secondTail)
    assert(CrumblingTiles.ChainB.thirdHead  + 1 == CrumblingTiles.ChainB.thirdTail)

    ldx.w   #0xffff
    stx.w   CrumblingTiles.ChainA.firstHead
    stx.w   CrumblingTiles.ChainA.secondHead
    stx.w   CrumblingTiles.ChainA.thirdHead
    stx.w   CrumblingTiles.ChainB.firstHead
    stx.w   CrumblingTiles.ChainB.secondHead
    stx.w   CrumblingTiles.ChainB.thirdHead



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


    cpy.w   map.dataSize
    bcs     DoNotAddToQueue

    sep     #$20
a8()
    // If the tile has already been changed then ignore it
    lda.w   {ns}.firstTileId
    cmp.w   map.data,y
    beq     DoNotAddToQueue

    // Change the map tile to prevent a crumbling tile from being activated twice
    sta.w   map.data,y


    // Store mapDataIndex into first free node (if possible)
    rep     #$20
a16()
    tya

    sep     #$10
i8()
    ldy.w   {ns}.free
    bmi     QueueFull

    sta.w   {ns}.Queue.mapDataIndex,y


    // Move node to the first list

    //  y = free
    //  free = y->next
    //  y->next = null
    //
    //  if firstTail == null:
    //      firstHead = y
    //  else:
    //      firstTail->next = y
    //  firstTail = y

    sep     #$30
i8()
a8()
    // Y = {ns}.free
    lda.w   {ns}.Queue.next,y
    sta.w   {ns}.free

    lda.b   #0xff
    sta.w   {ns}.Queue.next,y

    ldx.w   {ns}.firstTail
    bpl     +
        sty.w   {ns}.firstHead
        bra     ++
    +
        tya
        sta.w   {ns}.Queue.next,x
    +
    sty.w   {ns}.firstTail


au()
iu()
DoNotAddToQueue:
    rep     #$30
a16()
i16()
    rts

QueueFull:
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
// REQUIRES: 8 bit A, 8 bit Index, DB = 0x7e, DP = 0
// PARAM: ns = ChainA or ChainB
// PARAM: mapSize = SMALL or LARGE
inline __Process(ns, mapSize) {
    assert8a()
    assert8i()

constant _queueIndex = utDpTmp0


    // Do not process crumbling tiles if the drawMapCellBuffer is full
    ldx.w   drawMapCellBufferIndex
    cpx.b   #drawMapCellBufferIndex.END
    bcs     Return


// Process the first queue
// -----------------------

    ldx.w   {ns}.firstHead
    bmi     SkipFirstQueue


    rep     #$21
a16()
    // carry clear

    i8()
    a16()
    FirstLoop:
        // carry clear

        // X = queue index
        stx.b   _queueIndex


        // Schedule second transition
        lda.w   time
        // carry clear
        adc.w   {ns}.firstWait
        sta.w   {ns}.Queue.time,x


        // Find and draw map cell
        lda.w   {ns}.Queue.mapDataIndex,x
        rep     #$30
    i16()
        tax
        lda.w   {ns}.firstTileId

        // No need to set `map.data,x`, already set in `__Activate` macro

        jsr     _DrawMapCell_{mapSize}
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        ldy.b   _queueIndex

        // End loop if drawMapCellBuffer is full
        bcs     EndFirstLoop

        // Continue loop if next node exists and time has expired
        ldx.w   {ns}.Queue.next,y
        bmi     EndFirstLoop

        lda.w   {ns}.Queue.time,x
        cmp.w   time
        bcc     FirstLoop
    EndFirstLoop:


    // Y = last queue node that was updated
    // Move queue nodes (firstHead to y) from first queue to end of second queue
    sep     #$30
a8()
i8()
    //  if secondTail != NULL:
    //      secondTail->next = firstHead
    //  else:
    //      secondHead = firstHead
    //  firstHead = x->next
    //  if firstHead == NULL:
    //      firstTail = NULL
    //  y->next = NULL
    //  secondTail = y

    lda.w   {ns}.firstHead
    ldx.w   {ns}.secondTail
    bmi     +
        sta.w   {ns}.Queue.next,x
        bra     ++
    +
        sta.w   {ns}.secondHead
    +

    lda.w   {ns}.Queue.next,y
    sta.w   {ns}.firstHead
    bpl     +
        sta.w   {ns}.firstTail
    +

    lda.b   #0xff
    sta.w   {ns}.Queue.next,y

    sty.w   {ns}.secondTail


    // Return early
    // If I remove this I MUST CHECK if the SetMapCell buffer is full
    // And I must place a `rts` directly after the `Return` label
Return:
    rts



i8()
a8()
SkipFirstQueue:


// Process the second queue
// ------------------------

    ldx.w   {ns}.secondHead
    bmi     SkipSecondQueue

    rep     #$20
a16()
    lda.w   {ns}.Queue.time,x
    cmp.w   time
    bcs     SkipSecondQueue
    // Carry clear

i8()
a16()
    SecondLoop:
        // carry clear

        // X = queue index
        stx.b   _queueIndex


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
        jsr     _DrawMapCell_{mapSize}
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        ldy.b   _queueIndex

        // End loop if drawMapCellBuffer is full
        bcs     EndSecondLoop

        // Continue loop if next node exists and time has expired
        ldx.w   {ns}.Queue.next,y
        bmi     EndSecondLoop

        lda.w   {ns}.Queue.time,x
        cmp.w   time
        bcc     SecondLoop
    EndSecondLoop:


    // Y = last queue node that was drawn
    // Move queue nodes (secondHead to y) from second queue to end of third queue
    sep     #$30
a8()
i8()
    //
    //  if thirdTail != NULL:
    //      thirdTail->next = secondHead
    //  else:
    //      thirdHead = secondHead
    //  secondHead = x->next
    //  if secondHead == NULL:
    //      secondTail = NULL
    //  y->next = NULL
    //  thirdTail = y
    lda.w   {ns}.secondHead
    ldx.w   {ns}.thirdTail
    bmi     +
        sta.w   {ns}.Queue.next,x
        bra     ++
    +
        sta.w   {ns}.thirdHead
    +

    lda.w   {ns}.Queue.next,y
    sta.w   {ns}.secondHead
    bpl     +
        sta.w   {ns}.secondTail
    +

    lda.b   #0xff
    sta.w   {ns}.Queue.next,y

    sty.w   {ns}.thirdTail


    // Return early
    // If I remove this I MUST CHECK if the SetMapCell buffer is full
    rts


a8()
i8()
NoThirdTransition:
    // Y = last queue node that was drawn
    // Move queue nodes (secondHead to y) from second queue to the free list

    //  x = secondHead
    //
    //  secondHead = y->next
    //  if secondHead == NULL:
    //      secondTail = NULL
    //
    //  y->next = free
    //  free = x

    ldx.w   {ns}.secondHead

    lda.w   {ns}.Queue.next,y
    sta.w   {ns}.secondHead
    bpl     +
        sta.w   {ns}.secondTail
    +

    lda.w   {ns}.free
    sta.w   {ns}.Queue.next,y

    stx.w   {ns}.free


    // Return early
    // If I remove this I MUST CHECK if the SetMapCell buffer is full
    rts


a8()
i8()
SkipSecondQueue:


// Process the third queue
// ------------------------

    ldx.w   {ns}.thirdHead
    bmi     SkipThirdQueue

    rep     #$20
a16()
    lda.w   {ns}.Queue.time,x
    cmp.w   time
    bcs     SkipThirdQueue


i8()
a16()
    ThirdLoop:
        // X = queue index
        stx.b   _queueIndex

        // Last transition - no need to schedule another transition

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
        jsr     _DrawMapCell_{mapSize}
        // Carry set if drawMapCellBuffer is full


        sep     #$10
    i8()
        ldy.b   _queueIndex

        // End loop if drawMapCellBuffer is full
        bcs     EndThirdLoop

        // Continue loop if next node exists and time has expired
        ldx.w   {ns}.Queue.next,y
        bmi     EndThirdLoop

        lda.w   {ns}.Queue.time,x
        cmp.w   time
        bcc     ThirdLoop
    EndThirdLoop:


    sep     #$30
a8()
i8()
    // Y = last queue node that was updated
    // Move queue nodes (thirdHead to y) from third queue to the free list

    //  x = thirdHead
    //
    //  thirdHead = y->next
    //  if thirdHead == NULL:
    //      thirdTail = NULL
    //
    //  y->next = free
    //  free = x

    ldx.w   {ns}.thirdHead

    lda.w   {ns}.Queue.next,y
    sta.w   {ns}.thirdHead
    bpl     +
        sta.w   {ns}.thirdTail
    +

    lda.w   {ns}.free
    sta.w   {ns}.Queue.next,y

    stx.w   {ns}.free


a8()
i8()
SkipThirdQueue:
    rts
}

a8()
i8()
code()
function __Process_ChainA_Small {
    __Process(ChainA, Small)
}
function __Process_ChainB_Small {
    __Process(ChainB, Small)
}
function __Process_ChainA_Large {
    __Process(ChainA, Large)
}
function __Process_ChainB_Large {
    __Process(ChainB, Large)
}



// Process Crumbling Tiles
//
// This function should be called in the gameloop before the players, entities
// or MetaTiles are processed.
//
// ASSUMES: There is enough DMA time in `dma.transfersLeft` to fill `drawMapCellBuffer`.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
function Process {
    lda.w   time
    inc
    bpl     TimeValid

TimeValid:
    sta.w   time

    sep     #$30
a8()
i8()
    assert(MAP_HEIGHT_LARGE == 0x80)
    bit.w   map.bytesPerColumn
    bmi     LargeMapHeight
        // Map height is small
        // A = time

        // Alternate between doing chainA and chainB first.
        // It should make it more fair to the system
        lsr
        bcs +
            jsr     __Process_ChainA_Small
            jsr     __Process_ChainB_Small

            rep     #$30
        a16()
        i16()
            rts

        +
            jsr     __Process_ChainB_Small
            jsr     __Process_ChainA_Small

            rep     #$30
        a16()
        i16()
            rts

    a8()
    i8()
    LargeMapHeight:
        // A = time
        lsr
        bcs +
            jsr     __Process_ChainA_Large
            jsr     __Process_ChainB_Large

            rep     #$30
        a16()
        i16()
            rts

        +
            jsr     __Process_ChainB_Large
            jsr     __Process_ChainA_Large

            rep     #$30
        a16()
        i16()
            rts
}



namespace _DrawMapCell_Small {
a16()
i16()
code()
DrawMapCell_Offscreen:
    // buffer is unchanged so it will never be full
    clc
    rts
}

// Add a map cell to the drawMapCellBuffer (large map height).
//
// ASSUMES: The `drawMapCellBuffer` is not full.
//
// INPUT: A = tileId
// INPUT: X = map.data index
// RETURN: carry set if buffer full
a16()
i16()
code()
function _DrawMapCell_Small {
    // ::TODO skip dataSize check::
    // ::TODO skip drawMapCell check::
    Render._DrawMapCell(SMALL)
    cpx.w   #drawMapCellBufferIndex.END
    // carry set if drawMapCellBuffer is full
    rts


au()
iu()
DrawMapCell_BufferFull:
    // This should not happen.
    // `__Process` macro contains bufferIndex checks to prevent this from happening
    break(DRAW_MAP_CELL_BUFFER_OVERRUN)
}



namespace _DrawMapCell_Large {
a16()
i16()
code()
DrawMapCell_Offscreen:
    // buffer is unchanged so it will never be full
    clc
    rts
}

// Add a map cell to the drawMapCellBuffer (large map height).
//
// ASSUMES: The `drawMapCellBuffer` is not full.
//
// INPUT: A = tile
// INPUT: X = map.data index
// RETURN: carry set if buffer full
a16()
i16()
code()
function _DrawMapCell_Large {
    // ::TODO skip dataSize check::
    // ::TODO skip drawMapCell check::
    Render._DrawMapCell(LARGE)
    cpx.w   #drawMapCellBufferIndex.END
    // carry set if drawMapCellBuffer is full
    rts


au()
iu()
DrawMapCell_BufferFull:
    // `__Process` macro contains bufferIndex checks to prevent this from happening
    break(DRAW_MAP_CELL_BUFFER_OVERRUN)
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

