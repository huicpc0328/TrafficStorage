#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <string.h>

using namespace std;
const int N = 11111;
int	 a[11];

char *filename;
set<int>iset[15];
const char delimeter[] = " ";

void q1(){
	FILE *fp = fopen( filename, "r" );
	if( !fp ) {
		printf("cannot open file %s\n",filename );
		return ;
	}
	char s[128];
	int  cnt = 0;

	while( fgets( s, 128, fp ) ) {
		if( *s == 0 ) continue;
		char *p = strtok( s, delimeter );
		for( int i = 0 ; i < 11; ++i ) {
			a[i] = atoi( p );
			p = strtok( NULL, delimeter );
		}
		cnt ++;
		uint32_t tmp = 0;
		for( int i = 0 ; i < 4;i++ ) {
			tmp = (tmp<<8) + a[i];
			iset[i].insert( tmp );
		}
		tmp = 0;
		for( int i = 4; i < 8;++i ) {
			tmp = (tmp<<8) + a[i];
			iset[i].insert( tmp );
		}
		iset[8].insert( a[8]>>8 );
		iset[9].insert( a[8] );
		iset[10].insert( a[9]>>8 );
		iset[11].insert( a[9] );

	}
	printf("%d records are read\n", cnt );
	fclose( fp );
	for( int i = 0 ; i < 12; ++i ) {
		printf("size of %d = %d\n", i , iset[i].size());
	}
}


int main( int argc, char **argv ) {
	filename = argv[1];
	q1();
	return 0;
}

