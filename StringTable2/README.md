# StringTable2

This component implements an optimized routine for determining if a string
has any prefix matches within a table of known strings.

It is the second iteration of the functionality, with ../StringTable being the
first.  This version differs from the first version primarily in that it does
not use an array of 16-bit unsigned shorts for capturing string lengths, which
means the main IsPrefixOfStringInTable routine only uses AVX primitives, not
AVX2 primitives.

This component is supported by some unit tests, living at ../StringTable2Test,
and a benchmark utility, ../StringTable2BenchmarkExe.

See also: [Is Prefix Of String In Table?  A Journey Into SIMD String Processing](http://trent.me/is-prefix-of-string-in-table/).
