
// inserts a single row of a 2bpp tile into the ROM
macro TileRow(evaluate row) {
    variable t = 0
    variable n = 7
    while n >= 0 {
        variable i = ({row} >> (n * 4)) & 3

        t = t << 1
        t = t | (i & 1) | ((i & 2) << 7)

        n = n - 1
    }
    dw  t
}

macro EndTile() {
    fill 16, 0
}

// Tiles Block
// Tile 0 - blank
    fill 32, 0

// Tile 1
    TileRow(0x00000000)
    TileRow(0x00000000)
    TileRow(0x00000000)
    TileRow(0x00000033)
    TileRow(0x00000322)
    TileRow(0x00003222)
    TileRow(0x00032222)
    TileRow(0x00032223)
    EndTile()

// Tile 2
    TileRow(0x00000000)
    TileRow(0x00000000)
    TileRow(0x00000000)
    TileRow(0x33333333)
    TileRow(0x22222222)
    TileRow(0x22222222)
    TileRow(0x22222222)
    TileRow(0x33333333)
    EndTile()

// Tile 3
    TileRow(0x00032223)
    TileRow(0x00032223)
    TileRow(0x00032223)
    TileRow(0x00032223)
    TileRow(0x00032223)
    TileRow(0x00032223)
    TileRow(0x00032223)
    TileRow(0x00032223)
    EndTile()

// Tile 4
    TileRow(0x11111111)
    TileRow(0x11111111)
    TileRow(0x11111111)
    TileRow(0x11111111)
    TileRow(0x11111111)
    TileRow(0x11111111)
    TileRow(0x11111111)
    TileRow(0x11111111)
    EndTile()


// Animated Tiles

// Frame 0
// Tile 5
    TileRow(0x00001000)
    TileRow(0x00010000)
    TileRow(0x00100000)
    TileRow(0x01110000)
    TileRow(0x10011000)
    TileRow(0x00001100)
    TileRow(0x00000000)
    TileRow(0x00000000)
    EndTile()

// Frame 1
// Tile 5
    TileRow(0x00011110)
    TileRow(0x00010000)
    TileRow(0x00010000)
    TileRow(0x11100000)
    TileRow(0x10000000)
    TileRow(0x10000000)
    TileRow(0x10000000)
    TileRow(0x00000000)
    EndTile()

