test : test.o
	$(MAKE) -C ./../libfractal/
	$(MAKE) -C ./../
	gcc -o test test.o ../FractalMaker.o ../libfractal/libfractal.a -lpthread -lSDL -lcunit

test3.o : test.c ./../FractalMaker.h
	gcc -c -I./../ test.c ./../FractalMaker.h -lpthread -lSDL -lcunit
clean :
	rm test test.o ./../FractalMaker.o ./../FractalMaker.h.gch ./../libfractal/fractal.o ./../libfractal/tools.o ./../libfractal/libfractal.a ./../main ./../main.o
