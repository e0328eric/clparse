LANGUAGE = "c"
COMPILER = "clang"
STD = 11
OPT_LEVEL = 0
WARNINGS = { "all", "extra", "pedantic", "conversion" }
ERRORS = { "return-type" }
FLAGS = { "-ggdb" }

Compile("drapeauTest", "src/main.c", nil, nil, "lib", nil)
