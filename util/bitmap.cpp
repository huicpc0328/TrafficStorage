/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-11-14 15:01
#
# Filename: bitmap.cpp
#
# Description: 
#
===========================================================================*/
#include "bitmap.h"
#include <cstring>
#include <unistd.h>
#include <errno.h>

void Bitmap::merge( const Bitmap& bm ) {
	for( int i = 0 ; i < bm.elemSize && i < elemSize; ++i ) {
		byteArray[ i ] |= bm.byteArray[i];
	}
}

void Bitmap::copy( Bitmap & bm ) {
	for( int i = 0 ; i < bm.elemSize && i < elemSize; ++i ) {
		byteArray[i] = bm.byteArray[i];
	}
}

int Bitmap::dump2array( void *dst , int32_t byteLen ) const {
	assert( dst && byteLen >= 0 );

	if( !dst || byteLen < 0 ) {
		return -1;
	}
	byteLen = std::min( byteLen, getByteSize() );
	memcpy( dst, (void *)byteArray, byteLen );
	return 0;
}

int Bitmap::dump2file( int fd, int32_t offset ) const {
	assert( offset >= 0 );
	if( offset < 0 ) {
		return -1;
	}

	// seek target position that have a distance offset from current position defaultly. 
	off_t ret = lseek( fd, offset, SEEK_CUR );
	if( ret == -1 ) {
		ERROR_INFO( strerror(errno),return -1);
	}

	if( -1 == write( fd, (void *)&bitSize, sizeof(bitSize)) ) {
		ERROR_INFO( strerror(errno), return -1);
	}

	if( -1 == write( fd, (void *)&elemSize, sizeof(elemSize))) {
		ERROR_INFO( strerror(errno), return -1);
	}

	char *wp = (char *)byteArray;
	int  wsize = elemSize * sizeof( int32_t );

	while( 1 ) {
		ret = write( fd, (void *)wp, wsize ); 
		if( ret == -1 ) {
			ERROR_INFO( strerror(errno), return -1);
		}
		if( ret >= wsize ) break;
		wp += ret;
		wsize -= ret;
	}
	return 0;
}

int Bitmap::readFromFile( int fd, int32_t offset ) {
	assert( offset >= 0 );
	if( offset < 0 ) {
		return -1;
	}

	// seek target position that have a distance offset from current position defaultly. 
	int ret = lseek( fd, offset, SEEK_CUR );
	if( ret == -1 ) {
		ERROR_INFO( strerror(errno), return -1);
	}

	if( read( fd, (void *)&bitSize, sizeof(bitSize)) == -1 ) {
		ERROR_INFO( strerror(errno), return -1);
	}
	
	if( read( fd, (void *)&elemSize, sizeof(elemSize)) == -1 ) {
		ERROR_INFO( strerror(errno), return -1);
	}

	if( !byteArray ) delete[] byteArray;
	byteArray = new int32_t[ elemSize ];

	char *rp = (char *)byteArray;
	int rsize = elemSize * sizeof( int32_t );

	while(1) {
		ret = read( fd, (void *)rp, rsize );
		if( ret == -1 ) {
			ERROR_INFO( strerror(errno), return -1);
		}

		if( ret >= rsize ) break;
		rp += ret;
		rsize -= ret;
	}
	return 0;
}


// file io
int Bitmap::dump2file( FILE * fp ) const {
	if( !fp ) {
		ERROR_INFO("NULL file pointer fp",return -1);
	}

	fwrite( &bitSize, sizeof(bitSize), 1, fp );
	fwrite( &elemSize, sizeof(elemSize), 1, fp);
	fwrite( byteArray, sizeof(int32_t)*elemSize, 1, fp );
	return 0;
}


int Bitmap::readFromFile( FILE * fp ) {
	if( !fp ) {
		ERROR_INFO("NULL file pointer fp",return -1);
	}
	
	fread( &bitSize, sizeof(bitSize), 1, fp );
	fread( &elemSize, sizeof(elemSize), 1, fp);

	if( !byteArray ) delete[] byteArray;
	byteArray = new int32_t[ elemSize ];

	fread( byteArray, sizeof(int32_t)*elemSize, 1, fp );
	return 0;
}
