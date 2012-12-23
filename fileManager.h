/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-9-17 17:43
#
# Filename: FileManager.h
#
# Description: 
#
===========================================================================*/

#ifndef _FILEMANAGER_H
#define _FILEMANAGER_H

#include <string>
#include <stdint.h>

#define MAX_FILE_NUM 100

using std::string;

class FileManager {
	static string	directory_name;
	static uint16_t	file_num;

	public:

	FileManager();

	~FileManager() {
	}

	// return an available file name
	static inline uint16_t apply_file_ID() {
		file_num %= MAX_FILE_NUM;
		return file_num++;
	}

	static inline string get_directory_name() {
		return directory_name;
	}
};


#endif
