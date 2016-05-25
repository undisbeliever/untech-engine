// Test cpu.inc

arch snes.cpu

include "../../../src/common/cpu.inc"

a8()
rep($20)
assert8a()

