
// Temporary - will eventually be generated by a program

include "../../../src/common/registers.inc"

dw  ToPalette(31, 31, 31)
fill 15 * 2

dw  0, ToPalette(20, 15, 15), ToPalette(20, 0, 0), ToPalette(15, 0, 0)
fill 12 * 2

dw  0, ToPalette(15, 20, 15)

