test : test.o 
	gcc -o test test.o -lpthread -lSDL
	$(MAKE) -C ./../libfractal/
lib : 
	$(MAKE) -C ./../libfractal/
test.o : test.c ./../libfractal/fractal.h
	gcc -c -I./../libfractal test.c -lpthread -lSDL -lcunit

clean :
	rm test test.o ./../libfractal/fractal.o ./../libfractal/tools.o ./../libfractal/libfractal.a
