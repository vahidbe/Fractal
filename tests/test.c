#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "./../FractalMaker.h"
#include "CUnit/Basic.h"
#include <CUnit/CUnit.h>

static struct sbuf* sb;
static struct charbuf* cb;
static struct fractal * fract;
char *name;
int height;
int width;
double a;
double b;


int init_suite1(void)
{
   if (NULL == (sb =(struct sbuf *)malloc(sizeof(struct sbuf)))) {
      return -1;
   }
   else if(NULL == (fract=(struct fractal*)malloc(sizeof(struct fractal)))) {
      free(sb);
      return -1;
   }
   else if(NULL == (cb=(struct charbuf*)malloc(sizeof(struct charbuf)))) {
      free(sb);
      free(fract);
      return -1;
   }
   else {
      return 0;
   }
}

int clean_suite1(void)
{
   free(fract);
   sbuf_clean(sb);
   charbuf_clean(cb);
   return 0;
}

void testFRACTALNEW(void){
   if(NULL!=fract){
   name = "nom de la fractale";
   height = 1400;
   width = 1400;
   a = 0.37;
   b = 1.37;
   fract = fractal_new(name,width,height,a,b);
   CU_ASSERT(fract != NULL);
   }
}
void testGETNAME(void)
{
   if(NULL!=fract){
   CU_ASSERT(strcmp(fractal_get_name(fract),name)==0);
   }
}

void testGETWIDTH(void)
{
   if(NULL!=fract){
   CU_ASSERT_EQUAL(fractal_get_width(fract),width);
   }
}

void testGETHEIGHT(void)
{
   if(NULL!=fract){
   CU_ASSERT_EQUAL(fractal_get_height(fract),height);
   }
}

void testGETA(void)
{
   if(NULL!=fract){
   CU_ASSERT_EQUAL(fractal_get_a(fract),a);
   }
}

void testGETB(void)
{
   if(NULL!=fract){
   CU_ASSERT_EQUAL(fractal_get_b(fract),b);
   }
}

void testSBUFINIT(void){
   if(NULL!=sb){
     int* empty;
     int* full;
     sbuf_init(sb,10);
     CU_ASSERT_EQUAL(sb->n,10);
     CU_ASSERT_PTR_NOT_NULL(sb->buf);
     CU_ASSERT_EQUAL(sb->rear,0);
     CU_ASSERT_EQUAL(sb->front,0);
     sem_getvalue(&sb->empty,empty);
     CU_ASSERT_EQUAL(*empty,10);
     sem_getvalue(&sb->full,full);
     CU_ASSERT_EQUAL(*full,0);
     CU_ASSERT(!pthread_mutex_trylock(&(sb->mutex)));
     pthread_mutex_unlock(&(sb->mutex));
   }
}


void testCHARBUFINIT(void){
   if(NULL!=cb){
     charbuf_init(cb,10);
     CU_ASSERT_EQUAL(cb->n,10);
     CU_ASSERT_PTR_NOT_NULL(cb->buf);
     CU_ASSERT_EQUAL(cb->rear,0);
     CU_ASSERT_EQUAL(cb->front,0);
     CU_ASSERT(!pthread_mutex_trylock(&(cb->mutex)));
     pthread_mutex_unlock(&(cb->mutex));
   }
}

void testSBUFINSERT(void)
{
   if (NULL != sb & NULL != fract) {
      sbuf_insert(sb, fract);
      CU_ASSERT_EQUAL(fractal_get_name(fract),fractal_get_name(sb->buf[0]));
      CU_ASSERT_EQUAL(fractal_get_width(fract),fractal_get_width(sb->buf[0]));
      CU_ASSERT_EQUAL(fractal_get_height(fract),fractal_get_height(sb->buf[0]));
      CU_ASSERT_EQUAL(fractal_get_a(fract),fractal_get_a(sb->buf[0]));
      CU_ASSERT_EQUAL(fractal_get_b(fract),fractal_get_b(sb->buf[0]));
      CU_ASSERT_EQUAL(fract,sb->buf[0]);
      CU_ASSERT_EQUAL(sb->rear,1);
      CU_ASSERT_EQUAL(sb->front,0);
   }
}

void testCHARBUFINSERT(void)
{
   if (NULL != cb & NULL != fract) {
      charbuf_insert(cb, fractal_get_name(fract));
      CU_ASSERT(!strcmp(fractal_get_name(fract),cb->buf[0]));
      CU_ASSERT_EQUAL(cb->rear,1);
      CU_ASSERT_EQUAL(cb->front,0);
   }
}

void testSBUFREMOVE(void)
{
   if (NULL != sb & NULL != fract) {
     struct fractal* f;
     int rear=sb->rear;
     f = sbuf_remove(sb);
     CU_ASSERT_EQUAL(f,fract);
     fractal_free(f);
     CU_ASSERT(sb->rear == rear-1);
   }
}


void testALREADYUSED(void)
{
   char* C_TEST = fractal_get_name(fract);
   if (NULL != cb & NULL != fract) {
      CU_ASSERT(1==charbuf_already_used(cb,fractal_get_name(fract)));
      CU_ASSERT(!strcmp(cb->buf[0],fractal_get_name(fract)));
      CU_ASSERT_EQUAL(cb->rear,1);
      CU_ASSERT_EQUAL(cb->front,0);
   }
}

void testCHARBUFCLEAN(void)
{
   if (NULL != cb & NULL != fract) {
      charbuf_clean(cb);
      CU_ASSERT_PTR_NULL(cb->buf);
   }
}


int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test of fractal_new", testFRACTALNEW)) ||
       (NULL == CU_add_test(pSuite, "test of fractal_get_name", testGETNAME)) ||
       (NULL == CU_add_test(pSuite, "test of fractal_get_width", testGETWIDTH)) ||
       (NULL == CU_add_test(pSuite, "test of fractal_get_height", testGETHEIGHT)) ||
       (NULL == CU_add_test(pSuite, "test of fractal_get_a", testGETA)) ||
       (NULL == CU_add_test(pSuite, "test of fractal_get_b", testGETB)) ||
       (NULL == CU_add_test(pSuite, "test of sbuf_init", testSBUFINIT)) ||
       (NULL == CU_add_test(pSuite, "test of charbuf_init", testCHARBUFINIT)) ||
       (NULL == CU_add_test(pSuite, "test of sbuf_insert", testSBUFINSERT)) ||
       (NULL == CU_add_test(pSuite, "test of charbuf_insert", testCHARBUFINSERT)) ||
       (NULL == CU_add_test(pSuite, "test of charbuf_already_used", testALREADYUSED)) ||
       (NULL == CU_add_test(pSuite, "test of sbuf_remove", testSBUFREMOVE)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   //CU_cleanup_registry();
   return CU_get_error();
}


