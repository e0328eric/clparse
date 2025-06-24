/*
Copyright (C) 2021-2025  Sungbae Jeong

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

//////////////////////////////////////////////////////////////////////////////

Clparse Command line parser library v0.5.0

It is a command line parser inspired by go's flag module and tsodings flag.h
( tsodings flag.h source code : https://github.com/tsoding/flag.h )

# Changelog
- v0.1.0:    First Release
- v0.2.0:    Add essential arguments (it is called in here as `main args`)
- v0.2.1:    Fix a crucial memory bug
- v0.3.0:    Supports a long flag and a short flag
- v0.4.0:    Supports multiple arguments for flags and main
- v0.5.0:    Supports windows UTF-16 argvs
*/

#ifndef CLPARSE_LIBRARY_H_
#define CLPARSE_LIBRARY_H_

#ifndef CLPDEF
#   ifdef CLPARSE_STATIC
#       define CLPDEF static
#   else
#       define CLPDEF extern
#   endif // CLAP_STATIC
#endif // CLPDEF

// Windows uses UTF-16 for argv in default
#if defined(_WIN32) && !defined(NO_USE_WIDE_ARGV)
#define USE_WIDE_ARGV
#endif

#ifdef __cplusplus
#   include <cstdbool>
#   include <cstdint>
#   include <cstdio>
#   ifdef USE_WIDE_ARGV
#       include <cwchar>
#   endif // USE_WIDE_ARGV
#else
#   include <stdbool.h>
#   include <stdint.h>
#   include <stdio.h>
#   ifdef USE_WIDE_ARGV
#       include <wchar.h>
#   endif // USE_WIDE_ARGV
#endif // __cpluplus

// Macros
//
// * CLPARSE_IMPLEMENTATION
// This library follows stb header only library format. That macro makes implementation
// of functions include on the one of the source file.
//
// * NOT_ALLOW_EMPTY_ARGUMENT
// If this macro turns on, then clparse disallows the empty argument and emit an error
// * NO_SHORT
// Default value of the short flag name
// * NO_LONG
// Default value of the long flag name
// * NO_SUBCMD
// Default value of the subcmd specifier of the flag declare functions
#define NO_SHORT 0
#ifdef USE_WIDE_ARGV
#   define NO_LONG L""
#else
#   define NO_LONG ""
#endif // USE_WIDE_ARGV
#define NO_SUBCMD NULL

// cstr_t is a type alias for either char* or wchar_t*
#ifdef USE_WIDE_ARGV
typedef wchar_t cchar;
#else
typedef char    cchar;
#endif // USE_WIDE_ARGV

// because of the different cchar* type, following macros are needed
CLPDEF int cprintf_impl_(const cchar* fmt, ...);
#ifdef USE_WIDE_ARGV
#   define cstrlen    wcslen
#   define cstrcmp    wcscmp
#   define cstrtoull  wcstoull
#   define iscdigit   iswdigit
#   define CSTR2(val) L##val
#   define CSTR(val)  CSTR2(val)
#   define cprintf    cprintf_impl_
#   define CCHAR_FMT  "lc"
#   define CSTR_FMT   "ls"
#else
#   define cstrlen    strlen
#   define cstrcmp    strcmp
#   define cstrtoull  strtoull
#   define iscdigit   isdigit
#   define CSTR2(val) val
#   define CSTR(val)  val
#   define cprintf    cprintf_impl_
#   define CCHAR_FMT  "c"
#   define CSTR_FMT   "s"
#endif // USE_WIDE_ARGV

// Data Structure
typedef enum {
    ARRAY_LIST_BOOL,
    ARRAY_LIST_I8,
    ARRAY_LIST_I16,
    ARRAY_LIST_I32,
    ARRAY_LIST_I64,
    ARRAY_LIST_U8,
    ARRAY_LIST_U16,
    ARRAY_LIST_U32,
    ARRAY_LIST_U64,
    ARRAY_LIST_STRING,
} ArrayListKind;

typedef struct {
	void* items;
	ArrayListKind kind;
	size_t len;
} ArrayList;

// Function Signatures
CLPDEF void clparseInit(const cchar* name, const cchar* desc);
CLPDEF bool clparseParse(int argc, cchar** argv);
CLPDEF void clparseDeinit(void);
CLPDEF const char* clparseGetErr(void);
CLPDEF bool clparseIsHelp(void);
CLPDEF void clparsePrintHelp(void);
CLPDEF bool* clparseSubcmd(const cchar* subcmd_name, const cchar* desc);
CLPDEF const ArrayList* clparseMainArg(const cchar* name, const cchar* desc, const cchar* subcmd);

