#include "../btree.h"
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

const int N = 4000000;
BTree<int32_t,int32_t> iibtree;

void test_read() {
	timeval begin, end;
	gettimeofday( &begin, NULL );
	iibtree.readFromFile( "test.dat" );
	gettimeofday( &end, NULL );
	int gap = (end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec; 
	printf("READ TIME: %lf\n", 1.0*gap/1000000);
}

inline int get() {
	return ( random() ) % 20000000;
}

void test_insert() {
	srand( time(0) );
	int *a = new int[N];
	for( int i = 0; i < N; i++ ) {
		a[i] = get();
	}

	timeval begin, end;
	gettimeofday( &begin, NULL );
	for( int i = 0; i < N; i++ ) {
		iibtree.addItem( a[i] , i );
	}

	for( int i = 2; i < 100; ++i ) {
		iibtree.addItem( 1000000, i );
	}
	gettimeofday( &end, NULL );
	int gap = (end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec; 
	printf("INSERT TIME: %lf\n", 1.0*gap/1000000);
	delete[] a ;
}

void test_write( const char *fileName = "test.dat" ) {
	timeval begin, end;
	gettimeofday( &begin, NULL );
	iibtree.write2file( fileName );
	gettimeofday( &end, NULL );
	int gap = (end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec; 
	printf("WRITE TIME: %lf\n", 1.0*gap/1000000);
}

void test_queryFromFile( const char *fileName = "test.dat") {
	srand( time(0) );
	timeval begin, end;
	gettimeofday( &begin, NULL );
	LinkList<int32_t>* list = NULL;
	list = iibtree.queryFromFile( fileName, 1000000, 1000000 );
	if( list ) list->outInfo();
	delete list;
	for( int i = 0 ; i < 1000000; ++i ) {
		int key = get();
		list = iibtree.queryFromFile( fileName, key, key + 1000);
#if 0
		if( i%10000 == 0 ) {
			printf("query at %d\n",i);
			if(list) list->outInfo();
		}
#endif
		delete list;
	}
	gettimeofday( &end, NULL );
	int gap = (end.tv_sec - begin.tv_sec) * 1000000 + end.tv_usec - begin.tv_usec; 
	printf("QUERY TIME: %lf\n", 1.0*gap/1000000);
}

int main() {
#ifdef WRITE 
	test_insert();
	test_write(); 
	//test_queryFromFile( "test.dat");
#else
	test_read();
	test_write( "test2.dat" );
	test_queryFromFile( "test2.dat");
#endif
	//iibtree.outInfo();
	return 0;
}
