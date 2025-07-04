SRC = src/lib/*
INC = include/
TEST_DCD = test/sort1_par1.dcd

test: test/test.c ${SRC}
	gcc -o $@ -I ${INC} $^ -DDEBUG
	./test ${TEST_DCD}