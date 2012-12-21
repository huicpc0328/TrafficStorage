/*
 * dbhandle.h
 *
 *  Created on: 2012-9-6
 *      Author: hth
 */

#ifndef DBHANDLE_H_
#define DBHANDLE_H_

#include "monetdb/mapi.h"
#include <string>
#include <inttypes.h>
#include "include/JsonBox.h"

using std::string;



class DBHandle{
protected:
	string 		username;
	string 		host;
	string 		password;
	string		dbname;
	uint16_t	port;
public:

	// data format for queried answer;
	typedef JsonBox::Array		JSONArray;
	typedef JsonBox::Object		JSONNode;
	typedef JsonBox::Value		JSONValue;

	DBHandle( string h, string u, string p, string name, int _port ):\
		host(h), username(u), password(p), dbname(name), port(_port){}
	virtual ~DBHandle(){}
	virtual int 		connect_database() = 0;
	virtual int 		write2db( string sql ) = 0;
	virtual void		print_error_info() = 0 ;
	virtual JSONArray*	query( string sql ) = 0;
	virtual int	readOneRecord(string sql, JSONNode &) = 0;
};


class MonetDBHandle: DBHandle {
private:
	Mapi			db_handle;				/* the handle to connect and process with monet database */
	MapiHdl			statement_handle;		/* the statement object to execute sql statement */
public:
	MonetDBHandle(string h, string u, string p, string name, int _port):
		DBHandle(h , u , p , name , _port ) {
		db_handle = NULL;
		statement_handle = NULL;
	}
	virtual ~MonetDBHandle() {
		if( NULL != db_handle ) mapi_destroy( db_handle );
	}

	int connect_database();
	int write2db(string sql);
	void print_error_info();
	// NULL for querying failed.
	JSONArray* 	query( string sql );
	// -1 failed, 0 succeed.
	int	readOneRecord(string sql, JSONNode &jsonNode);
};

#endif /* DBHANDLE_H_ */
