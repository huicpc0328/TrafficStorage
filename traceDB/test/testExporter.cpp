#include "../exporter.h"
#include "../record.h"
#include "../collector.h"
#include "../parser.h"
#include "../include/JsonBox.h"
#include "../dbHandle.h"
#include <stdio.h>
#include <stdlib.h>
#include "monetdb/mapi.h"
#include "libtrace.h"

using namespace std;

Collector collector;
MonetDBHandle* db = new MonetDBHandle("localhost","monetdb","monetdb","test",50000);
Exporter<1024> ipv4_exporter("ipv4",collector,(DBHandle*)db);
Exporter<1024> ipv6_exporter("ipv6",collector,(DBHandle*)db);

#define NUM 3


void die(Mapi dbh, MapiHdl hdl)
{
	if (hdl != NULL) {
			mapi_explain_query(hdl, stderr);
				do {
							if (mapi_result_error(hdl) != NULL)
											mapi_explain_result(hdl, stderr);
								} while (mapi_next_result(hdl) == 1);
					mapi_close_handle(hdl);
					mapi_destroy(dbh);
	} else if (dbh != NULL) {
			mapi_explain(dbh, stderr);
			mapi_destroy(dbh);
	} else {
			fprintf(stderr, "command failed\n");
	}
	exit(-1);
}

int main() {
	Parser *paser[NUM];
	for( int i = 0 ;i < NUM;i++ ) {
		paser[i] = new Parser( collector );
		paser[i]->start();
	}

	if( db->connect_database() == -1 ) {
		db->print_error_info();
		return 0;
	}

	JsonBox::Object jsonNode;
	for( int i = 0 ; i < 20 ; ++i ) {
		db->readOneRecord("select * from ipv4", jsonNode);
		cout<<jsonNode<<endl;
	}

	collector.collect();

	if( db != NULL ) delete db;
	return 0;
}
