#include "exporter.h"

#ifdef DEBUG
#include <stdio.h>
#endif

static void * scan_thread_start( void *arg ) {
	int state = PTHREAD_CANCEL_DEFERRED, oldstate ;
	pthread_setcanceltype( state, &oldstate );
	((Exporter*)arg)->scan_hash_table();
}

void Exporter::scan_hash_table() {
	
	printf("Scan Thread %d has started!\n", (uint32_t)pthread_self());

	while( 1 ) {
		pthread_mutex_lock( &(index->queueLock));
		while( index->readCode != 0 ) {
			pthread_cond_wait( &(index->emptyCond), &(index->queueLock) );
		}

		int size = index->linkNodePool.size();
		for( int i = 0 ; i < size; i++ ) {
			delete index->linkNodePool.get_free_resource();
		}

		// check receiving exit signal or not
		// did this examination comsume much cpu resource ?
		if( this->get_exit_signal() ) {
			// export all records in hash table when received exit signal;
			export_timeout_flows( 0 );
			index->readCode = (index->indexFlag);
			pthread_mutex_unlock( &(index->queueLock) );
#ifdef DEBUG
			printf("%d packets need to be export!\n", index->linkNodePool.size());
#endif
			pthread_cond_broadcast( &(index->fullCond) );
			this->index->set_exit_signal();	
			pthread_exit(0);
		}
		export_timeout_flows();
		index->readCode = (index->indexFlag);
		pthread_mutex_unlock( &(index->queueLock) );
#ifdef DEBUG
		printf("%d packets need to be export!\n", index->linkNodePool.size());
#endif
		pthread_cond_broadcast( &(index->fullCond) );

		/*
		// we will check the cancel condition when flow_table is empty
		if( totalRecordNum == 0 ) {
			pthread_testcancel();
		}
		*/;
		sleep( SCAN_INTERVAL );
	}
}

Exporter::Exporter( const string & name, Collector &c, DBHandle *db ) : collector(c){
	totalRecordNum = 0;
	for( int i = 0 ; i < BUCKETSIZE; i++ ) {
		hash_table[i] = NULL;
		packet_num[i] = pair<int,int>( 0 , i );
		pthread_mutex_init( &bucket_mutex[i] , NULL );
	}
	index = new Index( SRCIP|DSTIP );
	pthread_create( &scanThread, NULL, scan_thread_start, this);
	
	// initialization for exit-signal
	exit_signal = false;
	pthread_mutex_init( &signal_mutex, NULL );
}


Exporter::~Exporter() {
	for( int i = 0 ; i < BUCKETSIZE; ++i ) {
		pthread_mutex_destroy( &bucket_mutex[i] );
	}
	
	pthread_mutex_destroy( &signal_mutex );
	//pthread_cancel( scanThread );
	if( db_handle ) delete db_handle;
	if( index )  delete index;
}


int Exporter::write2db( const string& sql) {
		if( db_handle == NULL ) return -1;
#ifdef 	DEBUG
		std::cout<<sql<<"\n";
#endif
		// TODO: if there are huge records needed to insert into database, we should change the way to insert.
		return db_handle->write2db(sql);
}

/* push one record into exporter buffer and put it to one bucket according to its hash code */
int	Exporter::push_record( RECORD * rec ) {
	if( rec == NULL ) return -1;

	int bucketID = ( rec->hash() )%BUCKETSIZE;
	LinkNode * current = new LinkNode( rec );
	struct timeval tv;
	gettimeofday(&tv, NULL );

	if( current == NULL ) {
		fprintf( stderr, "can't allocate memory for LinkNode\n" );
		return -1;
	}

	pthread_mutex_lock( &bucket_mutex[ bucketID ] );

	packet_num[ bucketID ].first++;
	// TODO:thread synchronization
	if( hash_table[bucketID] == NULL ) {
		hash_table[bucketID] = current;
		current->tail = current;

#ifdef		HASH_PERF
		bucket_size[bucketID] = 1;
#endif
	} else {
		LinkNode  *ptr = NULL;

#ifdef		HASH_PERF
		++bucket_size[bucketID];
#endif

		bool flag = false;
		for( ptr = hash_table[bucketID] ; ptr ; ptr = ptr->nextlink ) {
			if( (*(ptr->data)).equals( rec ) ) {
				ptr->tail->nextdata = current;
				ptr->tail = current;
				flag = true;
				break;
			}
		}

		// maybe we should put this record to the beginning of list, somewhat like LRU algorithm
		LinkNode *tmp = hash_table[bucketID];
		if( !flag ) {
			// if this packet is the first packet of some flow
			current->tail = current;
			current->nextlink = tmp;
			hash_table[bucketID] = current;

			assert( current->tail->data != NULL );
		} else {
			// swap two linklist pointers, we can just swap the significant data it point to .

			assert( tmp->tail->data != NULL );
			assert( ptr->tail->data != NULL );

			Swap( tmp->data, ptr->data );
			/* some problems will happened here. when the link-list that pointer "ptr" or "tmp" point to
			   has only one element, then tmp->tail = tmp or ptr->tail = ptr.
			   The pointer variable "tail" means the last element of this link-list.
			   It will point wrong place when we just use Swap function to swap them.
			   */

			Swap( tmp->tail, ptr->tail );
			if( tmp->tail == ptr ) tmp->tail = tmp; // link-list that "ptr" point to has only one element.
			if( ptr->tail == tmp ) ptr->tail = ptr; // link-list that "tmp" point to has only one element.

			Swap( tmp->nextdata, ptr->nextdata );
			assert( tmp->tail->data != NULL );
			assert( ptr->tail->data != NULL );

		}
	}

	++totalRecordNum;
	/*
	   if( totalRecordNum > HASH_PACKETS_UPBOUNDER ) {
	   export_all();
	   }
	   */
	pthread_mutex_unlock( &bucket_mutex[bucketID] );

	return 0;
}


