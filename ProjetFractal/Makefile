main : main.o 
	cc -o main main.o -lpthread
	$(MAKE) -C ./libfractal/
lib : 
	$(MAKE) -C ./libfractal/
main.o : main.c ./libfractal/fractal.h
	cc -c -I./libfractal main.c

clean :
	rm main main.o ./libfractal/fractal.o ./libfractal/tools.o ./libfractal/libfractal.a
