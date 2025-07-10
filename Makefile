SRC = src/lib/*
INC = include/
TEST_XTC = example/traj/md.xtc

xtc: test/xtc.c ${SRC} ${INC}
	gcc -o $@ -I ${INC} $^ -DDEBUG
	./xtc ${TEST_XTC}