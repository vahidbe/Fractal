#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fractal.h"

struct fractal *fractal_new(char* name, int width, int height, double a, double b)
{
	struct fractal* f = (struct fractal*) malloc(sizeof(struct fractal));
	if(f==NULL){
		exit(-1);
	}
	else {
		double** values= (double**) malloc(width*sizeof(double*));
		if (values==NULL)
			exit(-1);
                int i;
		for(i=0;i<width;i++){
			values[i]= (double*) malloc(height*sizeof(double));
			if (values[i]==NULL)
				exit(-1);
		}
		f->name=(char*) malloc(sizeof(char)*64);
		f->values=values;
		strcpy(f->name,name);
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

char* fractal_get_name(const struct fractal *f)
{
	/**/printf("F - ### Fractale recue par get name : %s, %d, %d, %f, %f ###\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
	/**/fflush(stdout);
  return  (f->name);
}

int fractal_get_value(const struct fractal *f, int x, int y)
{
    return (f->values[x][y]);
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

double fractal_compute_average(const struct fractal* f)
{
	int i;
	int j;
	double res=0.0;
	/**/fflush(stdout);
	for(i=0;i<(fractal_get_width(f));i++){
		for(j=0;j<(fractal_get_height(f));j++){

			res=res+fractal_get_value(f,i,j);

		}
	}
	return (res/((double)((fractal_get_width(f))*(fractal_get_height(f)))));
}
