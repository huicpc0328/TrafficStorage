#include "index.h"
#include "fileManager.h"
#include <utility>
#include <stdio.h>

#define DEBUG

typedef pair<uint16_t,Index *> UIPAIR;

const char * INDEX_FILE_SUFFIX = ".idx";

// sipindex, dipindex, sportindex ...
static void *index_thread_start( void * arg) {
	UIPAIR* uipair = (UIPAIR *) arg;
	INDEXCODE code = (INDEXCODE)uipair->first; 
	Index* index = uipair->second;
	delete uipair; 

	/*
	int state = PTHREAD_CANCEL_DEFERRED, oldstate ;
	pthread_setcanceltype( state, &oldstate );
	*/
	printf("Thread %d has code %d\n", (int32_t)pthread_self(), (int)code );

	while( 1 ) {
		pthread_mutex_lock( & (index->queueLock) );

		while( !(code & (index->readCode ) )) {
			pthread_cond_wait( &(index->fullCond), &(index->queueLock));
			printf("thread whose code is %d received a signal!\n", code );
		}

#ifdef DEBUG
		printf("Thread %d has code %d readCode = %u\n", pthread_self(), code, index->readCode );
#endif
		(index->readCode) ^= code;
		pthread_mutex_unlock( &(index->queueLock) );
		index->addPkt( code );
#ifdef DEBUG
		printf("Thread %d whose code is %d finished reading, readCode = %u\n", pthread_self(), code, index->readCode );
#endif
		pthread_cond_signal( &(index->emptyCond) );

		// check exit-signal;
		if( index->get_exit_signal() ) {
			pthread_exit(0);
		}

		/*
		// check the cancel condition when exporter's queue was empty
		if( index->linkNodePool.size() == 0 ) {
			pthread_testcancel();
		}
		*/
	}
}

Index::Index( uint16_t flag ): indexFlag(flag) {
	sipIndex = dipIndex = NULL;
	sportIndex = dportIndex = NULL;
	pthread_mutex_init( &queueLock, NULL);
	pthread_cond_init( &emptyCond, NULL );
	pthread_cond_init( &fullCond, NULL );
	readCode = 0;

	// initialization of exit signal;
	exit_signal = false;
	pthread_mutex_init( &exit_mutex, NULL );

	if( flag & SRCIP ) {
		sipIndex = new BTree<uint32_t,uint32_t>();
		pthread_t	tid;
		// this memory should be delete in function index_thread_start
		// why we did it in this way because the local stack memory may be not
		// available or correct when the created thread access
		UIPAIR 	*uipair = new UIPAIR(SRCIP,this);
		int err = pthread_create( &tid, NULL, index_thread_start, (void *)uipair);
		if( err ) {
			fprintf(stderr,"Cannot create thread in file %s, function %s, MSG:%s\n",__FILE__,__func__,strerror(err));
		} else {
			threadVec.push_back( tid );
		}
	}

	if( flag & DSTIP ) {
		dipIndex = new BTree<uint32_t,uint32_t>();
		pthread_t	tid;
		// this memory should be delete in function index_thread_start
		UIPAIR 	*uipair = new UIPAIR(DSTIP,this);
		int err = pthread_create( &tid, NULL, index_thread_start, (void *)uipair );
		if( err ) {
			fprintf(stderr,"Cannot create thread in file %s, function %s, MSG:%s\n",__FILE__,__func__,strerror(err));
		}else {
			threadVec.push_back( tid );
		}
	}

	if( flag & SPORT ) {
		sportIndex = new BTree<uint16_t,uint32_t>();
		pthread_t	tid;
		// this memory should be delete in function index_thread_start
		UIPAIR 	*uipair = new UIPAIR(SPORT,this);
		int err = pthread_create( &tid, NULL, index_thread_start, (void *)uipair );
		if( err ) {
			fprintf(stderr,"Cannot create thread in file %s, function %s, MSG:%s\n",__FILE__,__func__,strerror(err));
		} else {
			threadVec.push_back( tid );
		}
	}

	if( flag & DPORT ) {
		dportIndex = new BTree<uint16_t,uint32_t>();
		pthread_t	tid;
		// this memory should be delete in function index_thread_start
		UIPAIR 	*uipair = new UIPAIR(DPORT,this);
		int err = pthread_create( &tid, NULL, index_thread_start, (void *)uipair);
		if( err ) {
			fprintf(stderr,"Cannot create thread in file %s, function %s, MSG:%s\n",__FILE__,__func__,strerror(err));
		} else {
			threadVec.push_back( tid );
		}
	}
}

