/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-9-17 17:42
#
# Filename: fileWriter.h
#
# Description: 
#
===========================================================================*/

#ifndef _FILEWRITER_H
#define _FILEWRITER_H

#define MAX_FILE_SIZE 4LL*1000*1000*1000

#include <string>
#include <stdint.h>
using std::string;

class FileWriter {
private:
	string	 			file_name;    /** absolute file path */
	FILE*				file_handle;  /** file handle to write data into */
	uint32_t 			record_num;
	uint32_t			file_size; /* file size must be less than 4G */

public:
	FileWriter( const string& name ); 

	~FileWriter();

	int reset_file(const string& name);

	int	write_file( void *addr, size_t size) ;

	inline int get_record_num() {
		return record_num;
	}

	inline uint32_t get_file_size() {
		return file_size;
	}

	inline void incRecord() {
		++record_num;
	}

};

#endif /* _FILEWRITER_H */