// windows specific feature
#ifdef _WIN32
CLPDEF bool clparseGetCmdlineW(int* argc, LPWSTR** argv);
CLPDEF void clparseFreeCmdlineW(const LPWSTR* argv);
#endif

#define CLPARSE_TYPES(T)                                                       \
	T(Bool, bool,       boolean, FLAG_TYPE_BOOL,   ARRAY_LIST_BOOL)            \
	T(I8,   int8_t,     i8,      FLAG_TYPE_I8,     ARRAY_LIST_I8)              \
	T(I16,  int16_t,    i16,     FLAG_TYPE_I16,    ARRAY_LIST_I16)             \
	T(I32,  int32_t,    i32,     FLAG_TYPE_I32,    ARRAY_LIST_I32)             \
	T(I64,  int64_t,    i64,     FLAG_TYPE_I64,    ARRAY_LIST_I64)             \
	T(U8,   uint8_t,    u8,      FLAG_TYPE_U8,     ARRAY_LIST_U8)              \
	T(U16,  uint16_t,   u16,     FLAG_TYPE_U16,    ARRAY_LIST_U16)             \
	T(U32,  uint32_t,   u32,     FLAG_TYPE_U32,    ARRAY_LIST_U32)             \
	T(U64,  uint64_t,   u64,     FLAG_TYPE_U64,    ARRAY_LIST_U64)             \
	T(Str,  const cchar*, str,     FLAG_TYPE_STRING, ARRAY_LIST_STRING)

#define T(_name, _type, _foo1, _foo2, _foo3)                                   \
    CLPDEF _type* clparse##_name(                                              \
        const cchar* flag_name,                                                \
        cchar short_name,                                                      \
        _type dfault,                                                          \
        const cchar* desc,                                                     \
        const cchar* subcmd);

    CLPARSE_TYPES(T)
#undef T

#define T(_name, _type, _foo1, _foo2, _foo3)                                   \
    CLPDEF const ArrayList* clparse##_name##List(                              \
        const cchar* flag_name,                                                \
        cchar short_name,                                                      \
        _type dfault,                                                          \
        const cchar* desc,                                                     \
        const cchar* subcmd);

    CLPARSE_TYPES(T)
#undef T

#endif // CLPARSE_LIBRARY_H_

/************************/
/* START IMPLEMENTATION */
/************************/
#ifdef CLPARSE_IMPLEMENTATION

#ifdef __cplusplus
#   include <cassert>
#   include <cctype>
#   include <cerrno>
#   include <climits>
#   include <cstdlib>
#   include <cstring>
#else
#   include <assert.h>
#   include <ctype.h>
#   include <errno.h>
#   include <limits.h>
#   include <stdlib.h>
#   include <string.h>
#endif // __cplusplus

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

// A Flag and a Subcmd struct definitions
typedef enum {
    FLAG_TYPE_NIL = 0,
    FLAG_TYPE_BOOL,
    FLAG_TYPE_I8,
    FLAG_TYPE_I16,
    FLAG_TYPE_I32,
    FLAG_TYPE_I64,
    FLAG_TYPE_U8,
    FLAG_TYPE_U16,
    FLAG_TYPE_U32,
    FLAG_TYPE_U64,
    FLAG_TYPE_STRING,
    FLAG_TYPE_LIST,
} FlagType;

typedef union {
    bool boolean;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    const cchar* str;
    ArrayList lst;
} FlagKind;

typedef struct {
    const cchar* name;
    cchar short_name;
    FlagType type;
    FlagKind kind;
    FlagKind dfault;
    const cchar* desc;
} Flag;

#ifndef FLAG_CAPACITY
#define FLAG_CAPACITY 256
#endif // FLAG_CAPACITY

typedef struct {
    const cchar* name;
    ArrayList value;
    const cchar* desc;
} MainArg;

#ifndef MAIN_ARGS_CAPACITY
#define MAIN_ARGS_CAPACITY 16
#endif // MAIN_ARGS_CAPACITY

typedef struct Subcmd {
    const cchar* name;
    const cchar* desc;
    bool is_activate;
    MainArg main_arg;
    Flag flags[FLAG_CAPACITY];
    size_t flags_len;
} Subcmd;

