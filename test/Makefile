CXX = gcc

.PHONY: all clean 

all: test_ll
	./check_linked_list

test_ll: ../src/linked_list.c ../src/linked_list.h check_linked_list.c
	$(CXX) check_linked_list.c ../lib/liblinked_list.a -lcheck -o check_linked_list

clean:
	rm -rf check_linked_list
