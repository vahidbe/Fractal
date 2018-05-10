main : main.o FractalMaker.o
	$(MAKE) -C ./libfractal/
	gcc -o main main.o FractalMaker.o ./libfractal/libfractal.a -lpthread -lSDL
lib : 
	$(MAKE) -C ./libfractal/
main.o : main.c ./libfractal/fractal.h
	gcc -c -I./libfractal main.c -lpthread -lSDL
FractalMaker.o : FractalMaker.c ./libfractal/fractal.h ./FractalMaker.h
	gcc -c -I./libfractal ./FractalMaker.h FractalMaker.c -lpthread -lSDL
clean :
	rm main main.o ./libfractal/fractal.o ./libfractal/tools.o ./libfractal/libfractal.a FractalMaker.h.gch