#ifndef SUBCOMMAND_CAPACITY
#define SUBCOMMAND_CAPACITY 64
#endif // SUBCOMMAND_CAPACITY

// static members
static const cchar* main_prog_name = NULL;
static const cchar* main_prog_desc = NULL;
static Subcmd* activated_subcmd = NULL;

static Subcmd subcommands[SUBCOMMAND_CAPACITY];
static size_t subcommands_len = 0;

static MainArg main_main_arg;

static Flag main_flags[FLAG_CAPACITY];
static size_t main_flags_len = 0;

static bool* help_cmd[SUBCOMMAND_CAPACITY + 1];
static size_t help_cmd_len = 0;

// Error kinds
typedef enum ClparseErrKind
{
    CLPARSE_ERR_KIND_OK = 0,
    CLPARSE_ERR_KIND_SUBCOMMAND_FIND,
    CLPARSE_ERR_KIND_FLAG_FIND,
    CLPARSE_ERR_KIND_MAIN_ARG_NUM_OVERFLOWED,
    CLPARSE_ERR_KIND_INAVLID_NUMBER,
    CLPARSE_ERR_KIND_LONG_FLAG_WITH_SHORT_FLAG,
    CLPARSE_INTERNAL_ERROR,
} ClparseErrKind;

static ClparseErrKind clparse_err = CLPARSE_ERR_KIND_OK;
static char internal_err_msg[201];
static const char* err_msg_detail = NULL;

// A container of subcommand names (with a hashmap)
// This hashmap made of fnv1a hash algorithm
#define CLPARSE_HASHMAP_CAPACITY 1024

typedef struct HashBox
{
    const cchar* name;
    size_t where;
    struct HashBox* next;
} HashBox;

static HashBox hash_map[CLPARSE_HASHMAP_CAPACITY];

/******************************/
/* Static Function Signatures */
/******************************/
static bool isTruthy(const cchar* string);
static void deinitFlag(Flag* flag);
static size_t clparseHash(const cchar* letter);
static MainArg* clparseGetMainArg(const cchar* subcmd);
static Flag* clparseGetFlag(const cchar* subcmd);
static bool findSubcmdPosition(size_t* output, const cchar* subcmd_name);
static void freeNextHashBox(HashBox* hashbox);

/************************************/
/* Implementation of Main Functions */
/************************************/
void clparseInit(const cchar* name, const cchar* desc) {
    main_prog_name = name;
    main_prog_desc = desc;
    memset(hash_map, 0, sizeof(HashBox) * CLPARSE_HASHMAP_CAPACITY);
    memset(subcommands, 0, sizeof(Subcmd) * SUBCOMMAND_CAPACITY);

    help_cmd[help_cmd_len++] =
        clparseBool(CSTR("help"), CSTR('h'), false, CSTR("Print this help message"), NULL);
}

void clparseDeinit(void) {
    for (size_t i = 0; i < CLPARSE_HASHMAP_CAPACITY; ++i) {
        freeNextHashBox(&hash_map[i]);
    }

    free(main_main_arg.value.items);
    for (size_t i = 0; i < main_flags_len; ++i) {
        deinitFlag(&main_flags[i]);
    }

    Subcmd* subcmd;
    for (size_t i = 0; i < subcommands_len; ++i) {
        subcmd = &subcommands[i];
        free(subcmd->main_arg.value.items);
        for (size_t j = 0; j < subcmd->flags_len; ++j) {
            deinitFlag(&subcmd->flags[j]);
        }
    }
}

bool clparseIsHelp(void) {
    bool output = false;

    for (size_t i = 0; i < help_cmd_len; ++i) {
        output |= *help_cmd[i];
    }

    return output;
}

