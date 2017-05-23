import qbs

CppApplication {
    consoleApplication: true
    files: [
        "bottom.c",
        "bottom.h",
        "call.c",
        "call.h",
        "main.c",
        "paging.c",
        "paging.h",
        "test.c",
        "type.h",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
