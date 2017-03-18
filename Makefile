all: cachehog testbench

cachehog: cachehog.c cachehog_lib.h

testbench: testbench.c cachehog_lib.h