void clparsePrintHelp(void) {
    size_t tmp, name_len = 0;
    const cchar* main_arg_name;

    if (!main_prog_name) main_prog_name = CSTR("(*.*)");
    if (main_prog_desc) cprintf(CSTR("%s\n\n"), main_prog_desc);

    if (activated_subcmd) {
        cprintf(CSTR("Usage: %"CSTR_FMT" %"CSTR_FMT" [ARGS] [FLAGS]\n\n"),
            main_prog_name, activated_subcmd->name);

        main_arg_name = activated_subcmd->main_arg.name;
        if (main_arg_name) {
            name_len = cstrlen(main_arg_name);
            cprintf(CSTR("Args:\n"));
            cprintf(CSTR("     %*"CSTR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
                activated_subcmd->main_arg.name,
                activated_subcmd->main_arg.desc);
        }

        cprintf(CSTR("Options:\n"));
        for (size_t i = 0; i < activated_subcmd->flags_len; ++i) {
            tmp = cstrlen(activated_subcmd->flags[i].name);
            name_len = name_len > tmp ? name_len : tmp;
        }
        for (size_t i = 0; i < activated_subcmd->flags_len; ++i) {
            if (cstrcmp(activated_subcmd->flags[i].name, NO_LONG) != 0) {
                cprintf(CSTR("    --%*"CSTR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
                    activated_subcmd->flags[i].name,
                    activated_subcmd->flags[i].desc);
            } else if (activated_subcmd->flags[i].short_name == NO_SHORT) {
                cprintf(CSTR("    -%*"CCHAR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
                    activated_subcmd->flags[i].short_name,
                    activated_subcmd->flags[i].desc);
            } else {
                cprintf(CSTR("    -%*"CCHAR_FMT" --%*"CSTR_FMT"%"CSTR_FMT"\n"),
                    -(int)name_len - 4,
                    activated_subcmd->flags[i].short_name,
                    activated_subcmd->flags[i].name,
                    activated_subcmd->flags[i].desc);
            }
        }
    } else {
        if (subcommands_len > 0) {
            cprintf(CSTR("Usage: %"CSTR_FMT" [SUBCOMMANDS] [ARGS] [FLAGS]\n\n"),
                main_prog_name);
        } else {
            cprintf(CSTR("Usage: %"CSTR_FMT" [ARGS] [FLAGS]\n\n"),
                main_prog_name);
        }
    }

    main_arg_name = main_main_arg.name;
    if (main_arg_name) {
        name_len = cstrlen(main_arg_name);
        cprintf(CSTR("Args:\n"));
        cprintf(CSTR("    %*"CSTR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
            main_main_arg.name, main_main_arg.desc);
    }

    cprintf(CSTR("Options:\n"));
    for (size_t i = 0; i < main_flags_len; ++i) {
        tmp = cstrlen(main_flags[i].name);
        name_len = name_len > tmp ? name_len : tmp;
    }
    for (size_t i = 0; i < main_flags_len; ++i) {
        if (cstrcmp(main_flags[i].name, NO_LONG) != 0) {
            cprintf(CSTR("    --%*"CSTR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
                main_flags[i].name, main_flags[i].desc);
        } else if (main_flags[i].short_name == NO_SHORT) {
            cprintf(CSTR("    -%*"CCHAR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
                main_flags[i].short_name, main_flags[i].desc);
        } else {
            cprintf(CSTR("    -%*"CCHAR_FMT" --%*"CSTR_FMT"%"CSTR_FMT"\n"),
                -(int)name_len - 4,
                main_flags[i].short_name,
                main_flags[i].name,
                main_flags[i].desc);
        }
    }

    if (subcommands_len > 0) {
        cprintf(CSTR("\nSubcommands:\n"));

        for (size_t i = 0; i < subcommands_len; ++i) {
            tmp = cstrlen(subcommands[i].name);
            name_len = name_len > tmp ? name_len : tmp;
        }
        for (size_t i = 0; i < subcommands_len; ++i) {
            cprintf(CSTR("    %*"CSTR_FMT"%"CSTR_FMT"\n"), -(int)name_len - 4,
                subcommands[i].name, subcommands[i].desc);
        }
    }
}

// Helper macros to implement clparseParse
#define IMPL_PARSE_INTEGER(_field, _type)                                      \
    do {                                                                       \
        flag->kind._field = (_type)cstrtoull(argv[arg++], NULL, 0);            \
        if (errno == EINVAL || errno == ERANGE) {                              \
            clparse_err = CLPARSE_ERR_KIND_INAVLID_NUMBER;                     \
            return false;                                                      \
        }                                                                      \
    } while (0)

#define IMPL_PARSE_INTEGER_LIST(_type)                                         \
    do {                                                                       \
        size_t prev_lst_len = flag->kind.lst.len;                              \
        size_t lst_len = 0;                                                    \
        size_t init_arg = arg;                                                 \
                                                                               \
        while (argv[arg + lst_len] != NULL &&                                  \
                (argv[arg + lst_len][0] != CSTR('-') ||                        \
                    iscdigit(argv[arg + lst_len][1]))) {                       \
            ++lst_len;                                                         \
        }                                                                      \
                                                                               \
        if (flag->kind.lst.len == 0) {                                         \
            flag->kind.lst.items = malloc(sizeof(_type) * lst_len);            \
            flag->kind.lst.len = lst_len;                                      \
        }                                                                      \
        else {                                                                 \
            flag->kind.lst.items =                                             \
                realloc(flag->kind.lst.items,                                  \
                        sizeof(_type) * (prev_lst_len + lst_len));             \
            flag->kind.lst.len += lst_len;                                     \
        }                                                                      \
                                                                               \
        for (size_t i = prev_lst_len; i < flag->kind.lst.len; ++i) {           \
            ((_type*)flag->kind.lst.items)[i] =                                \
                (_type)cstrtoull(argv[arg++], NULL, 0);                        \
            if (errno == EINVAL || errno == ERANGE) {                          \
                clparse_err = CLPARSE_ERR_KIND_INAVLID_NUMBER;                 \
                free(flag->kind.lst.items);                                    \
                return false;                                                  \
            }                                                                  \
        }                                                                      \
        assert((init_arg + lst_len == (size_t)arg) &&                          \
               "argument parsing failed");                                     \
    } while (0)

bool clparseParse(int argc, cchar** argv) {
    MainArg* main_arg;
    Flag *flags, *flag;
    size_t flags_len;
    int arg = 1;

    if (argc < 2) {
#ifdef NOT_ALLOW_EMPTY_ARGUMENT
        clparsePrintHelp();
        return false;
#else
        return true;
#endif
    }

    // check whether has a subcommand
    if (subcommands_len > 0 && argv[arg][0] != CSTR('-')) {
        size_t pos;
        if (!findSubcmdPosition(&pos, argv[arg++])) {
            clparse_err = CLPARSE_ERR_KIND_SUBCOMMAND_FIND;
            return false;
        }
        activated_subcmd = &subcommands[pos];
        activated_subcmd->is_activate = true;
        main_arg = &activated_subcmd->main_arg;
        flags = activated_subcmd->flags;
        flags_len = activated_subcmd->flags_len;
    } else {
        main_arg = &main_main_arg;
        flags = main_flags;
        flags_len = main_flags_len;
    }
    while (arg < argc) {
        if (cstrcmp(argv[arg], CSTR("--")) == 0) {
            ++arg;
            continue;
        }

        size_t j = 0;
        if (argv[arg][0] != CSTR('-')) {
            if (main_arg->value.len >= MAIN_ARGS_CAPACITY) {
                clparse_err = CLPARSE_ERR_KIND_MAIN_ARG_NUM_OVERFLOWED;
                return false;
            }

            ((const cchar**)main_arg->value.items)[main_arg->value.len++] = argv[arg++];
        } else {
            if (argv[arg][1] == CSTR('-')) {
                if (!flags[j].name) {
                    clparse_err = CLPARSE_ERR_KIND_FLAG_FIND;
                    return false;
                }
                for (; j < flags_len &&
                    cstrcmp(&argv[arg][2], flags[j].name) != 0; ++j);
            } else {
                if (cstrlen(&argv[arg][1]) > 1) {
                    clparse_err = CLPARSE_ERR_KIND_LONG_FLAG_WITH_SHORT_FLAG;
                    return false;
                }
                for (; j < flags_len && argv[arg][1] != flags[j].short_name; ++j);
            }
        }

        if (j >= flags_len) {
            clparse_err = CLPARSE_ERR_KIND_FLAG_FIND;
            return false;
        }

        flag = &flags[j];
        ++arg;

        switch (flag->type) {
            case FLAG_TYPE_BOOL:
                flag->kind.boolean = true;
                break;

            case FLAG_TYPE_I8:
                IMPL_PARSE_INTEGER(i8, int8_t);
                break;

            case FLAG_TYPE_I16:
                IMPL_PARSE_INTEGER(i16, int16_t);
                break;

            case FLAG_TYPE_I32:
                IMPL_PARSE_INTEGER(i32, int32_t);
                break;

            case FLAG_TYPE_I64:
                IMPL_PARSE_INTEGER(i64, int64_t);
                break;

            case FLAG_TYPE_U8:
                IMPL_PARSE_INTEGER(u8, uint8_t);
                break;

            case FLAG_TYPE_U16:
                IMPL_PARSE_INTEGER(u16, uint16_t);
                break;

            case FLAG_TYPE_U32:
                IMPL_PARSE_INTEGER(u32, uint32_t);
                break;

            case FLAG_TYPE_U64:
                IMPL_PARSE_INTEGER(u64, uint64_t);
                break;

            case FLAG_TYPE_STRING:
                flag->kind.str = argv[arg++];
                break;

            case FLAG_TYPE_LIST:
                switch (flag->kind.lst.kind) {
                case ARRAY_LIST_BOOL: {
                    size_t prev_lst_len = flag->kind.lst.len;
                    size_t lst_len = 0;
                    size_t init_arg = arg;

                    while (argv[arg + lst_len] != NULL &&
                           argv[arg + lst_len][0] != CSTR('-')) {
                        ++lst_len;
                    }

                    if (flag->kind.lst.len == 0) {
                        flag->kind.lst.items = malloc(sizeof(bool) * lst_len);
                        flag->kind.lst.len = lst_len;
                    }
                    else {
                        flag->kind.lst.items =
                            realloc(flag->kind.lst.items,
                                    sizeof(bool) * (prev_lst_len + lst_len));
                        flag->kind.lst.len += lst_len;
                    }

                    for (size_t i = prev_lst_len; i < flag->kind.lst.len; ++i) {
                        ((bool*)flag->kind.lst.items)[i] =
                            isTruthy(argv[arg++]);
                    }
                    assert((init_arg + lst_len == (size_t)arg) &&
                           "argument parsing failed");
                }
                break;

                case ARRAY_LIST_I8:
                    IMPL_PARSE_INTEGER_LIST(int8_t);
                    break;

                case ARRAY_LIST_I16:
                    IMPL_PARSE_INTEGER_LIST(int16_t);
                    break;

                case ARRAY_LIST_I32:
                    IMPL_PARSE_INTEGER_LIST(int32_t);
                    break;

                case ARRAY_LIST_I64:
                    IMPL_PARSE_INTEGER_LIST(int64_t);
                    break;

                case ARRAY_LIST_U8:
                    IMPL_PARSE_INTEGER_LIST(uint8_t);
                    break;

                case ARRAY_LIST_U16:
                    IMPL_PARSE_INTEGER_LIST(uint16_t);
                    break;

                case ARRAY_LIST_U32:
                    IMPL_PARSE_INTEGER_LIST(uint32_t);
                    break;

                case ARRAY_LIST_U64:
                    IMPL_PARSE_INTEGER_LIST(uint64_t);
                    break;

                case ARRAY_LIST_STRING: {
                    size_t prev_lst_len = flag->kind.lst.len;
                    size_t lst_len = 0;
                    size_t init_arg = arg;

                    while (argv[arg + lst_len] != NULL &&
                           argv[arg + lst_len][0] != CSTR('-')) {
                        ++lst_len;
                    }

                    if (flag->kind.lst.len == 0) {
                        flag->kind.lst.items =
                            malloc(sizeof(const cchar*) * lst_len);
                        flag->kind.lst.len = lst_len;
                    }
                    else {
                        flag->kind.lst.items = realloc(
                            flag->kind.lst.items,
                            sizeof(const cchar*) * (prev_lst_len + lst_len));
                        flag->kind.lst.len += lst_len;
                    }

                    for (size_t i = prev_lst_len; i < flag->kind.lst.len; ++i) {
                        ((const cchar**)flag->kind.lst.items)[i] = argv[arg++];
                    }
                    assert((init_arg + lst_len == (size_t)arg) &&
                           "argument parsing failed");
                }
                break;
            }
            break;

            default:
                assert(false && "Unreatchable(clparseParse)");
                return false;
        }
    }

    return true;
}

#undef IMPL_PARSE_INTEGER
#undef IMPL_PARSE_INTEGER_LIST

bool* clparseSubcmd(const cchar* subcmd_name, const cchar* desc) {
    assert(subcommands_len < SUBCOMMAND_CAPACITY);

    HashBox* hash_box;
    Subcmd* subcmd;
    size_t hash = clparseHash(subcmd_name);

    if (hash_map[hash].next) {
        hash_box = hash_map[hash].next;
        while (hash_box->next) hash_box = hash_box->next;
    } else {
        hash_box = &hash_map[hash];
    }

    hash_box->name = subcmd_name;
    hash_box->where = subcommands_len;
    hash_box->next = (HashBox*)malloc(sizeof(HashBox));
    hash_box->next->next = NULL;

    subcmd = &subcommands[subcommands_len++];

    subcmd->name = subcmd_name;
    subcmd->desc = desc;
    subcmd->is_activate = false;
    subcmd->main_arg.value.kind = ARRAY_LIST_STRING;
    subcmd->main_arg.value.len = 0;
    subcmd->main_arg.value.items = NULL;
    subcmd->flags_len = 0;

    help_cmd[help_cmd_len++] =
        clparseBool(CSTR("help"), CSTR('h'), false,
            CSTR("Print this help message"), subcmd_name);

    return &subcmd->is_activate;
}

const ArrayList* clparseMainArg(
    const cchar* name,
    const cchar* desc,
    const cchar* subcmd
) {
    MainArg* main_arg = clparseGetMainArg(subcmd);
    if (!main_arg) {
        if (!err_msg_detail) clparse_err = CLPARSE_ERR_KIND_SUBCOMMAND_FIND;
        return NULL;
    }

    main_arg->name = name;
    main_arg->value.kind = ARRAY_LIST_STRING;
    main_arg->value.len = 0;
    main_arg->value.items = malloc(sizeof(const cchar*) * MAIN_ARGS_CAPACITY);
    main_arg->desc = desc;

    return &main_arg->value;
}

#define T(_name, _type, _arg, _flag_type, _foo)                                \
    _type* clparse##_name(                                                     \
        const cchar* flag_name,                                                \
        cchar short_name,                                                      \
        _type dfault,                                                          \
        const cchar* desc,                                                     \
        const cchar* subcmd                                                    \
    ) {                                                                        \
        Flag* flag = clparseGetFlag(subcmd);                                   \
        if (!flag) {                                                           \
            if (!err_msg_detail) {                                             \
                clparse_err = CLPARSE_ERR_KIND_SUBCOMMAND_FIND;                \
            }                                                                  \
            return NULL;                                                       \
        }                                                                      \
                                                                               \
        flag->name = flag_name;                                                \
        flag->short_name = short_name;                                         \
        flag->type = _flag_type;                                               \
        flag->kind._arg = dfault;                                              \
        flag->dfault._arg = dfault;                                            \
        flag->desc = desc;                                                     \
                                                                               \
        return &flag->kind._arg;                                               \
    }

// implementation of clparseBool kinds
CLPARSE_TYPES(T)
#undef T

#define T(_name, _type, _foo1, _foo2, _array_list_type)                        \
    const ArrayList* clparse##_name##List(                                     \
        const cchar* flag_name,                                                \
        cchar short_name,                                                      \
        _type dfault,                                                          \
        const cchar* desc,                                                     \
        const cchar* subcmd                                                    \
    ) {                                                                        \
        (void)dfault;                                                          \
        Flag* flag = clparseGetFlag(subcmd);                                   \
        if (!flag) {                                                           \
            if (!err_msg_detail) {                                             \
                clparse_err = CLPARSE_ERR_KIND_SUBCOMMAND_FIND;                \
            }                                                                  \
            return NULL;                                                       \
        }                                                                      \
                                                                               \
        flag->name = flag_name;                                                \
        flag->short_name = short_name;                                         \
        flag->type = FLAG_TYPE_LIST;                                           \
        flag->kind.lst.items = NULL;                                           \
        flag->kind.lst.kind = _array_list_type;                                \
        flag->kind.lst.len = 0;                                                \
        flag->desc = desc;                                                     \
                                                                               \
        return &flag->kind.lst;                                                \
    }

// implementation of clparseBool kinds
CLPARSE_TYPES(T)
#undef T

// TODO: implement better and clean error printing message
const char* clparseGetErr(void) {
    switch (clparse_err) {
    case CLPARSE_ERR_KIND_OK:
        return NULL;

    case CLPARSE_ERR_KIND_SUBCOMMAND_FIND:
        return "Cannot find an appropriate subcommnads";

    case CLPARSE_ERR_KIND_FLAG_FIND:
        return "Cannot find an appropriate flag";

    case CLPARSE_ERR_KIND_MAIN_ARG_NUM_OVERFLOWED:
        return "Too many main arguments are given";

    case CLPARSE_ERR_KIND_INAVLID_NUMBER:
        return "Invalid number or overflowed number is given";

    case CLPARSE_ERR_KIND_LONG_FLAG_WITH_SHORT_FLAG:
        return "Long flags must start with `--`, not `-`";

    case CLPARSE_INTERNAL_ERROR:
        snprintf(internal_err_msg, 200, "Internal error was found at %s",
                 err_msg_detail);
        internal_err_msg[200] = '\0';
        return internal_err_msg;

    default:
        assert(false && "Unreatchable (clparseGetErr)");
        return "";
    }
}

#ifdef _WIN32
bool clparseGetCmdlineW(int* argc, LPWSTR** argv) {
    LPWSTR args = GetCommandLineW();
    *argv = CommandLineToArgvW(args, argc);
    return *argv != NULL;
}

void clparseFreeCmdlineW(const LPWSTR* argv) {
    LocalFree((void*)argv);
}
#endif

/************************************/
/* Static Functions Implementations */
/************************************/
static MainArg* clparseGetMainArg(const cchar* subcmd) {
    MainArg* main_arg;

    if (subcmd) {
        size_t pos;
        if (!findSubcmdPosition(&pos, subcmd)) return NULL;
        main_arg = &subcommands[pos].main_arg;
    }
    else {
        main_arg = &main_main_arg;
    }

    return main_arg;
}

static void deinitFlag(Flag* flag) {
    if (flag->type == FLAG_TYPE_LIST) {
        free(flag->kind.lst.items);
        flag->kind.lst.items = NULL;
        flag->kind.lst.len = 0;
    }
}

static Flag* clparseGetFlag(const cchar* subcmd) {
    Flag* flag;

    if (subcmd != NO_SUBCMD) {
        size_t pos;
        if (!findSubcmdPosition(&pos, subcmd)) return NULL;

        size_t* idx = &subcommands[pos].flags_len;
        assert(*idx < FLAG_CAPACITY);

        flag = &subcommands[pos].flags[(*idx)++];
    }
    else {
        assert(main_flags_len < FLAG_CAPACITY);
        flag = &main_flags[main_flags_len++];
    }

    return flag;
}

static size_t clparseHash(const cchar* letter) {
    uint32_t hash = 0x811c9dc5;
    const uint32_t prime = 16777619;
    while (*letter) {
        hash = ((uint32_t)*letter++ ^ hash) * prime;
    }
    return hash ^ (hash >> 10) << 10;
}

static bool findSubcmdPosition(size_t* output, const cchar* subcmd_name) {
    size_t hash = clparseHash(subcmd_name);
    HashBox* hashbox = &hash_map[hash];

    if (!hashbox->name) return false;

    while (hashbox && cstrcmp(subcmd_name, hashbox->name) != 0) {
        hashbox = hashbox->next;
    }

    if (!hashbox) return false;

    *output = hashbox->where;
    return true;
}

static void freeNextHashBox(HashBox* hashbox) {
    if (!hashbox) return;

    HashBox* next = hashbox->next;
    while (next) {
        hashbox = next;
        next = next->next;
        free(hashbox);
    }
}

static bool isTruthy(const cchar* string) {
    if (!string) return false;

    switch (string[0]) {
    case CSTR('t'):
    case CSTR('T'):
        if (!string[1]) return true;
        return cstrcmp(string + 1, CSTR("rue")) == 0;

    default:
        return false;
    }
}

int cprintf_impl_(const cchar* fmt, ...) {
#ifdef USE_WIDE_ARGV
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    int len = vswprintf(NULL, 0, fmt, args);
#else
    int len = vfwprintf(stderr, fmt, args);
#endif // _WIN32
    va_end(args);
#ifndef _WIN32
    return len;
#endif // !_WIN32
    if (len < 0) return -1;

    wchar_t* buf = (wchar_t*)malloc(sizeof(wchar_t) * len);
    va_start(args, fmt);
    len = vswprintf(buf, len + 1, fmt, args);
    va_end(args);
    if (len < 0) return -1;

    DWORD written;
    HANDLE stderr_h = GetStdHandle(STD_ERROR_HANDLE);
    WriteConsoleW(stderr_h, buf, len, &written, NULL);
    free(buf);
    return written;
#else
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#endif // USE_WIDE_ARGV
}

#endif // CLPARSE_IMPLEMENTATION
