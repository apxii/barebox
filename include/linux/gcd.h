#ifndef _GCD_H
#define _GCD_H

#include <linux/compiler.h>

#if !defined(swap)
#define swap(x, y) do { typeof(x) z = x; x = y; y = z; } while (0)
#endif

unsigned long gcd(unsigned long a, unsigned long b) __attribute_const__;

#endif /* _GCD_H */
