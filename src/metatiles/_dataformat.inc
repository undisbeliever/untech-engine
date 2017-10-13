// metatiles/_dataformat.inc
// ==========================
//
// MetaTile Data Format.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {

// The MetaTiles subsystem requires the following to be defined:
//
//   MetaTiles.TilesetList
//      a list of long pointers that point to MetaTiles.Format.Tileset
//      data.
//
//   MetaTiles.TilesetList.count
//      the number of elements in the TilesetList
//

namespace Format {
    // MetaTile Tileset data format
    namespace Tileset {
        struct()
            // Size of the tiles data block
            // number of bytes to upload to VRAM
            // (uint16)
            field(tilesSize, 2)

            // ::TODO data block compression::

            // Three data blocks, one after each other
            //   * tilemap block
            //      - MetaTiles.N_TILES * 8 bytes in size
            //      - copied straight to WRAM
            //      - same format as MetaTiles.tileMap
            //   * Tiles
            //      - tilesSize bytes in size
            //      - copied straight to VRAM at METATILES_TILES_WADDR
            field(dataBlocks, 1)
        endstruct()
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

