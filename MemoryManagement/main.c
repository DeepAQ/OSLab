/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "stdio.h"
#include "call.h"

int test1();
int test2();
int test3();
int test4();
int test5();
int test6();
int test7();
int test8();

int my_test1() {
    v_address va[32000];
    init();
    for (int i = 0; i < 32000; i++) {
        allocate(va + i, 4096, 1);
    }
    for (int i = 0; i < 32000; i++) {
        write(i % 256, va[i], 1);
    }
    for (int i = 0; i < 32000; i += 2) {
        free(va[i], 1);
    }
    for (int i = 0; i < 32000 / 2; i += 2) {
        allocate(va + i, 8192, 2);
    }
    for (int i = 0; i < 32000 / 2; i += 2) {
        write(i % 256, va[i], 2);
    }
    for (int i = 1; i < 32000; i += 2) {
        data_unit data;
        read(&data, va[i], 1);
        if (data != (i % 256)) {
            printf("Fail: %d\n", i);
            return -1;
        }
    }
    for (int i = 0; i < 32000 / 2; i += 2) {
        data_unit data;
        read(&data, va[i], 2);
        if (data != (i % 256)) {
            printf("Fail: %d\n", i);
            return -1;
        }
    }
    printf("Passed my_test1\n");
    return 0;
}

int my_test2() {
    v_address va[163072 / 1024];
    init();
    for (int i = 0; i < 163072 / 1024; i++) {
        if (allocate(va + i, 4096 * 1024, 1) == -1) {
            printf("Fail: %d\n", i);
            return -1;
        }
    }
    if (allocate(va, 40960 * 1024, 1) != -1) {
        printf("Fail: second round");
        return -1;
    }
    printf("Passed my_test2\n");
    return 0;
}

int my_test3() {
    init();
    count_t mr1, mw1, dr1, dw1;
    evaluate(&mr1, &mw1, &dr1, &dw1);

    v_address va;
    if (allocate(&va, 131072 * 4096, 1) != 0) {
        printf("Fail: big allocate");
        return -1;
    }

    evaluate(&mr1, &mw1, &dr1, &dw1);
    for (int i = 131071; i > 130000; i--) {
        write(i % 66666, va + i * 4096 + 2048, 1);
    }
    evaluate(&mr1, &mw1, &dr1, &dw1);
    for (int i = 131071; i > 130000; i--) {
        data_unit data;
        read(&data, va + i * 4096 + 2048, 1);
        if (data != (i % 66666) % 256) {
            printf("Fail: read");
            return -1;
        }
    }
    evaluate(&mr1, &mw1, &dr1, &dw1);

    printf("Passed my_test3\n");
    return 0;
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();

    my_test3();
    return 0;
}
