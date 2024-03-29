// math/_variables.inc
// ===================
//
// Variables used by the Math module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Math {

allocate(product32, lowram, 4)
allocate(factor32, lowram, 4)

allocate(dividend32, dp, 4)
allocate(divisor32, dp, 4)
allocate(remainder32, dp, 4)
constant result32 = dividend32

allocate(tmp1, lowram, 2)
allocate(tmp2, lowram, 2)
allocate(tmp3, lowram, 2)

}

// vim: ft=bass-65816 ts=4 sw=4 et:

