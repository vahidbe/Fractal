test3 : test3.o
	gcc -o test3 test3.o -lpthread -lSDL -lcunit
	$(MAKE) -C ./../libfractal/
	$(MAKE) -C ./../
test : test.o
	gcc -o test test.o main.o -lpthread -lSDL -lcunit
	$(MAKE) -C ./../libfractal/
	$(MAKE) -C ./../
lib : 
	$(MAKE) -C ./../libfractal/
main :
	$(MAKE) -C ./../
test.o : test.c ./../libfractal/fractal.h ./../main.c
	gcc -c -I./../libfractal -I./../ test.c -lpthread -lSDL -lcunit
test3.o : test3.c ./../libfractal/fractal.h ./../main.c
	gcc -c -I./../libfractal -I./../ test3.c -lpthread -lSDL -lcunit
clean :
	rm test3 test3.o test test.o ./../libfractal/fractal.o ./../libfractal/tools.o ./../libfractal/libfractal.a ./../main ./../main.o
