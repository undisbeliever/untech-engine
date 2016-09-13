
// Automatically builds the function table from a given struct
//
// NOTE: will change the current romblock to code.
//
// PARAM: struct - the struct that contains the function table
// PARAM: module - the struct that the table is called in (solves visibility problem)
// PARAM: tableName - the name of the function table (defualt FunctionTable)
macro buildFunctionTable(struct, module, tableName) {
    code()
    scope {tableName}: {
        _buildFunctionTable_structs({struct}, {module})
    }
}
macro buildFunctionTable(struct, module) {
    buildFunctionTable({struct}, {module}, FunctionTable)
}

macro _buildFunctionTable_structs(struct, module) {
    if {defined ::struct.bases.{struct}.parent} {
        _buildFunctionTable_structs({::struct.bases.{struct}.parent}, {module})
    }
    _buildFunctionTable_field({struct}, {module}, 0)
}

macro _buildFunctionTable_field(struct, module, evaluate n) {
    if {n} < {::struct.bases.{struct}.size} {
        assert({::struct.bases.{struct}.var{n}size} == 2)
        dw  {module}.{::struct.bases.{struct}.var{n}}

        _buildFunctionTable_field({struct}, {module}, {n} + 1)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

