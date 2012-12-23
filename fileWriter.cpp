/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-9-17 17:42
#
# Filename: fileWriter.cpp
#
# Description: 
#
===========================================================================*/

#include "fileWriter.h"
#include <cstdio>
#include <assert.h>

FileWriter::FileWriter(const string & name) {
		file_name = name;
		record_num = 0;
		file_size = 0;
		file_handle = fopen( name.c_str(), "wb" );
		if( !file_handle ) {
			fprintf( stderr, "Cannot open file %s in code %s, line %d\n",name.c_str(),__FILE__,__LINE__ );
			assert( false );
		}
}

FileWriter::~FileWriter() {
	if( file_handle != NULL ) {
		fclose( file_handle );
	}
}

int FileWriter::reset_file( const string& name ) {
	file_size = 0;
	record_num = 0;
	file_name = name;

	if( file_handle ) {
		fclose( file_handle );
	}
	file_handle = fopen( name.c_str(), "wb" );
	if( !file_handle ) {
		fprintf( stderr, "Cannot open file %s in code %s, line %d\n",name.c_str(),__FILE__,__LINE__ );
		return -1;	
	}
	return 0;

}

int FileWriter::write_file( void * addr, size_t size) {
	if( file_handle == NULL || file_size >= MAX_FILE_SIZE ) return -1;
	if( addr == NULL ) {
		fprintf( stderr, "argument addr is a NULL pointer in code %s, line %d\n",__FILE__,__LINE__ );
		return -1;	
	}

	fwrite( addr, size, 1, file_handle );
	file_size += size;
	return 0;
}


