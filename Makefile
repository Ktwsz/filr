CXX = gcc

PLATFORM=linux

.PHONY: all clean raylib

ifeq ($(PLATFORM), linux)
CFLAGS = -Wall -Wextra
all: linux test
else
CFLAGS = -Wall -Wextra -D_WINDOWS_IMPL
all: windows
endif

windows: filr view input
	$(CXX) src/main.c -Llib -lfilr -linputs -lview -lraylib -lcstr -lhash_map -llinked_list -o build/filr_explorer -lm -lgdi32 -lwinmm $(CFLAGS)

linux: filr view input
	$(CXX) src/main.c -Llib -lfilr -linputs -lview -lraylib -lcstr -lhash_map -llinked_list -o build/filr_explorer -lm $(CFLAGS)


test: linux
	cd test && make all

filr: src/filr.c src/filr.h src/result.h cstr
	$(CXX) -c src/filr.c src/filr.h $(CFLAGS)
	ar rcs lib/libfilr.a filr.o
	rm filr.o src/filr.h.gch

view: src/view.c src/view.h src/result.h raylib hash_map cstr
	$(CXX) -c src/view.c src/view.h $(CFLAGS)
	ar rcs lib/libview.a view.o
	rm view.o src/view.h.gch

input: src/inputs.c src/inputs.h config/inputs.h raylib linked_list
	$(CXX) -c src/inputs.c src/inputs.h $(CFLAGS)
	ar rcs lib/libinputs.a inputs.o
	rm inputs.o src/inputs.h.gch

raylib:
	 cd raylib/src && make PLATFORM=PLATFORM_DESKTOP
	 mv raylib/src/libraylib.a lib/libraylib.a

cstr: src/cstr.c src/cstr.h
	$(CXX) -c src/cstr.h src/cstr.c $(CFLAGS)
	ar rcs lib/libcstr.a cstr.o
	rm cstr.o src/cstr.h.gch

hash_map: src/hash_map.c src/hash_map.h
	$(CXX) -c src/hash_map.h src/hash_map.c $(CFLAGS)
	ar rcs lib/libhash_map.a hash_map.o
	rm hash_map.o src/hash_map.h.gch

linked_list: src/linked_list.c src/linked_list.h
	$(CXX) -c src/linked_list.h src/linked_list.c $(CFLAGS)
	ar rcs lib/liblinked_list.a linked_list.o
	rm linked_list.o src/linked_list.h.gch


clean:
	rm -rf lib/*.a
	cd test && make clean
