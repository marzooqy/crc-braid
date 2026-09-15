#include "crc.h"

//Static function declarations.
static uint64_t reflect(uint64_t x, uint8_t w);
static uint64_t swap(uint64_t x);
static uint64_t crc_initial(params_t *params, uint64_t crc);
static uint64_t crc_final(params_t *params, uint64_t crc);
static uint64_t crc_zeros(params_t *params, uint64_t crc, uint64_t len);
static uint64_t crc_bytes(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len);
static void crc_build_table(params_t *params);
static void crc_build_braid_table(params_t *params);

//Reflect an integer x of width w.
static uint64_t reflect(uint64_t x, uint8_t w) {
    x = ((x >> 32) & 0xffffffff) | ((x << 32) & 0xffffffff00000000);
    x = ((x >> 16) & 0xffff0000ffff) | ((x << 16) & 0xffff0000ffff0000);
    x = ((x >> 8) & 0xff00ff00ff00ff) | ((x << 8) & 0xff00ff00ff00ff00);
    x = ((x >> 4) & 0xf0f0f0f0f0f0f0f) | ((x << 4) & 0xf0f0f0f0f0f0f0f0);
    x = ((x >> 2) & 0x3333333333333333) | ((x << 2) & 0xcccccccccccccccc);
    x = ((x >> 1) & 0x5555555555555555) | ((x << 1) & 0xaaaaaaaaaaaaaaaa);
    return x >> (64 - w);
}

//Swap the bytes of an integer.
static uint64_t swap(uint64_t x) {
    return (x & 0xff00000000000000) >> 56 | (x & 0xff) << 56
         | (x & 0xff000000000000) >> 40 | (x & 0xff00) << 40
         | (x & 0xff0000000000) >> 24 | (x & 0xff0000) << 24
         | (x & 0xff00000000) >> 8 | (x & 0xff000000) << 8;
}

//Create and initialize a params_t struct.
params_t crc_params(uint8_t width, uint64_t poly, uint64_t init, bool refin, bool refout, uint64_t xorout) {
    params_t params;

    params.width = width;
    params.poly = refin ? reflect(poly, width) : poly << (64 - width);
    params.refin = refin;
    params.refout = refout;
    params.init = (refout ? reflect(init, width) : init) ^ xorout;
    params.xorout = xorout;

    crc_build_table(&params);
    crc_build_braid_table(&params);

    return params;
}

//Build the crc table.
static void crc_build_table(params_t *params) {
    for(uint16_t i = 0; i < 256; i++) {
        uint64_t crc = i;

        if(params->refin) {
            for(uint8_t j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (params->poly & -(crc & 1));
                params->crc_table[i] = crc;
            }
        } else {
            crc <<= 56;
            for(uint8_t j = 0; j < 8; j++) {
                crc = (crc << 1) ^ (params->poly & -(crc >> 63));
                params->crc_table[i] = swap(crc);
            }
        }
    }
}

//Build the braid table.
//The table adds N * 8 + (7 - w) zeros to the input byte.
static void crc_build_braid_table(params_t *params) {
    for(uint16_t i = 0; i < 256; i++) {
        uint64_t crc = i;

        if(!params->refin) {
            crc = swap(crc << 56);
        }

        crc = crc_zeros(params, crc, N * 8 - 8);

        for(uint8_t w = 0; w < 8; w++) {
            crc = params->braid_table[8 - w - 1][i] = crc_zeros(params, crc, 1);
        }
    }
}

//Add len zeros to the input.
static uint64_t crc_zeros(params_t *params, uint64_t crc, uint64_t len) {
    while(len--) {
        crc = (crc >> 8) ^ params->crc_table[crc & 0xff];
    }
    return crc;
}

//Compute the CRC of the input buffer byte-by-byte.
static uint64_t crc_bytes(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len) {
    while(len--) {
        crc = (crc >> 8) ^ params->crc_table[(crc ^ *buf++) & 0xff];
    }
    return crc;
}

//Align the CRC.
static uint64_t crc_initial(params_t *params, uint64_t crc) {
    crc ^= params->xorout;
    if(params->refin ^ params->refout) {
        crc = reflect(crc, params->width);
    }
    if(!params->refin) {
        crc = swap(crc << 64 - params->width);
    }
    return crc;
}

//Return the CRC to its original alignment.
static uint64_t crc_final(params_t *params, uint64_t crc) {
    if(!params->refin) {
        crc = swap(crc) >> (64 - params->width);
    }
    if(params->refin ^ params->refout) {
        crc = reflect(crc, params->width);
    }
    return crc ^ params->xorout;
}

//Calculate the CRC using the crc table.
uint64_t crc_table(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len) {
    crc = crc_initial(params, crc);
    crc = crc_bytes(params, crc, buf, len);
    return crc_final(params, crc);
}

//Calculate the CRC using the braid table.
uint64_t crc_braid(params_t *params, uint64_t crc, unsigned char const *buf, uint64_t len) {
    crc = crc_initial(params, crc);

    //Align the buffer to a word boundary.
    uint64_t off = (uintptr_t)buf & 0x7;
    if(off) {
        uint64_t rem = 8 - off;
        crc = crc_bytes(params, crc, buf, rem);
        buf += rem;
        len -= rem;
    }

    if(len >= N * 8) {
        uint64_t *ptr = (uint64_t*)buf;
        uint64_t blks = len / (N * 8);
        uint64_t words[N];

        uint64_t crcs[N] = {0};
        crcs[0] = crc;

        len -= blks * N * 8;

        //The for loops must be unrolled by the optimizing compiler.
        while(--blks) {
            for(uint64_t n = 0; n < N; n++) {
                words[n] = crcs[n] ^ ptr[n];

                crcs[n] = 0;
                for(uint64_t w = 0; w < 8; w++) {
                    crcs[n] ^= params->braid_table[w][(words[n] >> (w * 8)) & 0xff];
                }
            }

            ptr += N;
        }

        //Combine the CRCs.
        crc = 0;
        for(uint8_t n = 0; n < N; n++) {
            crc = crc_zeros(params, crc ^ crcs[n] ^ ptr[n], 8);
        }

        buf = (unsigned char const*)(ptr + N);
    }

    crc = crc_bytes(params, crc, buf, len);
    return crc_final(params, crc);
}