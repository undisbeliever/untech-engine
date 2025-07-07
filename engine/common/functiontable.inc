// common/functiontable.inc
// ========================
//
// Macros to build a function table from a given struct.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


// Automatically builds the function table from a given struct
//
// NOTE: will change the current romblock to code.
//
// PARAM: struct - the struct that contains the function table
// PARAM: module - the struct that the table is called in (solves visibility problem)
// PARAM: tableName - the name of the function table (defualt FunctionTable)
inline buildFunctionTable(struct, module, tableName) {
    code()
    {tableName}: {
        _buildFunctionTable_structs({struct}, {module})
    }
}
inline buildFunctionTable(struct, module) {
    buildFunctionTable({struct}, {module}, FunctionTable)
}

macro _buildFunctionTable_structs(struct, module) {
    validateStructIsBase({struct})

    evaluate n = 0
    while {n} < {{struct}.__STRUCT__.fields.size} {
        assert({{struct}.__STRUCT__.fields.{n}.size} == 2)
        dw  {module}.{{struct}.__STRUCT__.fields.{n}.name}

        evaluate n = {n} + 1
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

