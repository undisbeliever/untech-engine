// common/functiontable.inc
// ========================
//
// Macros to build a function table from a given struct.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

