#include "../index.h"
#include "../util/linklist.h"
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

IndexBase<uint32_t>*	sipIndex;
IndexBase<uint32_t>*	dipIndex;
IndexBase<uint16_t>*	sportIndex;
IndexBase<uint16_t>*	dportIndex;
IndexBase<uint8_t>*		protoIndex;

const char *delimeter = " ";

struct node {
	uint32_t sip, dip;
	uint16_t sport,dport;
	uint8_t proto;

	void read( char * s) {
		if( s == NULL || *s == 0 ) return ;
		char *p = strtok( s," " );
		sip = 0;
		for( int i = 0 ; i < 4; ++i ){
			sip = (sip<<8) + atoi( p );
			p = strtok( NULL, " ");
		}
		dip = 0;
		for( int i = 0 ; i < 4; ++i ){
			dip = (dip<<8) + atoi( p );
			p = strtok( NULL, " ");
		}	
		sport = atoi( p ); p = strtok(NULL," ");
		dport = atoi( p ); p = strtok(NULL," ");
		proto = atoi( p );
	}
};

const int N = 21000005;
node t;

int main(int argc,char **argv) {

	if( argc < 2 ) {
		printf("usage: ./testIndex recordfile\n");
		return 0;
	}
	FILE *fp = fopen( argv[1], "r");

	if( !fp ) printf("cannot open file\n");

		
	time_t	begin;
	struct timeval tbeg, tend;
#ifdef LINKNODE_POOL
	LinkList<uint32_t>::initLinkNodePool(N*2);
#endif
	/*
	Trie<uint32_t>::initPool( 400000 );
	Trie<uint16_t>::initPool( 100 );
	*/
	begin = time(NULL);
	gettimeofday( &tbeg, NULL );
	sipIndex = new IndexBase<uint32_t>();
	dipIndex = new IndexBase<uint32_t>();
	
	char s[128];
	int  c = 0 ;
	while( fgets( s, 128, fp) ) {
		t.read(s);
		sipIndex->addItem( t.sip, c );
		dipIndex->addItem( t.dip, c );
		c++;
	}
	printf("%d flow records were read \n", c );
	fclose( fp );

	sipIndex->write2file("sip.idx");
	dipIndex->write2file("dip.idx");
	
	gettimeofday( &tend, NULL );
	printf("time = %lf\n",( tend.tv_sec-tbeg.tv_sec ) + (tend.tv_usec-tbeg.tv_usec)/1e6 );
	printf("time = %d\n", time(NULL) - begin );
	
	delete sipIndex;
	delete dipIndex;
	gettimeofday( &tend, NULL );
	printf("time = %lf\n",( tend.tv_sec-tbeg.tv_sec ) + (tend.tv_usec-tbeg.tv_usec)/1e6 );
	printf("time = %d\n", time(NULL) - begin );
#ifdef LINKNODE_POOL
	LinkList<uint32_t>::clearPool();
#endif
	//sleep(30);
	return 0;
}
