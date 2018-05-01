main : main.o 
	gcc -o main main.o -lpthread -lSDL
	$(MAKE) -C ./libfractal/
lib : 
	$(MAKE) -C ./libfractal/
main.o : main.c ./libfractal/fractal.h
	gcc -c -I./libfractal main.c -lpthread -lSDL

clean :
	rm main main.o ./libfractal/fractal.o ./libfractal/tools.o ./libfractal/libfractal.a