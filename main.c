#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define CLPARSE_IMPLEMENTATION
#include <clparse.h>

int main(void) {
    int argc;
    cchar** argv;
    clparseGetCmdlineW(&argc, &argv);

    clparseInit(L"foo", L"barboo");
    const bool* bar = clparseBool(L"한글", NO_SHORT,
            false, L"unicode character also works", NO_SUBCMD);
    clparseParse(argc, argv);
    clparseDeinit();

    if (*bar) {
        printf("Hello!\n");
    } else {
        printf("not Hello...\n");
    }
}

