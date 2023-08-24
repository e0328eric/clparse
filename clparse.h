//
// Copyright (C) 2021-2023  Sungbae Jeong
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
//
// Drapeau Command line parser library v0.4.0
//
// It is a command line parser inspired by go's flag module and tsodings flag.h
// ( tsodings flag.h source code : https://github.com/tsoding/flag.h )
//
//
// Changelog
// v0.1.0:                    First release
// v0.2.0:                    Add essential arguments(it is called in here as
//                            main argument)
// v0.2.1:                    Fix a crucial memory bug
//
// v0.3.0:                    Supports a long flag and a short flag
// v0.4.0:                    Supports multiple arguments for flags and main
// arguments

#ifndef CLPARSE_LIBRARY_H_
#define CLPARSE_LIBRARY_H_

// TODO: Test this library in C++
// clang-format off
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CLPARSEDEF
#ifdef CLPARSE_STATIC
#define CLPARSEDEF static
#else
#define CLPARSEDEF extern
#endif // CLAP_STATIC
#endif // CLPARSEDEF

#ifdef __cplusplus
#include <cstdbool>
#include <cstdint>
#include <cstdio>
#else
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#endif // __cpluplus

/* NOTE: Every flag, subcommand names and descriptions must have static
 * lifetimes
 */
// Macros
//
// * CLPARSE_IMPLEMENTATION
// This library follows stb header only library format. That macro makes implementation
// of functions include on the one of the source file.
//
// * NOT_ALLOW_EMPTY_ARGUMENT
// If this macro turns on, then clparse disallows the empty argument and emit an error
#define NO_SHORT 0
#define NO_LONG ""
#define NO_SUBCMD NULL
// * NO_SHORT
// Default value of the short flag name
// * NO_LONG
// Default value of the long flag name
// * NO_SUBCMD
// Default value of the subcmd specifier of the flag declare functions

// Data Structure
typedef enum
{
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

typedef struct
{
	void* items;
	ArrayListKind kind;
	size_t len;
} ArrayList;

// Function Signatures
CLPARSEDEF void clparseStart(const char* name, const char* desc);
CLPARSEDEF bool clparseParse(int argc, char** argv);
CLPARSEDEF void clparseClose(void);
CLPARSEDEF const char* clparseGetErr(void);
CLPARSEDEF bool clparseIsHelp(void);
CLPARSEDEF void clparsePrintHelp(void);

CLPARSEDEF bool* clparseSubcmd(const char* subcmd_name, const char* desc);

CLPARSEDEF const ArrayList* clparseMainArg(const char* name, const char* desc, const char* subcmd);

#define CLAP_TYPES(T)                                                     \
	T(Bool, bool, boolean, FLAG_TYPE_BOOL, ARRAY_LIST_BOOL)                  \
	T(I8, int8_t, i8, FLAG_TYPE_I8, ARRAY_LIST_I8)                           \
	T(I16, int16_t, i16, FLAG_TYPE_I16, ARRAY_LIST_I16)                      \
	T(I32, int32_t, i32, FLAG_TYPE_I32, ARRAY_LIST_I32)                      \
	T(I64, int64_t, i64, FLAG_TYPE_I64, ARRAY_LIST_I64)                      \
	T(U8, uint8_t, u8, FLAG_TYPE_U8, ARRAY_LIST_U8)                          \
	T(U16, uint16_t, u16, FLAG_TYPE_U16, ARRAY_LIST_U16)                     \
	T(U32, uint32_t, u32, FLAG_TYPE_U32, ARRAY_LIST_U32)                     \
	T(U64, uint64_t, u64, FLAG_TYPE_U64, ARRAY_LIST_U64)                     \
	T(Str, const char*, str, FLAG_TYPE_STRING, ARRAY_LIST_STRING)
    // clang-format on

#define T(_name, _type, _foo1, _foo2, _foo3)                                   \
    CLPARSEDEF _type* clparse##_name(const char* flag_name, char short_name,   \
                                     _type dfault, const char* desc,           \
                                     const char* subcmd);
    CLAP_TYPES(T)
#undef T

#define T(_name, _type, _foo1, _foo2, _foo3)                                   \
    CLPARSEDEF const ArrayList* clparse##_name##List(                          \
        const char* flag_name, char short_name, const char* desc,              \
        const char* subcmd);
    CLAP_TYPES(T)
