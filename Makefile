all: compile run

OBJS = objs/rrRenderer.o

objs/%.o: src/%.c
	gcc -c -std=c99 -I include -o $@ $<

objs/%.o: %.c
	gcc -c -std=c99 -I include -o $@ $<

compile: objs/example.o $(OBJS)
	ar rcs librr.a $(OBJS)
	gcc -std=c99 -I include $< -L. -lrr -o file.exe

run: file.exe
	./$<

clean:
	rm -rf file.exe objs/* librr.a out.png
