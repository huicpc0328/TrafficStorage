#include <stdlib.h>
#include <cstdio>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <table.h>
#include "ibis.h"
#include <fcntl.h>

using namespace std;

const int N = 10000005;
char a[N][64];

struct node {
	uint32_t sip, dip;
	uint16_t sport,dport;
	uint8_t proto;

	void read( char * s) {
		if( s == NULL || *s == 0 ) return ;
		char *p = strtok( s,"," );
		sip = 0;
		for( int i = 0 ; i < 4; ++i ){
			sip = (sip<<8) + atoi( p );
			p = strtok( NULL, ",");
		}
		dip = 0;
		for( int i = 0 ; i < 4; ++i ){
			dip = (dip<<8) + atoi( p );
			p = strtok( NULL, ",");
		}	
		sport = atoi( p ); p = strtok(NULL,",");
		dport = atoi( p ); p = strtok(NULL,",");
		proto = atoi( p );
	}
};

int main(int argc,char **argv) {
	
	if( argc < 3 ) {
		printf("usage: ./testIndex recordfile storedDir\n");
		return 0;
	}
	FILE *fp = fopen( argv[1], "r");

	if( !fp ) printf("cannot open file\n");

	int  c = 0 ;
	char s[64];
	node tmp;
	/*
	while( fgets( s, 64, fp) ) {
		tmp.read(s);
		sprintf( a[c++],"%d,%d,%d,%d,%d",tmp.sip,tmp.dip,tmp.sport,tmp.dport,tmp.proto);
	}
	*/

	while( fgets( a[c++], 64, fp ) ) {}
	printf("%d flow records were read \n", c );
	fclose( fp );
	
	time_t	begin;
	struct timeval tbeg, tend;
	gettimeofday( &tbeg, NULL );
	begin = time(NULL);
	ibis::init();
	ibis::tablex* table = ibis::tablex::create();
	if( !table ) {
		printf("error in table creating!\n");
	}
	table->addColumn( "sip1", ibis::UBYTE );
	table->addColumn( "sip2", ibis::UBYTE );
	table->addColumn( "sip3", ibis::UBYTE );
	table->addColumn( "sip4", ibis::UBYTE );
	table->addColumn( "dip1", ibis::UBYTE );
	table->addColumn( "dip2", ibis::UBYTE );
	table->addColumn( "dip3", ibis::UBYTE );
	table->addColumn( "dip4", ibis::UBYTE );
	table->addColumn( "sport",ibis::USHORT );
	table->addColumn( "dport",ibis::USHORT );
	table->addColumn( "proto",ibis::UBYTE );
	/*
	table->addColumn( "sip",ibis::UINT );
	table->addColumn( "dip",ibis::UINT );
	table->addColumn( "sport",ibis::USHORT );
	table->addColumn( "dport",ibis::USHORT );
	table->addColumn( "proto",ibis::UBYTE );
	*/
	for( int i = 0 ; i < c ; ++i ) {
		table->appendRow( a[i] );	
	}

	table->write( argv[2], "flows","");
	table->clearData();
	//table->write( "data", "flows","","index=<binning none/><encoding equality/>" );
	//ibis::table* tb = table->toTable();
	ibis::table* tb = ibis::table::create( argv[2] );
	if( !tb ) {
		printf("error in table construction\n");
	}
	tb->buildIndexes("index=<binning none/><encoding equality/>");
	/*
	tb->buildIndex("sip1","<binning none/><encoding equality/>");
	tb->buildIndex("sip2","<binning none/><encoding equality/>");
	tb->buildIndex("sip3","<binning none/><encoding equality/>");
	tb->buildIndex("sip4","<binning none/><encoding equality/>");
	tb->buildIndex("dip1","<binning none/><encoding equality/>");
	tb->buildIndex("dip2","<binning none/><encoding equality/>");
	tb->buildIndex("dip3","<binning none/><encoding equality/>");
	tb->buildIndex("dip4","<binning none/><encoding equality/>");
	tb->buildIndex("sport","<binning none/><encoding equality/>");
	tb->buildIndex("dport","<binning none/><encoding equality/>");
	*/
	delete tb;
	delete table;
	printf("fastbit building time = %d\n", time(NULL) - begin );
	gettimeofday( &tend, NULL );
	printf("time = %lf\n",( tend.tv_sec-tbeg.tv_sec ) + (tend.tv_usec-tbeg.tv_usec)/1e6 );
	return 0;
}
