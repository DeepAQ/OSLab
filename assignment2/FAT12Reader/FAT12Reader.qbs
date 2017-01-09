import qbs

CppApplication {
    consoleApplication: true
    files: "main.c"

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
