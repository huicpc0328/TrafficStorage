#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../index.h"
#include "ibis.h"
#include <table.h>

const int N = 11111;
int	 a[N][11];
char sql[N][128];

char *dirname;
char *filename;
inline void usage() {
	printf("type1: query for single srcip\n");
	printf("type2: query for combination of srcip and dstip\n");
	printf("type3: query for combination of srcip, dstip, sport, dport\n");
	printf("type4: range query for single srcip\n");

	printf("./testQuery querytype queryfile\n");
}

inline void gap( timeval & beg ) {
	timeval cur; 
	gettimeofday( &cur, NULL );
	printf("execution time = %lf\n", (cur.tv_sec-beg.tv_sec) + (cur.tv_usec-beg.tv_usec)/1e6 );
}

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
		char *p = strtok( s, " " );
		for( int i = 0 ; i < 4; ++i ) {
			a[cnt][i] = atoi( p );
			p = strtok( NULL," " );
		}
		snprintf( sql[cnt] , 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d",a[cnt][0],a[cnt][1],a[cnt][2],a[cnt][3]);
		cnt ++;
	}
	printf("%d queries are read\n", cnt );
	fclose( fp );

	timeval begin ;
	gettimeofday( &begin, NULL );
	IndexBase<uint32_t>* sipindex = new IndexBase<uint32_t>;
	LinkList<uint32_t>* ret;
	for( int i = 0 ; i < cnt; ++i ) {
		uint32_t tmp = (a[i][0]<<24)|(a[i][1]<<16)|(a[i][2]<<8)|a[i][3];
		ret = sipindex->queryFromFile( "sip.idx", tmp, tmp );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n", i, (ret == NULL) ? 0 :ret->size() );
#endif
		delete ret;
	}
	gap( begin );	

	printf("-----------------fastbit--------------\n");

	vector<uint32_t>resVec;
	gettimeofday( &begin, NULL );
	// fastbit
	// construct a data partition from the given data directory.
	ibis::part apart( dirname, static_cast<const char*>(0));
	// create a query object with the current user name.
	ibis::query aquery(ibis::util::userName(), &apart);
	// assign the query conditions as the where clause.
	
	int ierr;
	for( int i = 0 ; i < cnt; ++i ) {
		//snprintf( sql, 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d",a[i][0],a[i][1],a[i][2],a[i][3]);
		ierr = aquery.setWhereClause( sql[i] );
		if( ierr < 0 ) printf(" failed to set where clause %s\n", sql[i] );
		ierr = aquery.evaluate(); // evaluate the query
		
		//if( ierr < 0 ) printf(" failed to execute sql %s\n", sql );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n",i , aquery.getNumHits() );
		aquery.getHitRows( resVec );
		assert( resVec.size() == aquery.getNumHits() );
		resVec.clear();
#endif
	}
	gap( begin );
}

