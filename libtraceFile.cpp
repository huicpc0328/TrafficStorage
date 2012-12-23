/*
 * filewriter.cpp
 *
 *  Created on: 2012-9-7
 *      Author: hth
 */
#include "filewriter.h"
#include <cstdio>

uint32_t 	FileWriter::file_count = 0;
std::string FileWriter::file_path = "/home/hth/traceDB/data/";
std::string FileWriter::file_type = "pcapfile:";
std::string FileWriter::file_suffix = ".pcap.gz";


FileWriter::FileWriter( std::string name, COMPRESSTYPE t, int compressLevel ) {
		com_level = compressLevel;
		com_type = t;
		file_name = name;
		packet_num = 0;
		std::string prefix = FileWriter::file_type + FileWriter::file_path;
		file_handle = trace_create_output( (prefix+file_name+FileWriter::file_suffix).c_str() );
		if( trace_is_err_output(file_handle) ){
	        trace_perror_output(file_handle,"Opening output trace file");
	        trace_destroy_output(file_handle);
	        file_handle = NULL;
		} else {
			if( -1 == trace_config_output( file_handle, TRACE_OPTION_OUTPUT_COMPRESSTYPE,(void *)&t) ){
				trace_perror_output(file_handle,"Cannot set the compress type!");
			}
			if( -1 == trace_config_output( file_handle, TRACE_OPTION_OUTPUT_COMPRESS, (void *)&compressLevel) ) {
				trace_perror_output(file_handle,"Cannot set the compress level!");
			}
			if( -1 == trace_start_output(file_handle) ) {
				 trace_perror_output(file_handle,"Starting output trace");
			}
		}
}

FileWriter::~FileWriter() {
	if( file_handle != NULL ) {
		trace_destroy_output( file_handle );
	}
}


int FileWriter::write_record(PACKET *packet) {
	if( packet == NULL ) return -1;
	if( trace_write_packet( file_handle, packet ) == -1 ) {
		 trace_perror_output(file_handle, "Writing packet");
		 return -1;
	}
	++packet_num;
	return 0;
}


std::string FileWriter::assign_file_name() {
	char s[20];
	struct timeval tv;
	gettimeofday( &tv, NULL );
	snprintf(s, sizeof(s), "%ld-%u",tv.tv_sec,FileWriter::file_count++);
	return std::string(s);
}

std::string FileWriter::get_file_name(){
	return file_name;
}
