/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-12-18 15:17
#
# Filename: index.h
#
# Description: 
#
===========================================================================*/

#ifndef _INDEX_H
#define _INDEX_H

#include <pthread.h>
#include <vector>
#include "util/btree.h"
#include "resourcePool.h"
#include "record.h"
using std::vector;

enum INDEXCODE {
	SRCIP = 1,
	DSTIP = 2,
	SPORT = 4,
	DPORT = 8,
	PROTO = 16
};

static void *index_thread_start( void *) ;

class Index {
	BTree<uint32_t,uint32_t>* 	sipIndex;
	BTree<uint32_t,uint32_t>* 	dipIndex;
	BTree<uint16_t,uint32_t>* 	sportIndex;
	BTree<uint16_t,uint32_t>* 	dportIndex;

	ResourcePool<Record*,1000000>	linkNodePool;
	uint16_t					indexFlag; // represent which fileds should be indexed
	vector<pthread_t>			threadVec;
	pthread_mutex_t				queueLock;
	pthread_cond_t				emptyCond;
	pthread_cond_t				fullCond;
	int16_t						readCode; // represent which index have read the queue or not
	
	// signal for thread exiting
	pthread_mutex_t				exit_mutex;
	bool						exit_signal;

	public:
	
	Index( uint16_t flag = SRCIP|DSTIP|SPORT|DPORT ) ;
	
	~Index();

	inline bool get_exit_signal() {
		pthread_mutex_lock( &exit_mutex );
		bool ret = exit_signal;
		pthread_mutex_unlock( &exit_mutex );
		return ret;
	}

	inline void set_exit_signal() {
		pthread_mutex_lock( &exit_mutex );
		exit_signal = true;
		pthread_mutex_unlock( &exit_mutex );
	}
	
	void addPkt( INDEXCODE code );

	void clear_linkNodePool() ;

	void write_index_to_file( uint16_t fileID );
		
	friend void *index_thread_start( void  *);

	friend class Exporter;
};



#endif
