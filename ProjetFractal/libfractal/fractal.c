#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
	fractal* f = (fractal*) malloc(sizeof(fractal));
	if(f==NULL){
		return NULL;
	}
	else {
		double** values= (double**) malloc(width*height*sizeof(double));
		if (values==NULL)
			exit(1);
		f->values=values;
		f->name=name;
		f->width=width;
		f->height=height;
		f->a=a;
		f->b=b;
		return f;
	}
}

void fractal_free(struct fractal *f)
{
	free((void*) f);
}

const char *fractal_get_name(const struct fractal *f)
{
	return f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    fractal_compute_value(;
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    /* TODO */
}

int fractal_get_width(const struct fractal *f)
{
    return (f->width);
}

int fractal_get_height(const struct fractal *f)
{
    return (f->height);
}

double fractal_get_a(const struct fractal *f)
{
    return (f->a);
}

double fractal_get_b(const struct fractal *f)
{
    return (f->b);
}
