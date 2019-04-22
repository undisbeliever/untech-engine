// metatiles/_dataformat.inc
// ==========================
//
// MetaTile Data Format.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {

// List of MetaTile Tilesets
// (Long Addr Table)
constant TilesetList = Project.MetaTileTilesetList
constant TilesetList.count = Project.MetaTileTilesetList.count

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

    if MetaTiles.TILESET_FORMAT_VERSION != 3 {
        error "Invalid untech-compiler MetaTile Tileset Format"
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

