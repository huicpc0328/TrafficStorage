/*
 * dbhandle.cpp
 *
 *  Created on: 2012-9-6
 *      Author: hth
 */
#include "dbHandle.h"
//#include "monetdb/mapi.h"
#include <vector>
#include <cstdlib>

#if 0
// returned value: 0 for succeed, -1 for failed.
int MonetDBHandle::connect_database(){
	db_handle = mapi_connect(host.c_str(),port,username.c_str(),password.c_str(),"sql",dbname.c_str());
	if( mapi_error( db_handle ) ) {
		print_error_info();
		return -1;
	}
	return 0;
}

// -1 is returned when writing record to database failed.
int MonetDBHandle::write2db( string sql ) {

	if( db_handle == NULL )  return -1;

	statement_handle = mapi_query( db_handle, sql.c_str() );

	if( statement_handle == NULL || mapi_error(db_handle) != MOK ) {
		print_error_info();
		return -1;
	}
	if( mapi_close_handle(statement_handle) != MOK ) {
		print_error_info();
		return -1;
	}
	return 0;
}

void MonetDBHandle::print_error_info() {
	if (statement_handle != NULL) {
		mapi_explain_query(statement_handle, stderr);
		do {
			if (mapi_result_error(statement_handle) != NULL)
				mapi_explain_result(statement_handle, stderr);
		} while (mapi_next_result(statement_handle) == 1);
		mapi_close_handle(statement_handle);
		if( db_handle != NULL ) mapi_destroy( db_handle );
	} else if (db_handle != NULL) {
		mapi_explain(db_handle, stderr);
		mapi_destroy(db_handle);
		db_handle = NULL;
	} else {
		fprintf(stderr, "command failed\n");
	}
}

// return NULL when querying failed.
// we should free the memory of JSONArray manually after we using it.
MonetDBHandle::JSONArray* MonetDBHandle::query( string sql ) {

	if( db_handle == NULL )  return NULL;

	statement_handle = mapi_query( db_handle, sql.c_str() );

	if( statement_handle == NULL || mapi_error(db_handle) != MOK ) {
		print_error_info();
		return NULL;
	}

	JSONArray *total = new JSONArray();

	int 	columnNumber = mapi_get_field_count(statement_handle);
	std::vector<string> columnName;
#ifdef DEBUG
	printf("Column Number = %d\n",columnNumber);
#endif

	for( int i = 0 ; i < columnNumber; ++i) {
		columnName.push_back( string(mapi_get_name(statement_handle,i)) );
	}

	int rowNumber = 0;
	while( mapi_fetch_row(statement_handle) ) {
		JSONNode	row;
		for( int i = 0; i < columnNumber; ++i ) {
			row[ columnName[i] ] = JSONValue( mapi_fetch_field(statement_handle,i));
		}
		total->push_back( JSONValue(row) );
		++rowNumber;
		if( rowNumber%1000 == 0 ) printf("num=%d\n",rowNumber);
	}

	fprintf( stderr,"%d rows result matched query!\n",rowNumber);

#ifdef DEBUG
		std::cout<<*total<<std::endl;
#endif

	if( mapi_close_handle(statement_handle) != MOK ) {
		print_error_info();
		delete total;
		return NULL;
	}
	return total;
}

int MonetDBHandle::readOneRecord( string sql ,JSONNode& row) {
	if( db_handle == NULL )  return -1;

	if( statement_handle == NULL ) {
		statement_handle = mapi_query( db_handle, sql.c_str() );

		if( statement_handle == NULL || mapi_error(db_handle) != MOK ) {
			print_error_info();
			return -1;
		}
	}

	int 	columnNumber = mapi_get_field_count(statement_handle);
	std::vector<string> columnName;

	for( int i = 0 ; i < columnNumber; ++i) {
		columnName.push_back( string(mapi_get_name(statement_handle,i)) );
	}

	mapi_fetch_row(statement_handle);

	if( mapi_error(db_handle) != MOK ) {
		print_error_info();
		return -1;
	}

	for( int i = 0; i < columnNumber; ++i ) {
		row[ columnName[i] ] = JSONValue( mapi_fetch_field(statement_handle,i));
	}

	return 0;
}
#endif