#undef T

#ifdef __cplusplus
}
#endif
#endif // CLPARSE_LIBRARY_H_

/************************/
/* START IMPLEMENTATION */
/************************/
#ifdef CLPARSE_IMPLEMENTATION

#ifdef __cplusplus
#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#else
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#endif // __cplusplus

// A Flag and a Subcmd struct definitions
typedef enum
{
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

typedef union
{
    bool boolean;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    const char* str;
    ArrayList lst;
} FlagKind;

typedef struct
{
    const char* name;
    char short_name;
    FlagType type;
    FlagKind kind;
    FlagKind dfault;
    const char* desc;
} Flag;

#ifndef FLAG_CAPACITY
#define FLAG_CAPACITY 256
#endif // FLAG_CAPACITY

typedef struct
{
    const char* name;
    ArrayList value;
    const char* desc;
} MainArg;

#ifndef MAIN_ARGS_CAPACITY
#define MAIN_ARGS_CAPACITY 16
#endif // MAIN_ARGS_CAPACITY

typedef struct Subcmd
{
    const char* name;
    const char* desc;
    bool is_activate;
    MainArg main_arg;
    Flag flags[FLAG_CAPACITY];
    size_t flags_len;
} Subcmd;

#ifndef SUBCOMMAND_CAPACITY
#define SUBCOMMAND_CAPACITY 64
#endif // SUBCOMMAND_CAPACITY

// static members
static const char* main_prog_name = NULL;
static const char* main_prog_desc = NULL;
static Subcmd* activated_subcmd = NULL;

static Subcmd subcommands[SUBCOMMAND_CAPACITY];
static size_t subcommands_len = 0;

static MainArg main_main_arg;

static Flag main_flags[FLAG_CAPACITY];
static size_t main_flags_len = 0;

static bool* help_cmd[SUBCOMMAND_CAPACITY + 1];
static size_t help_cmd_len = 0;

// Error kinds
typedef enum ClapErrKind
{
    CLAP_ERR_KIND_OK = 0,
    CLAP_ERR_KIND_SUBCOMMAND_FIND,
    CLAP_ERR_KIND_FLAG_FIND,
    CLAP_ERR_KIND_MAIN_ARG_NUM_OVERFLOWED,
    CLAP_ERR_KIND_INAVLID_NUMBER,
    CLAP_ERR_KIND_LONG_FLAG_WITH_SHORT_FLAG,
    CLAP_INTERNAL_ERROR,
} ClapErrKind;

static ClapErrKind clparse_err = (ClapErrKind)0;
static char internal_err_msg[201];
static const char* err_msg_detail = NULL;

// A container of subcommand names (with a hashmap)
// This hashmap made of fnv1a hash algorithm
#define CLAP_HASHMAP_CAPACITY 1024

typedef struct HashBox
{
    const char* name;
    size_t where;
    struct HashBox* next;
} HashBox;

static HashBox hash_map[CLAP_HASHMAP_CAPACITY];

/******************************/
/* Static Function Signatures */
/******************************/
static bool isTruthy(const char* string);
static void deinitFlag(Flag* flag);
static size_t clparseHash(const char* letter);
static MainArg* clparseGetMainArg(const char* subcmd);
static Flag* clparseGetFlag(const char* subcmd);
static bool findSubcmdPosition(size_t* output, const char* subcmd_name);
static void freeNextHashBox(HashBox* hashbox);

/************************************/
/* Implementation of Main Functions */
/************************************/
void clparseStart(const char* name, const char* desc)
{
    main_prog_name = name;
    main_prog_desc = desc;
    memset(hash_map, 0, sizeof(HashBox) * CLAP_HASHMAP_CAPACITY);
    memset(subcommands, 0, sizeof(Subcmd) * SUBCOMMAND_CAPACITY);

    help_cmd[help_cmd_len++] =
        clparseBool("help", NO_SHORT, false, "Print this help message", NULL);
}

void clparseClose(void)
{
    for (size_t i = 0; i < CLAP_HASHMAP_CAPACITY; ++i)
    {
        freeNextHashBox(&hash_map[i]);
    }

    free(main_main_arg.value.items);
    for (size_t i = 0; i < main_flags_len; ++i)
    {
        deinitFlag(&main_flags[i]);
    }

    Subcmd* subcmd;
    for (size_t i = 0; i < subcommands_len; ++i)
    {
        subcmd = &subcommands[i];
        free(subcmd->main_arg.value.items);
        for (size_t j = 0; j < subcmd->flags_len; ++j)
        {
            deinitFlag(&subcmd->flags[j]);
        }
    }
}

bool clparseIsHelp(void)
{
    bool output = false;

    for (size_t i = 0; i < help_cmd_len; ++i)
    {
        output |= *help_cmd[i];
    }

    return output;
}

void clparsePrintHelp(void)
{
    size_t tmp;
    size_t name_len = 0;
    const char* main_arg_name;

    if (main_prog_name == NULL)
    {
        main_prog_name = "(*.*)";
    }

    if (main_prog_desc != NULL)
    {
        fprintf(stderr, "%s\n\n", main_prog_desc);
    }

    if (activated_subcmd != NULL)
    {
        fprintf(stderr, "Usage: %s %s [ARGS] [FLAGS]\n\n", main_prog_name,
                activated_subcmd->name);

        main_arg_name = activated_subcmd->main_arg.name;
        if (main_arg_name != NULL)
        {
            name_len = strlen(main_arg_name);
            fprintf(stderr, "Args:\n");
            fprintf(stderr, "     %*s%s\n", -(int)name_len - 4,
                    activated_subcmd->main_arg.name,
                    activated_subcmd->main_arg.desc);
        }

        fprintf(stderr, "Options:\n");
        for (size_t i = 0; i < activated_subcmd->flags_len; ++i)
        {
            tmp = strlen(activated_subcmd->flags[i].name);
            name_len = name_len > tmp ? name_len : tmp;
        }
        for (size_t i = 0; i < activated_subcmd->flags_len; ++i)
        {
            // TODO: make a case that has both long and short flags
            if (strcmp(activated_subcmd->flags[i].name, NO_LONG) != 0)
            {
                fprintf(stderr, "    --%*s%s\n", -(int)name_len - 4,
                        activated_subcmd->flags[i].name,
                        activated_subcmd->flags[i].desc);
            }
            else
            {
                fprintf(stderr, "    -%*c%s\n", -(int)name_len - 4,
                        activated_subcmd->flags[i].short_name,
                        activated_subcmd->flags[i].desc);
            }
        }
    }
    else
    {
        if (subcommands_len > 0)
        {
            fprintf(stderr, "Usage: %s [SUBCOMMANDS] [ARGS] [FLAGS]\n\n",
                    main_prog_name);
        }
        else
        {
            fprintf(stderr, "Usage: %s [ARGS] [FLAGS]\n\n", main_prog_name);
        }

        main_arg_name = main_main_arg.name;
        if (main_arg_name != NULL)
        {
            name_len = strlen(main_arg_name);
            fprintf(stderr, "Args:\n");
            fprintf(stderr, "     %*s%s\n", -(int)name_len - 4,
                    main_main_arg.name, main_main_arg.desc);
        }

        fprintf(stderr, "Options:\n");
        for (size_t i = 0; i < main_flags_len; ++i)
        {
            tmp = strlen(main_flags[i].name);
            name_len = name_len > tmp ? name_len : tmp;
        }
        for (size_t i = 0; i < main_flags_len; ++i)
        {
            // TODO: make a case that has both long and short flags
            if (strcmp(main_flags[i].name, NO_LONG) != 0)
            {
                fprintf(stderr, "    --%*s%s\n", -(int)name_len - 4,
                        main_flags[i].name, main_flags[i].desc);
            }
            else
            {
                fprintf(stderr, "    -%*c%s\n", -(int)name_len - 4,
                        main_flags[i].short_name, main_flags[i].desc);
            }
        }

        if (subcommands_len > 0)
        {
            fprintf(stderr, "\nSubcommands:\n");

            for (size_t i = 0; i < subcommands_len; ++i)
            {
                tmp = strlen(subcommands[i].name);
                name_len = name_len > tmp ? name_len : tmp;
            }
            for (size_t i = 0; i < subcommands_len; ++i)
            {
                fprintf(stderr, "    %*s%s\n", -(int)name_len - 4,
                        subcommands[i].name, subcommands[i].desc);
            }
        }
    }
}

// Helper macros to implement clparseParse
#define IMPL_PARSE_INTEGER(_field, _type)                                      \
    do                                                                         \
    {                                                                          \
        flag->kind._field = (_type)strtoull(argv[arg++], NULL, 0);             \
        if (errno == EINVAL || errno == ERANGE)                                \
        {                                                                      \
            clparse_err = CLAP_ERR_KIND_INAVLID_NUMBER;                        \
            return false;                                                      \
        }                                                                      \
    } while (false)

#define IMPL_PARSE_INTEGER_LIST(_type)                                         \
    do                                                                         \
    {                                                                          \
        size_t prev_lst_len = flag->kind.lst.len;                              \
        size_t lst_len = 0;                                                    \
        size_t init_arg = arg;                                                 \
                                                                               \
        while (argv[arg + lst_len] != NULL &&                                  \
               (argv[arg + lst_len][0] != '-' ||                               \
                isdigit(argv[arg + lst_len][1])))                              \
        {                                                                      \
            ++lst_len;                                                         \
        }                                                                      \
                                                                               \
        if (flag->kind.lst.len == 0)                                           \
        {                                                                      \
            flag->kind.lst.items = malloc(sizeof(_type) * lst_len);            \
            flag->kind.lst.len = lst_len;                                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            flag->kind.lst.items =                                             \
                realloc(flag->kind.lst.items,                                  \
                        sizeof(_type) * (prev_lst_len + lst_len));             \
            flag->kind.lst.len += lst_len;                                     \
        }                                                                      \
                                                                               \
        for (size_t i = prev_lst_len; i < flag->kind.lst.len; ++i)             \
        {                                                                      \
            ((_type*)flag->kind.lst.items)[i] =                                \
                (_type)strtoull(argv[arg++], NULL, 0);                         \
            if (errno == EINVAL || errno == ERANGE)                            \
            {                                                                  \
                clparse_err = CLAP_ERR_KIND_INAVLID_NUMBER;                    \
                free(flag->kind.lst.items);                                    \
                return false;                                                  \
            }                                                                  \
        }                                                                      \
        assert((init_arg + lst_len == (size_t)arg) &&                          \
               "argument parsing failed");                                     \
    } while (false)

bool clparseParse(int argc, char** argv)
{
    MainArg* main_arg;
    Flag* flags;
    size_t flags_len;
    Flag* flag;
    int arg = 1;

    if (argc < 2)
    {
#ifdef NOT_ALLOW_EMPTY_ARGUMENT
        clparsePrintHelp();
        return false;
#else
        return true;
#endif
    }

    // check whether has a subcommand
    if (subcommands_len > 0 && argv[arg][0] != '-')
    {
        size_t pos;
        if (!findSubcmdPosition(&pos, argv[arg++]))
        {
            clparse_err = CLAP_ERR_KIND_SUBCOMMAND_FIND;
            return false;
        }
        activated_subcmd = &subcommands[pos];
        activated_subcmd->is_activate = true;
        main_arg = &activated_subcmd->main_arg;
        flags = activated_subcmd->flags;
        flags_len = activated_subcmd->flags_len;
    }
    else
    {
        main_arg = &main_main_arg;
        flags = main_flags;
        flags_len = main_flags_len;
    }

    while (arg < argc)
    {
        if (strcmp(argv[arg], "--") == 0)
        {
            ++arg;
            continue;
        }

        size_t j = 0;
        if (argv[arg][0] != '-')
        {
            if (main_arg->value.len >= MAIN_ARGS_CAPACITY)
            {
                clparse_err = CLAP_ERR_KIND_MAIN_ARG_NUM_OVERFLOWED;
                return false;
            }

            ((const char**)main_arg->value.items)[main_arg->value.len++] =
                argv[arg];
            ++arg;
        }
        else
        {
            if (argv[arg][1] == '-')
            {
                if (flags[j].name == NULL)
                {
                    clparse_err = CLAP_ERR_KIND_FLAG_FIND;
                    return false;
                }
                while (j < flags_len &&
                       strcmp(&argv[arg][2], flags[j].name) != 0)
                {
                    ++j;
                }
            }
            else
            {
                if (strlen(&argv[arg][1]) > 1)
                {
                    clparse_err = CLAP_ERR_KIND_LONG_FLAG_WITH_SHORT_FLAG;
                    return false;
                }
                while (j < flags_len && argv[arg][1] != flags[j].short_name)
                {
                    ++j;
                }
            }

            if (j >= flags_len)
            {
                clparse_err = CLAP_ERR_KIND_FLAG_FIND;
                return false;
            }

            flag = &flags[j];
            ++arg;

            switch (flag->type)
            {
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
                switch (flag->kind.lst.kind)
                {
                case ARRAY_LIST_BOOL:
                {
                    size_t prev_lst_len = flag->kind.lst.len;
                    size_t lst_len = 0;
                    size_t init_arg = arg;

                    while (argv[arg + lst_len] != NULL &&
                           argv[arg + lst_len][0] != '-')
                    {
                        ++lst_len;
                    }

                    if (flag->kind.lst.len == 0)
                    {
                        flag->kind.lst.items = malloc(sizeof(bool) * lst_len);
                        flag->kind.lst.len = lst_len;
                    }
                    else
                    {
                        flag->kind.lst.items =
                            realloc(flag->kind.lst.items,
                                    sizeof(bool) * (prev_lst_len + lst_len));
                        flag->kind.lst.len += lst_len;
                    }

                    for (size_t i = prev_lst_len; i < flag->kind.lst.len; ++i)
                    {
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

                case ARRAY_LIST_STRING:
                {
                    size_t prev_lst_len = flag->kind.lst.len;
                    size_t lst_len = 0;
                    size_t init_arg = arg;

                    while (argv[arg + lst_len] != NULL &&
                           argv[arg + lst_len][0] != '-')
                    {
                        ++lst_len;
                    }

                    if (flag->kind.lst.len == 0)
                    {
                        flag->kind.lst.items =
                            malloc(sizeof(const char*) * lst_len);
                        flag->kind.lst.len = lst_len;
                    }
                    else
                    {
                        flag->kind.lst.items = realloc(
                            flag->kind.lst.items,
                            sizeof(const char*) * (prev_lst_len + lst_len));
                        flag->kind.lst.len += lst_len;
                    }

                    for (size_t i = prev_lst_len; i < flag->kind.lst.len; ++i)
                    {
                        ((const char**)flag->kind.lst.items)[i] = argv[arg++];
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
    }

    return true;
}
#undef IMPL_PARSE_INTEGER
#undef IMPL_PARSE_INTEGER_LIST

bool* clparseSubcmd(const char* subcmd_name, const char* desc)
{
    assert(subcommands_len < SUBCOMMAND_CAPACITY);

    HashBox* hash_box;
    Subcmd* subcmd;
    size_t hash = clparseHash(subcmd_name);

    if (hash_map[hash].next != NULL)
    {
        hash_box = hash_map[hash].next;
        while (hash_box->next != NULL)
        {
            hash_box = hash_box->next;
        }
    }
    else
    {
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

    help_cmd[help_cmd_len++] = clparseBool(
        "help", NO_SHORT, false, "Print this help message", subcmd_name);

    return &subcmd->is_activate;
}

const ArrayList* clparseMainArg(const char* name, const char* desc,
                                const char* subcmd)
{
    MainArg* main_arg = clparseGetMainArg(subcmd);
    if (main_arg == NULL)
    {
        if (err_msg_detail == NULL)
        {
            clparse_err = CLAP_ERR_KIND_SUBCOMMAND_FIND;
        }
        return NULL;
    }

    main_arg->name = name;
    main_arg->value.kind = ARRAY_LIST_STRING;
    main_arg->value.len = 0;
    main_arg->value.items = malloc(sizeof(const char*) * MAIN_ARGS_CAPACITY);
    main_arg->desc = desc;

    return &main_arg->value;
}

#define T(_name, _type, _arg, _flag_type, _foo)                                \
    _type* clparse##_name(const char* flag_name, char short_name,              \
                          _type dfault, const char* desc, const char* subcmd)  \
    {                                                                          \
        Flag* flag = clparseGetFlag(subcmd);                                   \
        if (flag == NULL)                                                      \
        {                                                                      \
            if (err_msg_detail == NULL)                                        \
            {                                                                  \
                clparse_err = CLAP_ERR_KIND_SUBCOMMAND_FIND;                   \
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
CLAP_TYPES(T)
#undef T

#define T(_name, _type, _foo1, _foo2, _array_list_type)                        \
    const ArrayList* clparse##_name##List(const char* flag_name,               \
                                          char short_name, const char* desc,   \
                                          const char* subcmd)                  \
    {                                                                          \
        Flag* flag = clparseGetFlag(subcmd);                                   \
        if (flag == NULL)                                                      \
        {                                                                      \
            if (err_msg_detail == NULL)                                        \
            {                                                                  \
                clparse_err = CLAP_ERR_KIND_SUBCOMMAND_FIND;                   \
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
CLAP_TYPES(T)
#undef T

// TODO: implement better and clean error printing message
const char* clparseGetErr(void)
{
    switch (clparse_err)
    {
    case CLAP_ERR_KIND_OK:
        return NULL;

    case CLAP_ERR_KIND_SUBCOMMAND_FIND:
        return "Cannot find an appropriate subcommnads";

    case CLAP_ERR_KIND_FLAG_FIND:
        return "Cannot find an appropriate flag";

    case CLAP_ERR_KIND_MAIN_ARG_NUM_OVERFLOWED:
        return "Too many main arguments are given";

    case CLAP_ERR_KIND_INAVLID_NUMBER:
        return "Invalid number or overflowed number is given";

    case CLAP_ERR_KIND_LONG_FLAG_WITH_SHORT_FLAG:
        return "Long flags must start with `--`, not `-`";

    case CLAP_INTERNAL_ERROR:
        snprintf(internal_err_msg, 200, "Internal error was found at %s",
                 err_msg_detail);
        internal_err_msg[200] = '\0';
        return internal_err_msg;

    default:
        assert(false && "Unreatchable (clparseGetErr)");
        return "";
    }
}

/************************************/
/* Static Functions Implementations */
/************************************/
static MainArg* clparseGetMainArg(const char* subcmd)
{
    MainArg* main_arg;

    if (subcmd != NULL)
    {
        size_t pos;
        if (!findSubcmdPosition(&pos, subcmd))
        {
            return NULL;
        }

        main_arg = &subcommands[pos].main_arg;
    }
    else
    {
        main_arg = &main_main_arg;
    }

    return main_arg;
}

static void deinitFlag(Flag* flag)
{
    if (flag->type == FLAG_TYPE_LIST)
    {
        free(flag->kind.lst.items);
        flag->kind.lst.items = NULL;
        flag->kind.lst.len = 0;
    }
}

static Flag* clparseGetFlag(const char* subcmd)
{
    Flag* flag;

    if (subcmd != NO_SUBCMD)
    {
        size_t pos;
        if (!findSubcmdPosition(&pos, subcmd))
        {
            return NULL;
        }

        size_t* idx = &subcommands[pos].flags_len;
        assert(*idx < FLAG_CAPACITY);

        flag = &subcommands[pos].flags[(*idx)++];
    }
    else
    {
        assert(main_flags_len < FLAG_CAPACITY);
        flag = &main_flags[main_flags_len++];
    }

    return flag;
}

static size_t clparseHash(const char* letter)
{
    uint32_t hash = 0x811c9dc5;
    const uint32_t prime = 16777619;
    while (*letter)
    {
        hash = ((uint32_t)*letter++ ^ hash) * prime;
    }
    return hash ^ (hash >> 10) << 10;
}

static bool findSubcmdPosition(size_t* output, const char* subcmd_name)
{
    size_t hash = clparseHash(subcmd_name);
    HashBox* hashbox = &hash_map[hash];

    if (hashbox->name == NULL)
    {
        return false;
    }

    while (hashbox != NULL && strcmp(subcmd_name, hashbox->name) != 0)
    {
        hashbox = hashbox->next;
    }

    if (hashbox == NULL)
    {
        return false;
    }

    *output = hashbox->where;
    return true;
}

static void freeNextHashBox(HashBox* hashbox)
{
    if (hashbox == NULL)
    {
        return;
    }

    HashBox* next = hashbox->next;
    while (next != NULL)
    {
        hashbox = next;
        next = next->next;
        free(hashbox);
    }
}

static bool isTruthy(const char* string)
{
    if (string == NULL)
    {
        return false;
    }

    switch (string[0])
    {
    case 't':
    case 'T':
        if (string[1] == 0)
        {
            return true;
        }
        return strcmp(string + 1, "rue") == 0;

    default:
        return false;
    }
}

#endif // CLPARSE_IMPLEMENTATION
