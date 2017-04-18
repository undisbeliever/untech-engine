
scope Hdma {

// The Hdma.CircularWindow module generates a HDMA table that renders a
// circle using screen window 1.
scope CircularWindow {

// ::TODO move memory used by this module into a shared pool::

constant DELTA_TABLE_SIZE(76)

// Delta table calculated in the DrawCircle.BuildDeltaTable scope
//
// This table contains the left/right incremental delta for each
// scanline in the top 45 degrees of the circle.
allocate(deltaTable, wram7e, DELTA_TABLE_SIZE)

// Current size of the deltaTable
allocateTmpWord(deltaCount)

// Previous radius used by BuildDeltaTable
allocateTmpWord(deltaTableRadius)



// Initializes the module.
//
// The caller is responsible for setting up the Window settings.
//
// REQUIRES: DB = 0x7e
au()
code()
scope Init: {
    // invalidate deltaTable value
    stz.w   deltaTableRadius

    rts
}



// Builds and enables the HdmaTable for a Circular HDMA window.
//
// INPUT: A = radius
//        X = xOrigin
//        Y = yOrigin
//
// ASSUMES: xOrigin >= 0 and xOrigin < 255
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0, unused dpTmp variables
a8()
i16()
code()
scope DrawCircle: {

assert(BUFFER_SIZE >= (255 * 2 + 3) * 3 + 1)

constant _xOrigin(dpTmp0)
constant _yOrigin(dpTmp1)
constant _radius(dpTmp2)

constant _hdmaBuffer(dpTmp3)
constant _hdmaCenterRow(dpTmp4)

constant _linesLeft(dpTmp5)

constant _j(dpTmp6)
constant _d(dpTmp7)


// The HdmaTable is offset by two rows as:
//   1) The first two rows are reserved for the yOffset
//   2) It simplifies accessing the previous row of the table
scope HdmaTable {
    struct()
        field(prev2.count, 1)
        field(prev2.left, 1)
        field(prev2.right, 1)

        field(prev.count, 1)
        field(prev.left, 1)
        field(prev.right, 1)

        field(count, 1)
        field(left, 1)
        field(right, 1)
    endstruct()
}



    rep     #$30
a16()

    and.w   #0xff
    sta.b   _radius

    scope CheckBounds {
        // if radius == 0 or
        //    yOrigin + radius < 0 or
        //    yOrigin - radius > DISPLAY_HEIGHT or
        //    xOrigin < 0 or xOrigin > 255:
        //      hdmaTable = GetBuffer()
        //      hdmaTable->count = 1
        //      hdmaTable->left = 255
        //      hdmaTable->right = 0
        //
        //      hdmaTable[1]->count = 0
        //
        //      goto SetupChannelSettings

        beq     OutOfBounds

        tya
        clc
        adc.b   _radius
        bmi     OutOfBounds

        tya
        sec
        sbc.b   _radius
        sec
        sbc.w   #Camera.DISPLAY_HEIGHT
        bpl     OutOfBounds

        txa
        and.w   #0xff00
        beq     InBounds

        OutOfBounds:
            jsr     GetBuffer

            // set fully blank then stop HDMA
            lda.w   #0xff01
            sta.w   HdmaTable.prev2.count,x
            stz.w   HdmaTable.prev2.count + 2,x

            jmp     SetupChannelSettings

    InBounds:
    }

    stx.b   _xOrigin
    sty.b   _yOrigin

    sep     #$10
i8()

    lda.b   _radius
    cmp.w   deltaTableRadius
    beq     DeltaTableAlreadyExists
    sta.w   deltaTableRadius

    scope BuildDeltaTable {
        // 75 is the size of the delta table when radius = 255
        // +1 if for the 16 bit store.
        assert(DELTA_TABLE_SIZE >= 75 + 1)

        // Builds the delta Table for the given radius
        // Based on the Integer Midpoint Circle Algorithm

        //  i = 0
        //  j = radius
        //  d = 1 - radius
        //
        //  deltaCount = 0
        //  offset = 0
        //
        //  while i < j:
        //      offset++
        //
        //      if d < 0:
        //          d += i * 2 + 3
        //          i++
        //      else:
        //          d += (i - j) * 2 + 5
        //          i++
        //          j--
        //
        //          deltaTable[deltaCount] = offset
        //          deltaCount++
        //          offset = 0
        //
        //  if offset != 0:
        //      deltaTable[deltaCount] = offset
        //      deltaCount++

    assert16a()
    assert8i()

        // x register = i
        // _j = j
        // y register = offset
        // _d = d

        ldx.b   #0

        sta.b   _j

        eor.w   #0xFFFF
        inc
        inc
        sta.b   _d

        stz.w   deltaCount
        ldy.b   #0

        cpx.b   _j
        bcs     EndLoop
        Loop:
            iny

            lda.b   _d
            bpl     PositiveD
                // d < 0
                txa
                asl
                clc
                adc.w   #3

                bra     EndIf

            PositiveD:
                tya
                ldy.w   deltaCount
                sta.w   deltaTable,y
                iny
                sty.w   deltaCount
                ldy.b   #0

                txa
                sec
                sbc.b   _j
                asl
                clc
                adc.w   #5

                dec.b   _j
            EndIf:

            clc
            adc.b   _d
            sta.b   _d

            inx
            cpx.b   _j
            bcc     Loop
        EndLoop:

        cpy.b   #0
        beq     +
            tya
            ldy.w   deltaCount
            sta.w   deltaTable,y
            iny
            sty.w   deltaCount
        +
    }
DeltaTableAlreadyExists:

    sep     #$20
    rep     #$10
a8()
i16()
    jsr     GetBuffer
    stx.b   _hdmaBuffer

    // x = hdmaTable
    // y = deltaIndex

    scope FirstScanline {
        // First scanline of the circle.

        //  hdmaTable->count = 1
        //
        //  l = xOrigin - deltaTable[0] + 1
        //  if (l < 0): l = 0
        //  hdmaTable->left = l
        //
        //  r = xOrigin + deltaTable[0] - 1
        //  if (r > 255): r = 255
        //  hdmaTable->right = r
        //
        //  hdmaTable++     // +3 bytes

        lda.b   #1
        sta.w   HdmaTable.count,x

        lda.b   _xOrigin
        sec
        sbc.w   deltaTable
        bcs     +
            lda.b   #0xff
        +
        inc
        sta.w   HdmaTable.left,x

        lda.b   _xOrigin
        clc
        adc.w   deltaTable
        bcc     +
            lda.b   #0
        +
        dec
        sta.w   HdmaTable.right,x

        inx
        inx
        inx
    }

    //  linesLeft = radius - deltaTable[0]
    //  deltaIndex = 1

    lda.b   _radius
    sec
    sbc.w   deltaTable

    sta.b   _linesLeft
    stz.b   _linesLeft + 1

    ldy.w   #1

    scope First45 {
        // First 45 degrees of the circle.
        // Width of circle increased by delta * 2, height increases by 1
        //
        //  while deltaIndex < deltaCount:
        //      hdmaTable->count = 1
        //
        //      l = hdmaTable[-1]->left - deltaTable[deltaIndex]
        //      if (l < 0): l = 0
        //      hdmaTable->left = l
        //
        //      r = hdmaTable[-1]->right + deltaTable[deltaIndex]
        //      if (r > 255): r = 255
        //      hdmaTable->right = r
        //
        //      hdmaTable++     // +3 bytes
        //
        //      linesLeft -= deltaTable[deltaIndex]
        //      deltaIndex++

        cpy.w   deltaCount
        bcs     SkipLoop

        Loop:
            lda.b   #1
            sta.w   HdmaTable.count,x

            lda.w   HdmaTable.prev.left,x
            sec
            sbc.w   deltaTable,y
            bcs     +
                lda.b   #0
            +
            sta.w   HdmaTable.left,x

            lda.w   HdmaTable.prev.right,x
            clc
            adc.w   deltaTable,y
            bcc     +
                lda.b   #0xff
            +
            sta.w   HdmaTable.right,x

            inx
            inx
            inx

            lda.b   _linesLeft
            sec
            sbc.w   deltaTable,y
            sta.b   _linesLeft

            iny
            cpy.w   deltaCount
            bcc     Loop

    SkipLoop:
    }

    scope Second45 {
        // Second 45 degrees of the circle.
        // Width of circle increased 2, height unknown, calculated later
        //
        //  while linesLeft != 0:
        //
        //      l = hdmaTable[-1]->left
        //      if (l < 0): l = 0
        //      hdmaTable->left = l
        //
        //      r = hdmaTable[-1]->right + 1
        //      if (r > 255): r = 255
        //      hdmaTable->right = r
        //
        //      hdmaTable++     // +3 bytes
        //      linesLeft--

        ldy.b   _linesLeft
        beq     SkipLoop

        Loop:
            lda.w   HdmaTable.prev.left,x
            beq     +
                dec
            +
            sta.w   HdmaTable.left,x

            lda.w   HdmaTable.prev.right,x
            inc
            bne     +
                lda.b   #0xff
            +
            sta.w   HdmaTable.right,x

            inx
            inx
            inx

            dey
            bne     Loop

    SkipLoop:
    }

    //  hdmaCenterRow = hdmaTable
    stx.b   _hdmaCenterRow

    scope Second45Count {
        // Fill in count for second 45 deg of circle.
        //
        // This is preformed in a separate loop as the height delta may
        // extend into the top 45 degrees of the circle.
        //
        //  deltaIndex = deltaCount - 2
        //
        //  for deltaIndex = 1; deltaIndex < deltaCount; deltaIndex++:
        //      hdmaTable--     // -3 bytes
        //      hdmaTable->count = deltaTable[deltaIndex]

        ldy.w   #1
        cpy.w   deltaCount
        bcs     SkipLoop

        Loop:
            dex
            dex
            dex

            lda.w   deltaTable,y
            sta.w   HdmaTable.count,x

            iny
            cpy.w   deltaCount
            bcc     Loop

    SkipLoop:
    }

    scope Center {
        //  hdmaCenterRow->count = deltaTable[0] * 2 - 1
        //  l = hdmaTable[-1]->left
        //  if (l < 0): l = 0
        //  hdmaTable->left = l
        //
        //  r = hdmaTable[-1]->right + 1
        //  if (r > 255): r = 255
        //  hdmaTable->right = r

        ldx.b   _hdmaCenterRow

        lda.w   deltaTable
        asl
        dec
        sta.w   HdmaTable.count,x

        lda.w   HdmaTable.prev.left,x
        beq     +
            dec
        +
        sta.w   HdmaTable.left,x

        lda.w   HdmaTable.prev.right,x
        inc
        bne     +
            lda.b   #0xff
        +
        sta.w   HdmaTable.right,x
    }


    scope CopyBottomHalf {
        //  if yOrigin + radius <= DISPLAY_HEIGHT
        //      topHalf = hdmaCenterRow - 1         // - 3 bytes
        //      bottomHalf = hdmaCenterRow + 1      // + 3 bytes
        //
        //      while topHalf > hdmaBuffer + 1:
        //          bottomHalf = topHalf
        //          topHalf--
        //          bottomHalf++
        //
        //      hdmaTable = bottomHalf
        //
        //  else:
        //      hdmaTable = hdmaCenterRow + 1

        rep     #$30
    a16()

        lda.b   _yOrigin
        bmi     +
        cmp.w   #Camera.DISPLAY_HEIGHT
        bcs     SkipCopy
    +
            // x = bottomHalf
            // y = topHalf

            ldy.b   _hdmaCenterRow
            ldx.b   _hdmaCenterRow
            bra     UpdateIndexes

            Loop:
                // copy 3 bytes
                lda.w   HdmaTable.count,y
                sta.w   HdmaTable.count,x
                lda.w   HdmaTable.left,y
                sta.w   HdmaTable.left,x

                // No early exit test is added to the loop, it is
                // faster to just waste RAM.

            UpdateIndexes:
                inx
                inx
                inx

                dey
                dey
                dey

                cpy.b   _hdmaBuffer
                bcs     Loop

            bra End

    SkipCopy:
        inx
        inx
        inx

    End:
    }

    scope LastScanline {
        assert16a()
        assert16i()
        // X = hdmaTable

        // Last scanline, fully blank then stop HDMA
        lda.w   #0xff01
        sta.w   HdmaTable.count,x
        stz.w   HdmaTable.count + 2,x
    }

    scope ProcessYOffset {
        assert16a()
        assert16i()

        //  offset = yOrigin - radius
        //
        //  if offset >= 0x80:
        //      hdmaTable = hdmaBuffer
        //      hdmaTable->count = 0x7e
        //      hdmaTable->left = 255
        //      hdmaTable->right = 0
        //
        //      hdmaTable[1]->count = offset - 0x80
        //      hdmaTable[1]->left = 255
        //      hdmaTable[1]->right = 0
        //
        //  else if offset > 0:
        //      hdmaTable = hdmaBuffer + 1 // +3 bytes
        //      hdmaTable->count = offset
        //      hdmaTable->left = 0
        //      hdmaTable->right = 255
        //
        //  else if offset == 0:
        //      hdmaTable = hdmaBuffer + 2 // +6 bytes
        //
        //  else:
        //      hdmaTable = hdmaBuffer + 2 // +6 bytes
        //      offset = -offset
        //
        //      while offset >= hdmaBuffer->count:
        //          offset -= hdmaBuffer->count
        //          hdmaBuffer++        // +3 bytes
        //
        //      hdmaBuffer->count = hdmaBuffer->count - offset

        ldx.w   _hdmaBuffer

        lda.b   _yOrigin
        sec
        sbc.b   _radius
        bmi     CircleAboveScanlineZero
        beq     CircleOnScanlineZero

            tay
            lda.w   #0x00ff
            sta.w   HdmaTable.prev.left,x
            sta.w   HdmaTable.prev2.left,x

            sep     #$20
        a8()
            tya
            cmp.b   #0x80
            bcs     +
                sta.w   HdmaTable.prev.count,x
                bra     EndIf_IncrementOneRow
            +

            sec
            sbc.b   #0x7f
            sta.w   HdmaTable.prev.count,x

            lda.b   #0x7f
            sta.w   HdmaTable.prev2.count,x
            bra     EndIf


        CircleAboveScanlineZero:
            eor.w   #0xffff
            inc

            sep     #$20
        a8()

            Loop:
                tay
                sec
                sbc.w   HdmaTable.count,x
                bcs     +
                    // 16 bit subtraction
                    xba
                    sbc.b   #0
                    xba
                    bcc     EndLoop
                +

                inx
                inx
                inx

                bra     Loop

            EndLoop:

            tya
            eor     #0xff
            sec
            adc.w   HdmaTable.count,x
            sta.w   HdmaTable.count,x

            // +6 bytes for the two row offset in HdmaTable

        CircleOnScanlineZero:
            inx
            inx
            inx

        EndIf_IncrementOneRow:
            inx
            inx
            inx

        EndIf:
    }


    scope SetupChannelSettings: {
        sep     #$20
    a8()
        // x = hdmaTable
        stx.w   ChannelSettings.tableAddr


        assert(ChannelSettings.dmap + 1 == ChannelSettings.register)

        ldx.w   #DMAP.direction.toPpu | DMAP.addressing.absolute | DMAP.transfer.two | (WH0 << 8)
        stx.w   ChannelSettings.dmap


        lda.b   #1
        sta.w   ChannelSettings.enabled
    }

    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

