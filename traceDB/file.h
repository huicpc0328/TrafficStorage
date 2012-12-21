/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-12-17 14:35
#
# Filename: file.h
#
# Description: 
#
===========================================================================*/

#ifndef _FILE_H
#define _FILE_H

#include <string>
#include <stdio.h>

using std::string;

class File {
	string 		fileName;
	FILE*		fileHandle;
	uint32_t	fileSize;

	public:
	
	File( const string& name ): fileName(name) {
		fileHandle = fopen()
	}

}


#endif

