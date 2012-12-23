/* This is a module of exporting packets from memory to disk,
 * and we put similar packets into the same file block.
 *
 * create by hth on 30/08/12
 */

#ifndef	_EXPORTER_H
#define	_EXPORTER_H

#include <string>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <utility>
#include <pthread.h>

#include "dbHandle.h"
#include "fileWriter.h"
#include "index.h"
#include "record.h"
#include "collector.h"

#define 	SCAN_INTERVAL 		1    /* how long to scan hash_table and export packets, unit: second */
#define		FLOW_TIMEOUT		180     /*  3 minutes */
#define		MAX_FILE_PACKETS	1024	/* max packets number per file stored */
#define		BUCKETSIZE 			1024*1024

typedef Record RECORD;

template<typename T>
void Swap( T& a, T& b ) {
	T t = a;
	a = b;
	b = t;
}

// ad-hoc structure
class LinkNode {
public:
	RECORD * 	data;     /* record content */
	LinkNode* 	nextdata; /* next LinkNode that has the same record content (some fields of packet header) */
	LinkNode*	nextlink; /* next LinkNode that has the same bucket index but different record content */
	LinkNode*	tail;	  /* the tail pointer of the chain that has the same record content */
	LinkNode( RECORD * rec ): data(rec) {
		nextdata = nextlink = tail = NULL;
		assert( data != NULL );
	}
};

using std::string;
using std::pair;

static void * scan_thread_start( void *);

class Exporter {
private:
	string			name;			/* ipv4,ipv6,arp etc. */
	uint32_t		totalRecordNum;  /* total number of records in current exporter buffer */
	LinkNode *		hash_table[BUCKETSIZE]; /* head pointer of each hash bucket */
	Collector&		collector;				/* the reference of unique collector */

#ifdef HASH_PERF
	int				bucket_size[BUCKETSIZE]; /* the number of different records located at one bucket, size of collision*/
#endif

	DBHandle*		db_handle;		    	/*  the handle of write record to database */
	pair<int,int>	packet_num[BUCKETSIZE]; /* packets number of each bucket */

	pthread_mutex_t bucket_mutex[BUCKETSIZE];	/* mutex to make sure multi-thread synchronization*/

	// scan which flow-packets that need to be exporter 
	pthread_t		scanThread;

	Index			*index;
	
	// signal for thread exiting
	pthread_mutex_t	signal_mutex;
	bool			exit_signal;

public:
	Exporter( const string& _name, Collector &c, DBHandle* db = NULL );

	~Exporter(); 

	inline uint32_t size() { 
		return totalRecordNum;
	}

	inline pthread_t get_scan_thread() {
		return scanThread;
	}

	inline vector<pthread_t> get_index_threads() {
		return index->threadVec;
	}

	inline bool get_exit_signal() {
		pthread_mutex_lock( &signal_mutex );
		bool ret = exit_signal;
		pthread_mutex_unlock( &signal_mutex );
		return ret;
	}

	inline void set_exit_signal() {
		pthread_mutex_lock( &signal_mutex );
		exit_signal = true;
		pthread_mutex_unlock( &signal_mutex );
	}

	int		write2db( const string& sql );
	
	int 	push_record( RECORD * );
	
	void 	export_chain( LinkNode* , FileWriter * );

	void 	export_longer_chains();

	void	export_timeout_flows(int timeout=FLOW_TIMEOUT);

	void 	scan_hash_table();

	friend	void * scan_thread_start( void *);
};

#endif
