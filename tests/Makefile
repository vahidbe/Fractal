test3 : test3.o
	$(MAKE) -C ./../libfractal/
	$(MAKE) -C ./../
	gcc -o test3 test3.o ../FractalMaker.o ../libfractal/libfractal.a -lpthread -lSDL -lcunit

test3.o : test3.c ./../FractalMaker.h
	gcc -c -I./../ test3.c ./../FractalMaker.h -lpthread -lSDL -lcunit
clean :
	rm test3 test3.o test test.o ./../libfractal/fractal.o ./../libfractal/tools.o ./../libfractal/libfractal.a ./../main ./../main.o
