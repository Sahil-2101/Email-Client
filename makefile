EXE=fetchmail
$(EXE): main.c main.h list.c list.h retrieve.c retrieve.h parse.c parse.h mime.c mime.h
	cc -Wall -o $(EXE) $^



clean:
	rm -f *.o $(EXE)

format:
	clang-format -style=file -i *.c
