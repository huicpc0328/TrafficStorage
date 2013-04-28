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
#ifdef BTREE
#include "util/btree.h"
#else 
#include "util/trie.h"
#endif
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

// used as an interface
template<typename KEY>
class IndexBase {
	protected:
	typedef uint32_t VALUE;
	typedef LinkList<VALUE>* RESULT;
#ifdef BTREE
	BTree<KEY,VALUE>*       index;
#else
	Trie<KEY>*			    index;
#endif

	public:
	IndexBase() {
#ifdef BTREE
		index = new BTree<KEY,VALUE>();
#else
		index = new Trie<KEY>();
#endif
	}
	~IndexBase() {
		if( index ) delete index;
	}

	inline int addItem( const KEY& key, const VALUE& value ) {
		return index->addItem( key, value );
	}

	inline int write2file( const char *fileName ) {
		return index->write2file( fileName );
	}

	inline RESULT queryFromFile( const char *file, const KEY& b, const KEY& e ) {
		return index->queryFromFile( file, b, e );
	}

	inline int readFromFile( const char * fileName ) {
		return index->readFromFile( fileName );
	}
};



class Index {
	IndexBase<uint32_t>*	sipIndex;
	IndexBase<uint32_t>*	dipIndex;
	IndexBase<uint16_t>*	sportIndex;
	IndexBase<uint16_t>*	dportIndex;

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
	
	void buildIndex( INDEXCODE code );

	void clear_linkNodePool() ;

	void write_index_to_file( uint16_t fileID );
		
	friend void *index_thread_start( void  *);

	friend class Exporter;
};



#endif
