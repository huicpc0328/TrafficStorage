/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-12-17 16:54
#
# Filename: libtraceFile.h
#
# Description: 
#
===========================================================================*/

#ifndef _LIBTRACEFILE_H
#define _LIBTRACEFILE_H

#include "global.h"
#include "libtrace.h"
#include <string>

using libtrace::PACKET;
using libtrace::COMPRESSTYPE;
using libtrace::FILEOUTPUT;
using std::string;

class FileWriter {
private:
	COMPRESSTYPE 		com_type;	  /** compress type for file, gzip, lzo etc. */
	int					com_level;    /** compress level for file, 0 for none compress, 1 for faster compress, 9 for better compress*/

	FILEOUTPUT*			file_handle;  /** file handle to write packets into */

public:
	FileWriter( string name, COMPRESSTYPE t = TRACE_OPTION_COMPRESSTYPE_ZLIB, \
			int compressLevel = 9 );

	~FileWriter();

	static string assign_file_name();

	int	write_record( PACKET * );

	string	get_file_name();

	inline int get_packet_num() { return packet_num; }

};

#endif /* _LIBTRACEFILE_H */

