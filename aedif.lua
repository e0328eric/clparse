LANGUAGE = "c"
COMPILER = "clang"
STD = 11
OPT_LEVEL = 0
WARNINGS = { "all", "extra", "pedantic", "conversion" }
ERRORS = { "return-type" }
FLAGS = { "-ggdb" }

aedif.compile("drapeauTest", "src/main.c", nil, nil, ".", nil)
