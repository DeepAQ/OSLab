/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "call.h"

int test1();
int test2();
int test3();
int test4();
int test5();
int test6();
int test7();
int test8();

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();

    v_address va, va2;
    init();
    allocate(&va, 8192, 1);
    printf("%d\n", va);
    allocate(&va2, 8192, 1);
    printf("%d\n", va2);
    free(va, 1);
    allocate(&va, 16384, 1);
    printf("%d\n", va);
    allocate(&va, 1024, 1);
    printf("%d\n", va);
    allocate(&va, 4096, 1);
    printf("%d\n", va);
    allocate(&va, 1024, 1);
    printf("%d\n", va);
    return 0;
}
