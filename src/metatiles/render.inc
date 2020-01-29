// metatiles/render.inc
// ====================
//
// MetaTile map renderer.
//
// Assumes:
//    * Camera xPos/yPos always inside map
//    * only Renderer edits bg1 ScrollBuffer
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
namespace Render {

allocateTmpWord(tmp0)
allocateTmpWord(tmp1)
allocateTmpWord(tmp2)


// Address of the top-left most visible cell
// (word index)
allocate(visibleMapPtr, wram7e, 2)


// Offset between the Camera position and the display offset
// Reset on every DrawFullScreen_ForceBlank
// (2x sint16)
allocate(cameraXoffset, wram7e, 2)
allocate(cameraYoffset, wram7e, 2)


// Display offset of the tilemap
constant displayHoffset = ScrollBuffer.bg1.hOffset
constant displayVoffset = ScrollBuffer.bg1.vOffset


assertPowerOfTwo(horizontalBuffer.buffer.size)
assertPowerOfTwo(verticalBuffer.buffer.size)


// Draw the visible map directly to the VRAM
//
// Also initializes the buffer state
//
// ASSUMES: Camera position inside map
//
// REQUIRES: Force Blank
// REQUIRES: Map loaded
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Force Blank
a16()
i16()
code()
function DrawFullScreen_ForceBlank {

constant _mapPtrTmp = tmp0
constant _vramAddr = tmp1
constant _mapEndColumnPtr = tmp2


    Dma.ForceBlank.AssertForceBlank()

    phd

    // set DP to registers
    lda.w   #0x2100
    tcd


    // Reset render state
    //
    // drawMapCellBufferIndex = 0
    // horizontalBuffer.cursorIndex = 0
    // horizontalBuffer.cursorYpos = Camera.yPos & ~(METATILE_SIZE_PX - 1)
    //
    // verticalBuffer.cursorIndex = 0
    // verticalBuffer.cursorXpos = Camera.xPos & ~(METATILE_SIZE_PX - 1)
    //
    // cameraXoffset = Camera.xPos & ~(METATILE_SIZE_PX - 1)
    // cameraYoffset = (Camera.yPos & ~(METATILE_SIZE_PX - 1)) + 1
    //
    // displayHoffset = Camera.xPos & (METATILE_SIZE_PX - 1)
    // displayVoffset = Camera.yPos & (METATILE_SIZE_PX - 1) - 1
    //
    // visibleMapPtr = (Camera.yPos & Y_POSITION_MASK) / METATILE_SIZE_PX
    //                 + (Camera.xPos & X_POSITION_MASK) / METATILE_SIZE_PX * map.HEIGHT
    //                 + map.data

    stz.w   drawMapCellBufferIndex

    stz.w   horizontalBuffer.cursorIndex

    lda.w   Camera.yPos
    and.w   #~(METATILE_SIZE_PX - 1)
    sta.w   horizontalBuffer.cursorYpos
    inc
    sta.w   cameraYoffset

    stz.w   verticalBuffer.cursorIndex

    lda.w   Camera.xPos
    and.w   #~(METATILE_SIZE_PX - 1)
    sta.w   verticalBuffer.cursorXpos
    sta.w   cameraXoffset

    lda.w   Camera.xPos
    and.w   #METATILE_SIZE_PX - 1
    sta.w   displayHoffset

    lda.w   Camera.yPos
    and.w   #METATILE_SIZE_PX - 1
    dec
    sta.w   displayVoffset


    assertPowerOfTwo(map.HEIGHT)
    assertPowerOfTwo(METATILE_SIZE_PX)
    lda.w   Camera.yPos
    and.w   #Y_POSITION_MASK
    lsr
    lsr
    lsr
    lsr
    sta.w   _mapPtrTmp

    lda.w   Camera.xPos
    and.w   #X_POSITION_MASK
    assert(map.HEIGHT / METATILE_SIZE_PX == 4)
    asl
    asl
    // carry clear
    adc.w   _mapPtrTmp
    clc
    adc.w   #map.data
    sta.w   visibleMapPtr


    // Draw Tilemap
    //
    //  VMAIN = incrementMode.high | increment.by32
    //
    //  x = visibleMapPtr
    //  _mapEndColumnPtr = x + _TILES_PER_COLUMN
    //
    //  _vramAddr = METATILES_MAP_WADDR
    //
    //  repeat:
    //      VMADD = _vramAddr
    //      repeat:
    //          y = *x * 2
    //          VMDATA = tileMap.topLeft[y]
    //          VMDATA = tileMap.bottomLeft[y]
    //      until x == _mapEndColumnPtr
    //
    //      x -= _TILES_PER_COLUMN
    //
    //      VMADD = _vramAddr + 1
    //      repeat:
    //          y = *x * 2
    //          VMDATA = tileMap.topRight[y]
    //          VMDATA = tileMap.bottomRight[y]
    //          x++
    //      until x == _mapEndColumnPtr
    //
    //      x += map.HEIGHT - _TILES_PER_COLUMN
    //      _mapEndColumnPtr = x + _TILES_PER_COLUMN
    //
    //      _vramAddr += 2
    //      if _vramAddr == METATILES_MAP_WADDR + 32
    //          _vramAddr = METATILES_MAP_WADDR + 32 * 32
    //  until _vramAddr >= _VRAM_WADDR_END

    constant _TILES_PER_COLUMN = METATILE_DISPLAY_HEIGHT + 1
    constant _VRAM_WADDR_END = METATILES_MAP_WADDR + 32 * 32 + (METATILE_DISPLAY_WIDTH - 16 + 1) * 2

    // A = visibleMapPtr
    tax
    clc
    adc.w   #_TILES_PER_COLUMN
    sta.w   _mapEndColumnPtr

    sep     #$20
a8()
    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by32
    sta.b   VMAIN

    assert(METATILE_DISPLAY_WIDTH * METATILE_SIZE == 32)

    rep     #$30
a16()

    lda.w   #METATILES_MAP_WADDR

    ColumnLoop:
        // A = _vramAddr
        sta.w   _vramAddr
        sta.b   VMADD

        LeftLoop:
            lda.w   0,x
            and.w   #0xff
            asl
            tay

            lda.w   tileMap.topLeft,y
            sta.b   VMDATA

            lda.w   tileMap.bottomLeft,y
            sta.b   VMDATA

            inx
            cpx.w   _mapEndColumnPtr
            bne     LeftLoop

        txa
        sec
        sbc.w   #_TILES_PER_COLUMN
        tax

        lda.w   _vramAddr
        inc
        sta.b   VMADD

        RightLoop:
            lda.w   0,x
            and.w   #0xff
            asl
            tay

            lda.w   tileMap.topRight,y
            sta.b   VMDATA

            lda.w   tileMap.bottomRight,y
            sta.b   VMDATA

            inx
            cpx.w   _mapEndColumnPtr
            bne     RightLoop

        txa
        clc
        adc.w   #map.HEIGHT - _TILES_PER_COLUMN
        tax
        clc
        adc.w   #_TILES_PER_COLUMN
        sta.w   _mapEndColumnPtr

        lda.w   _vramAddr
        inc
        inc
        cmp.w   #METATILES_MAP_WADDR + 32
        bne     +
            lda.w   #METATILES_MAP_WADDR + 32 * 32
        +
        cmp.w   #_VRAM_WADDR_END
        bcc     ColumnLoop

    pld

    rts
}



// Draw Horizontal Buffer
//
// Input: A = address of map segment to draw
//        Y = vBuffer offset (0 or DISPLAY_HEIGHT * 2)
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _DrawHorizontalBuffer() {
    assert16a()
    assert16i()

    assert(verticalBuffer.buffer.size == 32 * 2 * 2)
    assert(horizontalBuffer.buffer.size == 64 * 2 * 2)
    // prevent ptr under-run
    assert(map.data > 2)

    assertZeroPage(_mapPtr)
    assertZeroPage(_mapPtrRend)

    //  mapPtrRend = mapPtr
    //  mapPtr += METATILE_DISPLAY_WIDTH * map.HEIGHT
    //
    //  a = (verticalBuffer.cursorIndex + vBufferOffset) / 4 * 32 * 2
    //  a &= (32 - 1) * 32
    //  horizontalBuffer.vramWaddr = METATILES_MAP_WADDR + a
    //
    //  y = (horizontalBuffer.cursorIndex + DISPLAY_WIDTH) & 127
    //
    //  while mapPtr >= mapPtrRend:
    //      x = *mapPtr * 2
    //      buffer[y] = tileMap.topLeft[x]
    //      buffer[y + 2] = tileMap.topRight[x]
    //      buffer[y + 128] = tileMap.bottomLeft[x]
    //      buffer[y + 128 + 2] = tileMap.bottomRight[x]
    //
    //      y -= 2 // 4 bytes
    //      if y < 0:
    //          y = buffer.size / 2 - 4 bytes
    //
    //      mapPtr -= map.HEIGHT

    sta.b   _mapPtrRend
    clc
    adc.w   #METATILE_DISPLAY_WIDTH * map.HEIGHT
    tax

    tya
    clc
    adc.w   verticalBuffer.cursorIndex
    and.w   #((32 - 1) * 32) >> 4
    asl
    asl
    asl
    asl
    // c clear
    adc.w   #METATILES_MAP_WADDR
    sta.w   horizontalBuffer.vramWaddr

    lda.w   horizontalBuffer.cursorIndex
    clc
    adc.w   #METATILE_DISPLAY_WIDTH * METATILE_SIZE * 2
    and.w   #127
    tay

    txa
    Loop:
        sta.b   _mapPtr

        lda     (_mapPtr)
        and.w   #0xff
        asl
        tax

        lda.w   tileMap.topLeft,x
        sta.w   horizontalBuffer.buffer,y

        lda.w   tileMap.topRight,x
        sta.w   horizontalBuffer.buffer + 2,y

        lda.w   tileMap.bottomLeft,x
        sta.w   horizontalBuffer.buffer + 128,y

        lda.w   tileMap.bottomRight,x
        sta.w   horizontalBuffer.buffer + 128 + 2,y

        dey
        dey
        dey
        dey
        bpl     +
            ldy.w   #horizontalBuffer.buffer.size / 2 - 4
        +

        lda.b   _mapPtr
        sec
        sbc.w   #map.HEIGHT
        cmp.b   _mapPtrRend
        bcs     Loop

    Dma.MetaTiles.QueueHorizontalBuffer()
}



// Draw Vertical Buffer
//
// Input: A = address of map segment to draw
//        Y = hBuffer offset (0 or DISPLAY_WIDTH * 2)
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _DrawVerticalBuffer() {
    assert16a()
    assert16i()

    assert(verticalBuffer.buffer.size == 32 * 2 * 2)
    assert(horizontalBuffer.buffer.size == 64 * 2 * 2)
    // prevent ptr under-run (max value of map.bytesPerRow is 512)
    assert(map.data > 512)

    assertZeroPage(_mapPtr)
    assertZeroPage(_mapPtrRend)

    //  mapPtrRend = mapPtr
    //  mapPtr += METATILE_DISPLAY_HEIGHT
    //
    //  a = (horizontalBuffer.cursorIndex + hBufferOffset) / 4 * 2
    //  a &= 32 * 2 - 1
    //  if a & 32:
    //      a ^= 32 + 32 * 32
    //  verticalBuffer.vramWaddr = METATILES_MAP_WADDR + a
    //
    //  y = horizontalBuffer.cursorIndex + METATILE_DISPLAY_WIDTH * METATILE_SIZE * 2
    //
    //  while mapPtr >= mapPtrRend:
    //      x = *mapPtr * 2
    //      buffer[y] = tileMap.topLeft[x]
    //      buffer[y + 2] = tileMap.bottomLeft[x]
    //      buffer[y + 64] = tileMap.topRight[x]
    //      buffer[y + 64 + 2] = tileMap.bottomRight[x]
    //
    //      y -= 2 // 4 bytes
    //      if y < 0:
    //          y = buffer.size / 2 - 4 bytes
    //
    //      mapPtr--

    sta.b   _mapPtrRend
    clc
    adc.w   #METATILE_DISPLAY_HEIGHT
    tax

    tya
    clc
    adc.w   horizontalBuffer.cursorIndex
    lsr
    and.w   #32 * 2 - 1
    bit.w   #32
    beq     +
        eor.w   #32 + (32 * 32)
    +
    clc
    adc.w   #METATILES_MAP_WADDR
    sta.w   verticalBuffer.vramWaddr


    lda.w   verticalBuffer.cursorIndex
    clc
    adc.w   #METATILE_DISPLAY_HEIGHT * METATILE_SIZE * 2
    and.w   #63
    tay

    Loop:
        stx.b   _mapPtr

        lda.w   0,x
        and.w   #0xff
        asl
        tax

        lda.w   tileMap.topLeft,x
        sta.w   verticalBuffer.buffer,y

        lda.w   tileMap.bottomLeft,x
        sta.w   verticalBuffer.buffer + 2,y

        lda.w   tileMap.topRight,x
        sta.w   verticalBuffer.buffer + 64,y

        lda.w   tileMap.bottomRight,x
        sta.w   verticalBuffer.buffer + 64 + 2,y

        dey
        dey
        dey
        dey
        bpl     +
            ldy.w   #verticalBuffer.buffer.size / 2 - 4
        +

        ldx.b   _mapPtr
        dex
        cpx.b   _mapPtrRend
        bcs     Loop

    Dma.MetaTiles.QueueVerticalBuffer()
}



// Updates the position of the map, drawing new tiles as necessary
//
// Will change the camera position to ensure only 1 metatile row and/or column
// is processed per frame.
//
// This routine SHOULD BE called early in the game loop (ie, before
// `Entities.ProcessGameLoop` and `Resources.ProcessAnimations`) to prevent a
// VBlank overrun.
//
// ASSUMES: DrawFullScreen_ForceBlank has been called to setup map state
// ASSUMES: Camera position inside map
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function Update {
constant _mapPtr = utDpTmp0
constant _mapPtrRend = utDpTmp2

    //  xOffset = Camera.xPos - verticalBuffer.cursorXpos
    //
    //  if xOffset > METATILE_SIZE_PX:
    //      if xOffset >= METATILE_SIZE_PX * 2:
    //          Camera.xPos = verticalBuffer.cursorXpos + METATILE_SIZE_PX * 2 - 1
    //
    //      horizontalBuffer.cursorIndex += METATILE_SIZE * 2 bytes
    //      horizontalBuffer.cursorIndex &= horizontalBuffer.buffer.size / 2 - 1
    //
    //      verticalBuffer.cursorXpos += METATILE_SIZE_PX
    //      visibleMapPtr += map.HEIGHT
    //
    //      _DrawVerticalBuffer(visibleMapPtr + METATILE_DISPLAY_WIDTH * map.HEIGHT,
    //                          METATILE_DISPLAY_WIDTH * METATILE_SIZE * 2)
    //
    //  else if xOffset < 0:
    //      if xOffset <= METATILE_SIZE_PX:
    //          Camera.xPos = verticalBuffer.cursorXpos - METATILE_SIZE_PX + 1
    //
    //      horizontalBuffer.cursorIndex -= METATILE_SIZE * 2 bytes
    //      horizontalBuffer.cursorIndex &= horizontalBuffer.buffer.size / 2 - 1
    //
    //      verticalBuffer.cursorXpos -= METATILE_SIZE_PX
    //      visibleMapPtr -= map.HEIGHT
    //
    //      _DrawVerticalBuffer(visibleMapPtr, 0)

    lda.w   Camera.xPos
    sec
    sbc.w   verticalBuffer.cursorXpos
    bmi     XOffsetNegative
    cmp.w   #METATILE_SIZE_PX
    bcs     +
        jmp     SkipDrawVerticalBuffer
    +
        cmp.w   #METATILE_SIZE_PX * 2
        bcc     +
            lda.w   verticalBuffer.cursorXpos
            clc
            adc.w   #METATILE_SIZE_PX * 2 - 1
            sta.w   Camera.xPos
        +

        lda.w   horizontalBuffer.cursorIndex
        clc
        adc.w   #METATILE_SIZE * 2
        and.w   #horizontalBuffer.buffer.size / 2 - 1
        sta.w   horizontalBuffer.cursorIndex

        lda.w   verticalBuffer.cursorXpos
        clc
        adc.w   #METATILE_SIZE_PX
        sta.w   verticalBuffer.cursorXpos

        lda.w   visibleMapPtr
        clc
        adc.w   #map.HEIGHT
        sta.w   visibleMapPtr
        clc
        adc.w   #METATILE_DISPLAY_WIDTH * map.HEIGHT

        ldy.w   #METATILE_DISPLAY_WIDTH * METATILE_SIZE * 2

        bra     DrawVerticalBuffer

XOffsetNegative:
        cmp.w   #-METATILE_SIZE_PX + 1
        bcs     +
            lda.w   verticalBuffer.cursorXpos
            sec
            sbc.w   #METATILE_SIZE_PX - 1
            sta.w   Camera.xPos
        +

        lda.w   horizontalBuffer.cursorIndex
        sec
        sbc.w   #METATILE_SIZE * 2
        and.w   #horizontalBuffer.buffer.size / 2 - 1
        sta.w   horizontalBuffer.cursorIndex

        lda.w   verticalBuffer.cursorXpos
        sec
        sbc.w   #METATILE_SIZE_PX
        sta.w   verticalBuffer.cursorXpos

        lda.w   visibleMapPtr
        sec
        sbc.w   #map.HEIGHT
        sta.w   visibleMapPtr

        ldy.w   #0

    DrawVerticalBuffer:
        // A = map Index, Y = vramOffset
        _DrawVerticalBuffer()

SkipDrawVerticalBuffer:


    //  yOffset = Camera.yPos - horizontalBuffer.cursorYpos
    //
    //  if yOffset > METATILE_SIZE_PX:
    //      if yOffset >= METATILE_SIZE_PX * 2:
    //          Camera.yPos = horizontalBuffer.cursorYpos + METATILE_SIZE_PX * 2 - 1
    //
    //      verticalBuffer.cursorIndex += METATILE_SIZE * 2 bytes
    //      verticalBuffer.cursorIndex &= verticalBuffer.buffer.size / 2 - 1
    //
    //      horizontalBuffer.cursorYpos += METATILE_SIZE_PX
    //      visibleMapPtr++
    //
    //      _DrawHorizontalBuffer(visibleMapPtr + METATILE_DISPLAY_HEIGHT,
    //                            METATILE_DISPLAY_HEIGHT * METATILE_SIZE * 2)
    //
    //  else if yOffset < 0:
    //      if yOffset <= METATILE_SIZE_PX:
    //          Camera.yPos = horizontalBuffer.cursorYpos - METATILE_SIZE_PX + 1
    //
    //      verticalBuffer.cursorIndex -= METATILE_SIZE * 2 bytes
    //      verticalBuffer.cursorIndex &= verticalBuffer.buffer.size / 2 - 1
    //
    //      horizontalBuffer.cursorYpos -= METATILE_SIZE_PX
    //      visibleMapPtr--
    //
    //      _DrawHorizontalBuffer(visibleMapPtr, 0)

    lda.w   Camera.yPos
    sec
    sbc.w   horizontalBuffer.cursorYpos
    bmi     YOffsetNegative
    cmp.w   #METATILE_SIZE_PX
    bcs     +
        jmp     SkipDrawHorizontalBuffer
    +
        cmp.w   #METATILE_SIZE_PX * 2
        bcc     +
            lda.w   horizontalBuffer.cursorYpos
            clc
            adc.w   #METATILE_SIZE_PX * 2 - 1
            sta.w   Camera.yPos
        +

        lda.w   verticalBuffer.cursorIndex
        clc
        adc.w   #METATILE_SIZE * 2
        and.w   #verticalBuffer.buffer.size / 2 - 1
        sta.w   verticalBuffer.cursorIndex

        lda.w   horizontalBuffer.cursorYpos
        clc
        adc.w   #METATILE_SIZE_PX
        sta.w   horizontalBuffer.cursorYpos

        lda.w   visibleMapPtr
        inc
        sta.w   visibleMapPtr
        clc
        adc.w   #METATILE_DISPLAY_HEIGHT

        ldy.w   #METATILE_DISPLAY_HEIGHT * METATILE_SIZE * 2

        bra     DrawHorizontalBuffer

YOffsetNegative:
        cmp.w   #-METATILE_SIZE_PX + 1
        bcs     +
            lda.w   horizontalBuffer.cursorYpos
            sec
            sbc.w   #METATILE_SIZE_PX - 1
            sta.w   Camera.yPos
        +

        lda.w   verticalBuffer.cursorIndex
        sec
        sbc.w   #METATILE_SIZE * 2
        and.w   #verticalBuffer.buffer.size / 2 - 1
        sta.w   verticalBuffer.cursorIndex

        lda.w   horizontalBuffer.cursorYpos
        sec
        sbc.w   #METATILE_SIZE_PX
        sta.w   horizontalBuffer.cursorYpos

        lda.w   visibleMapPtr
        dec
        sta.w   visibleMapPtr

        ldy.w   #0

    DrawHorizontalBuffer:
        // A = map Index, Y = vramOffset
        _DrawHorizontalBuffer()

SkipDrawHorizontalBuffer:


    //  displayHoffset = Camera.xPos - cameraXoffset
    //  displayVoffset = Camera.yPos - cameraVoffset

    lda.w   Camera.xPos
    sec
    sbc.w   cameraXoffset
    sta.w   displayHoffset

    lda.w   Camera.yPos
    sec
    sbc.w   cameraYoffset
    sta.w   displayVoffset

    rts
}



// Draws a map cell to the VBlank buffer (if necessary).
//
// ASSUMES: DrawFullScreen_ForceBlank has been called to setup map state
// ASSUMES: Camera position inside map
// ASSUMES: X is inside the map
//
// NOTE: This macro will always add the MetaTile to the buffer, even if
// `Dma.transfersLeft` does not contain enough time to upload the
// MetaTile.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: X = index of map cell to draw
//        A = value of map cell (lower 8 bits only)
//
// BRANCH: to Return if cell outside visible range
// BRANCH: to RetrunWithWarning if drawMapCellBuffer is full
macro _DrawMapCell() {
    assert16a()
    assert16i()
    // DP unknown

allocateTmpWord(_tmp)

    // Get MetaTile tileMap index
    and.w   #0xff
    asl
    tay
    // MUST NOT MODIFY Y


    // test if cell is left/right of the visible display
    //
    //  offset = cellIndex + map.data
    //  if offset < visibleMapPtr:
    //      return
    //  offset -= visibleMapPtr
    //  if offset > map.HEIGHT * METATILE_DISPLAY_WIDTH + METATILE_DISPLAY_HEIGHT
    //      return
    //  if offset & (map.HEIGHT - 1) > METATILE_DISPLAY_HEIGHT
    //      return
    //
    //  RDDIV,RDMPY = offset / map.HEIGHT

    txa
    clc
    adc.w   #map.data
    sec
    sbc.w   Render.visibleMapPtr
    bcc     Return
    cmp.w   #map.HEIGHT * METATILE_DISPLAY_WIDTH + METATILE_DISPLAY_HEIGHT + 1
    bcs     Return

    sta.l   WRDIV

    // Test if cell is outside the display on the vertical axis
    assertPowerOfTwo(map.HEIGHT)
    and.w   #map.HEIGHT - 1
    cmp.w   #METATILE_DISPLAY_HEIGHT + 1
    bcs     Return

    sep     #$20
a8()
    lda.b   #map.HEIGHT
    sta.l   WRDIVB


    // Get drawMapCellBuffer index and decrement Dma.transfersLeft

    lda.b   #-Dma.CalcDmaTime(Render.DRAW_MAP_CELL_VBLANK_LOOP_CYCLES)
    ldx.w   drawMapCellBufferIndex
    bne     +
        lda.b   #-Dma.CalcDmaTime(Render.DRAW_MAP_CELL_VBLANK_INIT_CYCLES + Render.DRAW_MAP_CELL_VBLANK_LOOP_CYCLES)
    +
    clc
    adc.w   Dma.transfersLeft
    bcs     +
        // Dma.transfersLeft underflowed
        lda.b   #0
    +
    sta.w   Dma.transfersLeft

    // Exit with warning if drawMapCellBuffer is full
    cpx.w   #drawMapCellBuffer.BUFFER_SIZE * 2
    bcs     ReturnWithWarning


    // calculate Vram Word Address of map Cell
    //
    //  tmp = (RDDIV * METATILE_SIZE * 2 + horizontalBuffer.cursorIndex) / 2
    //  tmp &= (32 * 2 - 1)
    //  if tmp & 32:
    //      tmp += 32 * 32 - 32
    //
    //  a = RDMPY * METATILE_SIZE * 2 + verticalBuffer.cursorIndex
    //  a *= 32 / METATILE_SIZE
    //  a &= (32 - 1) * 32
    //
    //  vramWaddr = tmp + a + METATILES_MAP_WADDR

    assert(METATILE_SIZE == 2)

    rep     #$30
a16()
    lda.l   RDDIV
    asl
    asl
    // c clear
    adc.w   horizontalBuffer.cursorIndex
    lsr
    and.w   #32 * 2 - 1
    bit.w   #32
    beq     +
        clc
        adc.w   #32 * 32 - 32
    +
    sta.w   _tmp

    lda.l   RDMPY
    asl
    asl
    // c clear
    adc.w   verticalBuffer.cursorIndex
    and.w   #((32 - 1) * 32) >> 4
    asl
    asl
    asl
    asl
    // c clear
    adc.w   _tmp
    clc
    adc.w   #METATILES_MAP_WADDR


    // Fill drawMapCellBuffer
    //
    // a = vramWaddr
    // y = tileMap index
    // x = drawMapCellBufferIndex
    sta.w   drawMapCellBuffer.vramWaddr,x

    lda.w   tileMap.topLeft,y
    sta.w   drawMapCellBuffer.topLeft,x
    lda.w   tileMap.topRight,y
    sta.w   drawMapCellBuffer.topRight,x
    lda.w   tileMap.bottomLeft,y
    sta.w   drawMapCellBuffer.bottomLeft,x
    lda.w   tileMap.bottomRight,y
    sta.w   drawMapCellBuffer.bottomRight,x

    inx
    inx
    stx.w   drawMapCellBufferIndex
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

