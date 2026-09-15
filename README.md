This is a generalization of zlib's CRC braiding algorithm that accepts any [CRC parameters](https://reveng.sourceforge.io/crc-catalogue/all.htm).

## Benchmark

| Length | Reflected | Non-Reflected |
| --- | :-: | :-: |
| 100 B | 1.65 | 1.62 |
| 1 KB | 3.65 | 3.62 |
| 10 KB | 4.50 | 4.50 |
| 100 KB | 4.61 | 4.61 |
| 1 MB | 4.63 | 4.48 |
| 10 MB | 4.50 | 4.50 |
| 100 MB | 4.18 | 4.27 |

Tested on a 10th generation Intel i7 processor. Measured in GiB/s.

The number of CRCs computed can be adjusted in the header using the constant `N`. Different values could be more optimal depending on the hardware.

The algorithm is about 50% faster than the slicing algorithm.

## References

1- [Everything we know about CRC but afraid to forget](https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/crcutil/crc-doc.1.0.pdf)

2- [zlib](https://github.com/madler/zlib)