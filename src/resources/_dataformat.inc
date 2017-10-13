// resourcess/_dataformat.inc
// ==========================
//
// Data format of the resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

// The Resources subsystem requires the following to be defined:
//
//   Resources.PaletteList
//      a list of long pointers that point to Palette data format
//
//   Resources.PaletteList.count
//      the number of elements in the PaletteList
//

namespace Format {
    // Palette data format
    //
    //   Header
    //   PaletteData: 2 * (nColors + 1) bytes
    namespace PaletteHeader {
        struct()
            // number of colors to transfer - 1
            // (uint8)
            field(nColors, 1)
        endstruct()
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

