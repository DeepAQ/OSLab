/*
 * Copyright (C) 2017 DeepAQ.
 * All Rights Reserved.
 */

#include "call.h"
#include "bottom.h"
#include <stdio.h>

#define Fail(tips) \
        printf(tips); \
        printf("\n");\
        return -1;

#define Success(tips) \
        printf(tips); \
        printf("\n");\
        return 0;

// 测试内存碎片处理及数据正确性，预计运行时间：5s
int test_ljm_1() {
    init();
    count_t mr, mw, dr, dw;
    evaluate(&mr, &mw, &dr, &dw);
    // 申请 32000 * 4KB 空间并写入数据
    v_address va[32000];
    for (int i = 0; i < 32000; i++) {
        m_pid_t pid = (i % 999) + 1;
        if (allocate(va + i, 4096, pid) != 0) {
            Fail("test_ljm_1 failed: unable to allocate enough space");
        }
        for (int j = 0; j < 4096; j += 1) {
            if (write(i % 256, va[i] + j, pid) != 0) {
                Fail("test_ljm_1 failed: unable to fill allocated space");
            }
        }
    }
    // 释放编号为奇数的空间
    for (int i = 1; i < 32000; i += 2) {
        m_pid_t pid = (i % 999) + 1;
        if (free(va[i], pid) != 0) {
            Fail("test_ljm_1 failed: unable to free space");
        }
    }
    // 尝试访问已被释放的空间
    data_unit data;
    if (read(&data, va[1], 2) != -1) {
        Fail("test_ljm_1 failed: invalid space can be read");
    }
    // 申请 64000KB 空间并写入数据
    v_address va2;
    if (allocate(&va2, 64000 * 1024, 1) != 0) {
        Fail("test_ljm_1 failed: unable to allocate big space");
    }
    for (int i = 0; i < 64000 * 1024; i += 1) {
        if (write(i % 256, va2 + i, 1) != 0) {
            Fail("test_ljm_1 failed: unable to fill big space");
        }
    }
    // 验证第一次分配的偶数编号空间能否被正确读取
    for (int i = 0; i < 32000; i += 2) {
        m_pid_t pid = (i % 999) + 1;
        for (int j = 0; j < 4096; j += 1) {
            if (read(&data, va[i] + j, pid) != 0) {
                Fail("test_ljm_1 failed: unable to read from initial space");
            }
            if (data != (i % 256)) {
                Fail("test_ljm_1 failed: the data in initial space is not correct");
            }
        }
    }
    // 验证第二次分配的空间能否被正确读取
    for (int i = 0; i < 64000 * 1024; i += 1) {
        if (read(&data, va2 + i, 1) != 0) {
            Fail("test_ljm_1 failed: unable to read from big space");
        }
        if (data != (i % 256)) {
            Fail("test_ljm_1 failed: the data in big space is not correct");
        }
    }
    // 理论上不应有磁盘读写
    count_t mr2, mw2, dr2, dw2;
    evaluate(&mr2, &mw2, &dr2, &dw2);
    if (dr2 - dr > 0 || dw2 - dw > 0) {
        Fail("test_ljm_1 failed: disk access");
    }
    Success("test_ljm_1 passed, congratulations!");
}

// 测试有磁盘读写时的数据正确性，预计运行时间：3s
int test_ljm_2() {
    init();
    count_t mr, mw, dr, dw;
    evaluate(&mr, &mw, &dr, &dw);
    // 申请 512MB 空间
    v_address va;
    if (allocate(&va, 512 * 1024 * 1024, 233) != 0) {
        Fail("test_ljm_2 failed: unable to allocate bigger space");
    }
    // 随机写入数据
    for (int i = 0; i < 512 * 1024 * 1024; i += 1) {
        if (write(i % 256, va + i, 233) != 0) {
            Fail("test_ljm_2 failed: unable to fill bigger space");
        }
    }
    // 验证数据能否被读取
    data_unit data;
    for (int i = 0; i < 512 * 1024 * 1024; i += 1) {
        if (read(&data, va + i, 233) != 0) {
            Fail("test_ljm_2 failed: unable to read from bigger space");
        }
        if (data != (i % 256)) {
            Fail("test_ljm_2 failed: the data from bigger space is not correct");
        }
    }
    count_t mr2, mw2, dr2, dw2;
    evaluate(&mr2, &mw2, &dr2, &dw2);
    if (dr2 - dr > 20000000000 || dw2 - dw > 40000000000) {
        printf("test_ljm_2 warning: too much disk access\n");
    }
    Success("test_ljm_2 passed, congratulations!");
}

// 一些杂项测试，预计运行时间：1s
int test_ljm_3() {
    init();
    count_t mr, mw, dr, dw;
    evaluate(&mr, &mw, &dr, &dw);
    // 申请 512 * 1MB 空间
    v_address va[512];
    for (int i = 0; i < 512; i++) {
        if (allocate(va + i, 1024 * 1024, 1) != 0) {
            Fail("test_ljm_3 failed: unable to allocate space");
        }
    }
    // 释放其中前段的 1MB，使用错误的 pid
    if (free(va[100], 2) != -1) {
        Fail("test_ljm_3 failed: did not validate pid");
    }
    // 释放其中前段的 1MB，使用正确的 pid
    if (free(va[100], 1) != 0) {
        Fail("test_ljm_3 failed: unable to free");
    }
    // 再申请 1MB，理论上应该分配在内存中
    if (allocate(va + 100, 1024 * 1024, 3) != 0) {
        Fail("test_ljm_3 failed: unable to allocate again");
    }
    // 读写刚刚分配的 1MB 空间，分别使用错误和正确的 pid
    data_unit data;
    if (write(233, va[100] + 512 * 1024, 4) != -1 || write(233, va[100] + 512 * 1024, 3) != 0 ||
            read(&data, va[100] + 512 * 1024, 4) != -1 || read(&data, va[100] + 512 * 1024, 3) != 0) {
        Fail("test_ljm_3 failed: r/w error");
    }
    // 理论上不应该有磁盘读写
    count_t mr2, mw2, dr2, dw2;
    evaluate(&mr2, &mw2, &dr2, &dw2);
    if (dr2 - dr > 0 || dw2 - dw > 0) {
        Fail("test_ljm_3 failed: disk access");
    }
    Success("test_ljm_3 passed, congratulations!");
}
