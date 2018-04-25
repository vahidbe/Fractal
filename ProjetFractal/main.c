#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fractal.h"

int main(int argc, char *argv[])
{
	struct fractal* f1 = fractal_new(argv[1],*argv[2]-'0',*argv[3]-'0',*argv[4]-'0',*argv[5]-'0');
	printf("%s",fractal_get_name(f1));
	fractal_free(f1);
}
