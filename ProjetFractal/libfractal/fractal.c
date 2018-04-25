#include <stdlib.h>
#include "fractal.h"

struct fractal *fractal_new(const char *name, int width, int height, double a, double b)
{
	struct fractal* f = (struct fractal*) malloc(sizeof(struct fractal));
	if(f==NULL){
		exit(1);
	}
	else {
		double** values= (double**) malloc(width*sizeof(double*));
		if (values==NULL)
			exit(1);
                int i;
		for(i=0;i<width;i++){
			values[i]= (double*) malloc(height*sizeof(double));
			if (values[i]==NULL)
				exit(1);
		}
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
  return  f->name;
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    return f->values[x][y];
}

void fractal_set_value(struct fractal *f, int x, int y, int val)
{
	f->values[x][y]=val;
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
