.PHONY: test

CFLAGS=-I ../src -Wall -g -DSENTINEL

test: test-rbtree
	./test-rbtree
	valgrind --leak-check=full --show-leak-kinds=all ./test-rbtree

test-rbtree: test-rbtree.o ../src/rbtree.o

../src/rbtree.o:
	$(MAKE) -C ../src rbtree.o

clean:
	rm -f test-rbtree *.o
