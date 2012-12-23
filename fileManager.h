/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-9-17 17:43
#
# Filename: fileManager.h
#
# Description: 
#
===========================================================================*/

#ifndef _FILEMANAGER_H
#define _FILEMANAGER_H

#include <string>

#define MAX_FILE_NUM 100

using std::string;

class fileManager {
	string		directory_name;
	string		file_format;
	uint16_t	file_num;

	public:

	fileManager(const string& name, const string& format);

	~fileManager() {
	}

	// return an available file name
	string apply_file_name();

	// return the file name according to its file ID 
	string	get_file_name(uint16_t id);


}


#endif
