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
        // Two lz4 compressed data blocks, one after each other
        //   * tilemap block
        //      - lz4 compressed block, same format as MetaTiles.tileMap
        //      - copied straight to WRAM
        //   * Animated Tileset block
        //      - see Resources.Format.AnimatedTilesetHeader for data format
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

