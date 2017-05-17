// math/_variables.inc
// ===================
//
// Variables used by the Math module.


scope Math {

allocate(product32, shadow, 4)
allocate(factor32, shadow, 4)

allocate(dividend32, dp, 4)
allocate(divisor32, dp, 4)
allocate(remainder32, dp, 4)
constant result32(dividend32)

allocate(tmp1, shadow, 2)
allocate(tmp2, shadow, 2)
allocate(tmp3, shadow, 2)

}

// vim: ft=bass-65816 ts=4 sw=4 et:

