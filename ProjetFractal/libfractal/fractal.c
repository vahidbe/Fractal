#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
	fractal* f = (fractal*) malloc(sizeof(fractal));
	if(f==NULL){
		return NULL;
	}
	else {
		f->name=name=name;
		f->width=width;
		f->height=height;
		f->a=a;
		f->b=b;
		return f;
	}
}

void fractal_free(struct fractal *f)
{
   
}

const char *fractal_get_name(const struct fractal *f)
{
    /* TODO */
    return NULL;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    /* TODO */
    return 0;
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
    /* TODO */
}

int fractal_get_width(const struct fractal *f)
{
    /* TODO */
    return 0;
}

int fractal_get_height(const struct fractal *f)
{
    /* TODO */
    return 0;
}

double fractal_get_a(const struct fractal *f)
{
    /* TODO */
    return 0;
}

double fractal_get_b(const struct fractal *f)
{
    /* TODO */
    return 0;
}