// combination sip and dip
void q2() {
	FILE *fp = fopen( filename, "r" );
	if( !fp ) {
		printf("cannot open file %s\n",filename );
		return ;
	}
	char s[128];
	int  cnt = 0;

	while( fgets( s, 128, fp ) ) {
		if( *s == 0 ) continue;
		char *p = strtok( s, " " );
		for( int i = 0 ; i < 8; ++i ) {
			a[cnt][i] = atoi( p );
			p = strtok( NULL," " );
		}
		
		snprintf( sql[cnt] , 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d and dip1=%d and dip2=%d and dip3=%d and dip4=%d"
				,a[cnt][0],a[cnt][1],a[cnt][2],a[cnt][3],a[cnt][4],a[cnt][5],a[cnt][6],a[cnt][7]);
		
		//snprintf( sql[cnt] , 128, "sip=%d and dip=%d"
		//		,(a[cnt][0]<<24)|(a[cnt][1]<<16)|(a[cnt][2]<<8)|a[cnt][3],(a[cnt][4]<<24)|(a[cnt][5]<<16)|(a[cnt][6]<<8)|a[cnt][7]);
		cnt ++;
	}
	printf("%d queries are read\n", cnt );
	fclose( fp );

	timeval begin ;
	gettimeofday( &begin, NULL );
	IndexBase<uint32_t>* sipindex = new IndexBase<uint32_t>;
	IndexBase<uint32_t>* dipindex = new IndexBase<uint32_t>;
	LinkList<uint32_t> *ret, *ret2;
	for( int i = 0 ; i < cnt; ++i ) {
		uint32_t tmp = (a[i][0]<<24)|(a[i][1]<<16)|(a[i][2]<<8)|a[i][3];
		ret = sipindex->queryFromFile( "sip.idx", tmp, tmp );
		if( !ret ) {
			fprintf( stderr, "%d = result size = %d\n", i,0);
			continue;
		}
		tmp = (a[i][4]<<24)|(a[i][5]<<16)|(a[i][6]<<8)|a[i][7];
		ret2 = dipindex->queryFromFile( "dip.idx", tmp, tmp );
#ifdef PRINT
		//printf("size1 = %d, size2 = %d\n", (ret==NULL)?0:ret->size(), (ret2==NULL)?0:ret2->size());
#endif

		ret->intersection( ret2 );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n", i, (ret==NULL)?0:ret->size() );
#endif
		delete ret;
		delete ret2;
	}
	gap( begin );	

	printf("-----------------fastbit--------------\n");

	gettimeofday( &begin, NULL );
	// fastbit
	// construct a data partition from the given data directory.
	ibis::part apart( dirname, static_cast<const char*>(0));
	// create a query object with the current user name.
	ibis::query aquery(ibis::util::userName(), &apart);
	// assign the query conditions as the where clause.
	
	vector<uint32_t> resVec;
	int ierr;
	for( int i = 0 ; i < cnt; ++i ) {
		//snprintf( sql, 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d",a[i][0],a[i][1],a[i][2],a[i][3]);
		ierr = aquery.setWhereClause( sql[i] );
		if( ierr < 0 ) printf(" failed to set where clause %s\n", sql[i] );
		ierr = aquery.evaluate(); // evaluate the query
		aquery.getHitRows( resVec );
		//if( ierr < 0 ) printf(" failed to execute sql %s\n", sql );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n",i , aquery.getNumHits() );
#endif
		assert( resVec.size() == aquery.getNumHits() );
		resVec.clear();
	}
	gap( begin );
}

// combination sip , dip , sport, dport
void q3() {
	FILE *fp = fopen( filename, "r" );
	if( !fp ) {
		printf("cannot open file %s\n",filename );
		return ;
	}
	char s[128];
	int  cnt = 0;

	while( fgets( s, 128, fp ) ) {
		if( *s == 0 ) continue;
		char *p = strtok( s, " " );
		for( int i = 0 ; i < 10; ++i ) {
			a[cnt][i] = atoi( p );
			p = strtok( NULL," " );
		}
		snprintf( sql[cnt] , 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d and dip1=%d and dip2=%d and dip3=%d and dip4=%d and sport=%d and dport=%d"
				,a[cnt][0],a[cnt][1],a[cnt][2],a[cnt][3],a[cnt][4],a[cnt][5],a[cnt][6],a[cnt][7],a[cnt][8],a[cnt][9]);
		
		cnt ++;
	}
	printf("%d queries are read\n", cnt );
	fclose( fp );

	timeval begin ;
	gettimeofday( &begin, NULL );
	IndexBase<uint32_t>* sipindex = new IndexBase<uint32_t>;
	IndexBase<uint32_t>* dipindex = new IndexBase<uint32_t>;
	IndexBase<uint16_t>* sportindex = new IndexBase<uint16_t>;
	IndexBase<uint16_t>* dportindex = new IndexBase<uint16_t>;
	LinkList<uint32_t> *ret, *ret2;
	for( int i = 0 ; i < cnt; ++i ) {
		uint32_t tmp = (a[i][0]<<24)|(a[i][1]<<16)|(a[i][2]<<8)|a[i][3];
		ret = sipindex->queryFromFile( "sip.idx", tmp, tmp );
		if( !ret ) continue;

		tmp = (a[i][4]<<24)|(a[i][5]<<16)|(a[i][6]<<8)|a[i][7];
		ret2 = dipindex->queryFromFile( "dip.idx", tmp, tmp );
		ret->intersection( ret2 );
		delete ret2;

		if( !ret ) continue;
		ret2 = sportindex->queryFromFile( "sport.idx", a[i][8],a[i][8] );
#ifdef PRINT
		//printf("size1 = %d, size2 = %d\n", ret->size(), (ret2==NULL)?0:ret2->size());
#endif
		ret->intersection( ret2 );
		delete ret2;
		if( !ret ) continue;

		ret2 = dportindex->queryFromFile( "dport.idx", a[i][9],a[i][9] );
		ret->intersection( ret2 );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n", i, (ret==NULL)?0:ret->size() );
#endif
		delete ret;
		delete ret2;
	}
	gap( begin );	

	printf("-----------------fastbit--------------\n");

	vector<uint32_t>resVec;
	gettimeofday( &begin, NULL );
	// fastbit
	// construct a data partition from the given data directory.
	ibis::part apart( dirname, static_cast<const char*>(0));
	// create a query object with the current user name.
	ibis::query aquery(ibis::util::userName(), &apart);
	// assign the query conditions as the where clause.
	
	int ierr;
	for( int i = 0 ; i < cnt; ++i ) {
		//snprintf( sql, 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d",a[i][0],a[i][1],a[i][2],a[i][3]);
		ierr = aquery.setWhereClause( sql[i] );
		if( ierr < 0 ) printf(" failed to set where clause %s\n", sql[i] );
		ierr = aquery.evaluate(); // evaluate the query
		//if( ierr < 0 ) printf(" failed to execute sql %s\n", sql );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n",i , aquery.getNumHits() );
#endif 
		aquery.getHitRows( resVec );
		assert( resVec.size() == aquery.getNumHits() );
		resVec.clear();
	}
	gap( begin );
}