/* export all the similar packets in one chain, this chain is not the bucket head-pointer that point to but
 * a set of all packets that have the same record( some fields of packet header ).
 */
void 	Exporter::export_chain( LinkNode * chain, FileWriter * file_handle ) {
	if( chain == NULL ) return ;

	uint32_t fileOffset = 0; 
	LinkNode *ptr = chain;

	while( chain != NULL ) {
		chain = ptr->nextdata;
		assert( ptr->data != NULL );
	
		if( file_handle ) {
			// write packet to disk
			file_handle->write_file( ptr->data->get_packet_buffer(), ptr->data->get_packet_length() );
		}
		
		// the end of this chain
		if( chain == NULL ) {
			ptr->data->set_file_offset( fileOffset );
			collector.set_resource_free( ptr->data->get_packet_pointer());
			--totalRecordNum;
			(index->linkNodePool).push_resource( ptr->data );
			delete ptr;
			return ;
		}

		//set the resource "ptr->data->packet" free
		collector.set_resource_free( ptr->data->get_packet_pointer());
#ifdef DEBUG
		//std::cout<<ptr->data->get_sql_string()<<"\n";
#endif
		delete ptr->data;
		delete ptr;
		--totalRecordNum;
		ptr = chain;
	}

	// write this record to database.
	//if( sql != "" ) write2db( sql );
}

void  Exporter::export_timeout_flows(int timeout) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	FileWriter* filewriter = NULL;

	// scan the hash table to export record and free resource
	for( int i = 0 ; i < BUCKETSIZE; ++i ) {
		LinkNode *ptr = hash_table[i] , *pre = NULL;
		int pre_record_num = totalRecordNum;

		pthread_mutex_lock( &bucket_mutex[i] );
		while( ptr != NULL ) {
			// if the time period between now and the last packet of this flow arrived is more than
			// FLOW_TIME_OUT, we should export this record.

			assert( ptr->tail != NULL );
			if( ptr->tail->data == NULL ) {
				printf("address = %p\n",ptr->tail );
				for( LinkNode *iter = ptr; iter ; iter = iter->nextdata ) {
					printf("%p ",iter);
					iter->data->display();
				}
			}
			assert( ptr->tail->data != NULL);

			//if( tv.tv_sec - ptr->tail->data->get_time_second() >= FLOW_TIMEOUT ) {
			if( tv.tv_sec - ptr->data->get_time_second() >= timeout ) {
				if( ptr == hash_table[i] ) {
					hash_table[i] = ptr->nextlink;
				} else {
					assert( pre != NULL );
					pre->nextlink = ptr->nextlink;
				}

				LinkNode *next = ptr->nextlink;
#ifdef	WRITE_PACKETS
				if( filewriter == NULL ) {
					filewriter = new FileWriter( FileWriter::assign_file_name() );
				} else {
					if( filewriter->get_packet_num() >= MAX_FILE_PACKETS ) {
						delete filewriter;
						filewriter = new FileWriter( FileWriter::assign_file_name());
					}
				}
#endif
				export_chain( ptr, filewriter );
				ptr = next;
			} else {
				pre = ptr;
				ptr = ptr->nextlink;
			}
		}

		packet_num[i].first -= (pre_record_num-totalRecordNum);
		pthread_mutex_unlock( &bucket_mutex[i]);
	}

	if( filewriter != NULL ) {
		delete filewriter;
		filewriter = NULL;
	}
}

void  Exporter::export_longer_chains() {
	// if the rest free resource of the packets pool is still very little, we should export
	// the longer chains to make sure enough free resource to receive packets from device.
	sort( packet_num, packet_num+BUCKETSIZE );
	int id = 0 , low_bounder = totalRecordNum/4;
	FileWriter * filewriter = NULL;

	while( totalRecordNum > low_bounder && id < BUCKETSIZE ) {
		pthread_mutex_lock( &bucket_mutex[ packet_num[id].second ] );
		LinkNode *next = NULL ;
		/*
		for( LinkNode *ptr=hash_table[ packet_num[id].second ]; ptr; ptr = next ) {
			if( filewriter == NULL ) {
				filewriter = new FileWriter( FileWriter::assign_file_name() );
			} else {
				if( filewriter->get_packet_num() >= MAX_FILE_PACKETS ) {
					delete filewriter;
					filewriter = new FileWriter( FileWriter::assign_file_name());
				}
			}

			next = ptr->nextlink;
			export_chain( ptr, filewriter );
		}
		*/
		hash_table[ packet_num[id].second ] = NULL;
		packet_num[id].first = 0;
		pthread_mutex_unlock( &bucket_mutex[ packet_num[id].second ] );
		id++;
	}
	delete filewriter;
}
