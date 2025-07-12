SRC := $(wildcard src/lib/*)
INC := include/
TEST_XTC := example/traj/md.xtc

xtc: test/xtc.c ${SRC} ${INC}
	gcc -o $@ -I${INC} $< ${SRC}
	./xtc ${TEST_XTC}