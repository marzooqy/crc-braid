#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "crc.h"

uint64_t ten_pow(int8_t n) {
    uint64_t i = 1;

    while(n-- > 0) {
        i *= 10;
    }

    return i;
}

void bench(uint64_t n, uint64_t len) {
    if(len < ten_pow(3)) {
        printf("| %lli B |", len);
    } else if(len < ten_pow(6)) {
        printf("| %lli KB |", len / ten_pow(3));
    } else {
        printf("| %lli MB |", len / ten_pow(6));
    }

    clock_t start, end;
    float speed;
    params_t params;

    unsigned char* buf = (unsigned char*) malloc(len);

    for(uint64_t i = 0; i < len; i++) {
        buf[i] = i & 0xff;
    }

    //reflected
    params = crc_params(64, 0x42f0e1eba9ea3693, 0xffffffffffffffff, true, true, 0xffffffffffffffff);

    start = clock();

    for(uint64_t i = 0; i < n; i++) {
        crc_braid(&params, params.init, buf, len);
    }

    end = clock();

    speed = (float)(len * n) / ((float)(end - start) / CLOCKS_PER_SEC) / powf(1024, 3);

    printf(" %.2f ", speed);

    //non-Reflected
    params = crc_params(64, 0x42f0e1eba9ea3693, 0xffffffffffffffff, false, false, 0xffffffffffffffff);

    start = clock();

    for(uint64_t i = 0; i < n; i++) {
        crc_braid(&params, params.init, buf, len);
    }

    end = clock();

    speed = (float)(len * n) / ((float)(end - start) / CLOCKS_PER_SEC) / powf(1024, 3);

    printf("| %.2f |\n", speed);

    free(buf);
}

void main() {
    printf("| Length | Reflected | Non-Reflected |\n");
    printf("| --- | :-: | :-: |\n");

    bench(ten_pow(7), ten_pow(2));
    bench(ten_pow(6), ten_pow(3));
    bench(ten_pow(5), ten_pow(4));
    bench(ten_pow(4), ten_pow(5));
    bench(ten_pow(3), ten_pow(6));
    bench(ten_pow(2), ten_pow(7));
    bench(ten_pow(1), ten_pow(8));
}