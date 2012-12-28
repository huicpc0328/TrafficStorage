#include <iostream>
#include <cstdio>
#include <sys/stat.h>
#include <cstdlib>
#include "../bloomfilter.h"
#include "../string_hash.h"
#include <string>
#include <map>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

const int N = 1111111;
const int M = 16;

string a[N];

void generate() {
	char s[M];
	srand( time(0) );
	for( int i = 0 ; i < N; ++i ){
		int len = rand()%10 + 6;
		for( int j = 0 ; j < len; ++j ) {
			s[j] = rand()%26 + 'a';
		}
		s[len] = '\0';
		a[i] = string(s);
	}
}

map<string,int>hashMap;
BloomFilter<string>bf(N);

void test_accuracy() {
	for( int i = 0 ; i < N/2 ; ++i ) {
		hashMap[ a[i] ] = 1;
		if( i < 50 ) cout<<a[i]<<endl;
		bf.insertElement( a[i] );
	}
	int FP = 0;
	int TP = 0;

	for( int i = 0 ; i < N; ++i ) {
		bool ret = bf.containElement( a[i] );
		
		if( hashMap[a[i]] == 1 && !ret ) TP++;
		if( hashMap[a[i]] == 0 && ret )  FP++;
	}
	cout<<"FP: "<<FP<<" TP:"<<TP<<endl;
}

void test_file() {
	char s[] = "test.dat";
	int fd = open( s, O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR );
	if( fd < 0 ) {
		printf("cannot open file %s in File %s, line %d\n", s , __FILE__, __LINE__ );
		return ;
	}
	
	bf.dump2file( fd );
	close( fd );

	fd = open( s, O_RDONLY );
	if( fd < 0 ) {
		printf("cannot open file %s\n in File %s, line %d\n", s , __FILE__, __LINE__ );
		return ;
	}

	BloomFilter<string> bf2( N );
	bf2.readFromFile( fd );
	int hashNum = bf2.getHashFuncNum();
	
	for( int i = 0 ; i < hashNum; ++i ) {
		STRHASH hash = StringHash::getHashFunc(i);
		bf2.setHashFunc( hash, i);
	}
	int FP = 0;
	int TP = 0;

	for( int i = 0 ; i < N ; ++i ) {
		bool ret = bf2.containElement( a[i] );
		
		if( hashMap[a[i]] == 1 && !ret ) TP++;
		if( hashMap[a[i]] == 0 && ret )  FP++;
	}
	cout<<"FP: "<<FP<<" TP:"<<TP<<endl;
	close( fd );

}

int main() {
	generate();
	int hashNum = bf.getHashFuncNum();
	
	StringHash* ptr = StringHash::getHashPtr();
	for( int i = 0 ; i < hashNum; ++i ) {
		STRHASH hash = StringHash::getHashFunc(i);
		bf.setHashFunc( hash, i);
	}
	test_accuracy();
	test_file();
	return 0;
}
