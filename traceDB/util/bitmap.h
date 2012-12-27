/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-11-14 10:22
#
# Filename: bitmap.h
#
# Description: Implement general data structure bitmap
#
===========================================================================*/

#ifndef	_BITMAP_H
#define _BITMAP_H

#define BITMAP_DEFAULT_SIZE 1024 * 1024
#define RIGHT_SHIFT_NUM  5
#define BITMASK		0x0000001f

#include <stdint.h>
#include <cstdio>
#include <assert.h>
#include "../global.h"

//template<typename Type> class BloomFilter;

class Bitmap {
	public:
		Bitmap( int64_t _bitSize = BITMAP_DEFAULT_SIZE ) :bitSize( _bitSize ) {
			assert( bitSize > 0 );
			
			elemSize = (int32_t)( (bitSize+BITMASK) >> RIGHT_SHIFT_NUM ); 
			byteArray = new int32_t[elemSize];
			bitSize = elemSize << RIGHT_SHIFT_NUM;

			if( !byteArray ) {
				ERROR_INFO("Cannot alloc memory for bitmap",);
			}
		}

		~Bitmap() {
			if( byteArray != NULL ) delete[] byteArray;
		}
		
		void setBit( int64_t  bitPos )  {
			assert( bitPos >= 0 && bitPos < bitSize );
			// for assert will not be effective in release mode 
			if( bitPos < 0 || bitPos >= bitSize ) {
				return ;
			}
			byteArray[ bitPos >> RIGHT_SHIFT_NUM ] |= 1<<(bitPos & BITMASK) ;
		}

		bool getBit( int64_t  bitPos ) const {
			assert( bitPos >= 0 && bitPos < bitSize );
			// for assert will not be effective in release mode 
			if( bitPos < 0 || bitPos >= bitSize ) {
				return false;
			}
			int32_t  location = (bitPos & BITMASK );
			return  ( byteArray[ bitPos >> RIGHT_SHIFT_NUM ] >> location ) & 1;
		}

		int32_t getBitSize() const {
			return bitSize;
		}

		int32_t getByteSize() const {
			return elemSize * sizeof(int32_t);
		}

		/* merge the bitmap bm to current bitmap */
		void merge( const Bitmap& bm ) ;
		
		void copy( Bitmap & bm );

		/* dump the bitmap to a memory  byte array */
		int dump2array( void * array, int32_t byteLen) const; 

		// standard IO --- use file descripter to read/write
		int dump2file( int fd, int32_t offset = 0 ) const; 

		int readFromFile( int fd, int32_t offset = 0 );

		// file IO --- use file pointer to fread/fwrite
		int dump2file( FILE* fp ) const;

		int readFromFile( FILE * fp );

		template<typename Type> friend class BloomFilter;

	private:
		int64_t 	bitSize;	/* size of bit in bitmap */
		int32_t		elemSize;	/* size of byte in bitmap */
		int32_t *	byteArray;	/* byte array to store bit information of bitmap */
};


#endif