Index::~Index() {
	pthread_cond_destroy( &emptyCond );
	pthread_cond_destroy( &fullCond );
	pthread_mutex_destroy( &queueLock );
	
	this->set_exit_signal();
	for( int i = 0 ; i < threadVec.size(); ++i ) {
		pthread_join( threadVec[i], NULL);
	}
	pthread_mutex_destroy( &exit_mutex );

	if( sipIndex ) delete sipIndex;
	if( dipIndex ) delete dipIndex;
	if( sportIndex ) delete sportIndex;
	if( dportIndex ) delete dportIndex;

	// release memory in linkNodePool
	for( int i = 0 ; i < linkNodePool.size(); ++i ) {
		delete linkNodePool.get_resource(i);
	}
}

void Index::addPkt(INDEXCODE code) {
	Ipv4Record* cur = NULL;
	// cur->pktLen represent the start position in file of this flow-packets
	switch( code ) {
		case SRCIP: {
			for( int i = 0; i < linkNodePool.size(); ++i ) {
				cur = (Ipv4Record*)linkNodePool.get_resource(i);
				sipIndex->addItem( cur->get_srcip(), cur->get_file_offset() );
			}
			break;
		}
		case DSTIP: {
			for( int i = 0; i < linkNodePool.size();++i ) {
				cur = (Ipv4Record*)linkNodePool.get_resource(i);
				dipIndex->addItem( cur->get_dstip(), cur->get_file_offset() );
			}
			break;
		}
		case SPORT: {
			for( int i = 0 ; i < linkNodePool.size(); ++i) {
				cur = (Ipv4Record*)linkNodePool.get_resource(i);
				sportIndex->addItem( cur->get_sport(), cur->get_file_offset() );
			}
			break;
		}
		case DPORT: {
			for( int i = 0 ; i < linkNodePool.size(); ++i) {
				cur = (Ipv4Record*)linkNodePool.get_resource(i);
				sportIndex->addItem( cur->get_sport(), cur->get_file_offset() );
			}
			break;
		}
		default:
			fprintf(stderr, "Error Code in file %s function %s\n", __FILE__,__func__);
	}
}

void Index::write_index_to_file( uint16_t fileID ) {
	char fileName[128];

	if( indexFlag & SRCIP ) {
		snprintf( fileName, 128, "%ssip-%d%s",\
				FileManager::get_directory_name().c_str(),fileID,INDEX_FILE_SUFFIX );	
		sipIndex->write2file( fileName );
		delete sipIndex;
		sipIndex = new BTree<uint32_t,uint32_t>();
	}

	if( indexFlag & DSTIP ) {
		snprintf( fileName, 128, "%sdip-%d%s",\
				FileManager::get_directory_name().c_str(),fileID,INDEX_FILE_SUFFIX );	
		dipIndex->write2file( fileName );
		delete dipIndex;
		dipIndex = new BTree<uint32_t,uint32_t>();
	}

	if( indexFlag & SPORT ) {
		snprintf( fileName, 128, "%ssport-%d%s",\
				FileManager::get_directory_name().c_str(),fileID,INDEX_FILE_SUFFIX );	
		sportIndex->write2file( fileName );
		delete sportIndex;
		sportIndex = new BTree<uint16_t,uint32_t>();
	}

	if( indexFlag & DPORT ) {
		snprintf( fileName, 128, "%sdport-%d%s",\
				FileManager::get_directory_name().c_str(),fileID,INDEX_FILE_SUFFIX );	
		dportIndex->write2file( fileName );
		delete dportIndex;
		dportIndex = new BTree<uint16_t,uint32_t>();
	}
}

void Index::clear_linkNodePool() {
		int size = linkNodePool.size();
		for( int i = 0 ; i < size; i++ ) {
			delete linkNodePool.get_free_resource();
		}
}