void q4(){
	FILE *fp = fopen( filename, "r" );
	if( !fp ) {
		printf("cannot open file %s\n",filename );
		return ;
	}
	char s[128];
	int  cnt = 0;

	while( fgets( s, 128, fp ) ) {
		if( *s == 0 ) continue;
		char *p = strtok( s, " " );
		for( int i = 0 ; i < 4; ++i ) {
			a[cnt][i] = atoi( p );
			p = strtok( NULL," " );
		}
		snprintf( sql[cnt] , 128, "sip1=%d and sip2=%d and sip3=%d",a[cnt][0],a[cnt][1],a[cnt][2]);
		cnt ++;
	}
	printf("%d queries are read\n", cnt );
	fclose( fp );

	timeval begin ;
	gettimeofday( &begin, NULL );
	IndexBase<uint32_t>* sipindex = new IndexBase<uint32_t>;
	LinkList<uint32_t>* ret;
	for( int i = 0 ; i < cnt; ++i ) {
		uint32_t tmp = (a[i][0]<<24)|(a[i][1]<<16)|(a[i][2]<<8);
		ret = sipindex->queryFromFile( "sip.idx", tmp, tmp|0xff );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n", i, ret->size() );
#endif
		delete ret;
	}
	gap( begin );	

	printf("-----------------fastbit--------------\n");

	vector<uint32_t>resVec;
	gettimeofday( &begin, NULL );
	// fastbit
	// construct a data partition from the given data directory.
	ibis::part apart( dirname, static_cast<const char*>(0));
	// create a query object with the current user name.
	ibis::query aquery(ibis::util::userName(), &apart);
	// assign the query conditions as the where clause.
	
	int ierr;
	for( int i = 0 ; i < cnt; ++i ) {
		//snprintf( sql, 128, "sip1=%d and sip2=%d and sip3=%d and sip4=%d",a[i][0],a[i][1],a[i][2],a[i][3]);
		ierr = aquery.setWhereClause( sql[i] );
		if( ierr < 0 ) printf(" failed to set where clause %s\n", sql[i] );
		ierr = aquery.evaluate(); // evaluate the query
		//if( ierr < 0 ) printf(" failed to execute sql %s\n", sql );
#ifdef PRINT
		fprintf( stderr, "%d = result size = %d\n",i , aquery.getNumHits() );
#endif
		aquery.getHitRows( resVec );
		assert( resVec.size() == aquery.getNumHits() );
		resVec.clear();
	}
	gap( begin );
}

int main( int argc, char **argv ) {
	if( argc < 4 ) {
		usage(); return 0;
	}
	int type = atoi( argv[1] );
	filename = argv[2];
	dirname = argv[3];
#ifdef LINKNODE_POOL
	LinkList<uint32_t>::initLinkNodePool( 5000000 );
#endif

	switch( type ) {
		case 1: q1();break;
		case 2: q2();break;
		case 3: q3();break;
		case 4: q4();break;
		default: printf("error type\n");
	}

	return 0;
}

