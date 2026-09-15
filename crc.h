#ifndef CRC_BRAID_H
#define CRC_BRAID_H

#include <stdbool.h>
#include <stdint.h>

#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

/* Number of CRCs to be computed in each iteration. */
#define N 3

/* Holds CRC parameters and tables. */
typedef struct {
    uint8_t width;
    uint64_t poly;
    bool refin;
    bool refout;
    uint64_t init;
    uint64_t xorout;
    uint64_t crc_table[256];
    uint64_t braid_table[8][256];
} params_t;

/* Create a params_t struct and initialize it with the provided parameters. */
params_t DLL_EXPORT crc_params(uint8_t width, uint64_t poly, uint64_t init, bool refin, bool refout, uint64_t xorout);

/* Calculate the CRC using the table-based algorithm.
   Use params.init as the initial CRC value.*/
uint64_t DLL_EXPORT crc_table(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len);

/* Calculate the CRC using the braiding algorithm.
   Use params.init as the initial CRC value.*/
uint64_t DLL_EXPORT crc_braid(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len);

#endif
