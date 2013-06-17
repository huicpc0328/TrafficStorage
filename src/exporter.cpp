#include "exporter.h"
#include "fileManager.h"
#include <sstream>

#ifdef DEBUG
#include <stdio.h>
#endif

using std::stringstream;

const string DATA_FILE_SUFFIX=".dat";

static void * scan_thread_start( void *arg ) {
	int state = PTHREAD_CANCEL_DEFERRED, oldstate ;
	pthread_setcanceltype( state, &oldstate );
	((Exporter*)arg)->scan_hash_table();
}

void Exporter::scan_hash_table() {
	
	printf("Scan Thread %d has started!\n", (uint32_t)pthread_self());
	stringstream stream;
	string dir = FileManager::get_directory_name(), fileName;
	uint16_t	fileID = 0;

	while( 1 ) {
		pthread_mutex_lock( &(index->queueLock));
		while( index->readCode != 0 ) {
			pthread_cond_wait( &(index->emptyCond), &(index->queueLock) );
		}

		index->clear_linkNodePool();

		// check file size <= 4G
		//if( file_is_full || this->get_exit_signal() ) {
		if( file_is_full ) {
			index->write_index_to_file( fileID );
			fileID = FileManager::apply_file_ID();
			stream.clear();
			stream<<dir<<fileID<<DATA_FILE_SUFFIX;
			stream>>fileName;
			fileWriter->reset_file(fileName);
			// reset file_is_full 
			file_is_full = false;
		}

		// check receiving exit signal or not
		// did this examination comsume much cpu resource ?
		if( this->get_exit_signal() ) {
			// export all records in hash table when received exit signal;
#if 1
			export_timeout_flows( 0 );
			index->readCode = (index->indexFlag);
			pthread_mutex_unlock( &(index->queueLock) );
#ifdef DEBUG
			printf("EXIT:%d packets need to be export!\n", index->linkNodePool.size());
#endif
			pthread_cond_broadcast( &(index->fullCond) );
			// wait for all index threads to finish their inserting operations.
			pthread_mutex_lock( &(index->queueLock));
			while( index->readCode != 0 ) {
				pthread_cond_wait( &(index->emptyCond), &(index->queueLock) );
			}
			index->clear_linkNodePool();
#endif
			index->readCode = index->indexFlag;
			index->write_index_to_file( fileID );
			this->index->set_exit_signal();	
			pthread_mutex_unlock( &(index->queueLock) );
			/*
			for( int i = 0 ; i < index->threadVec.size(); ++i ) {
				pthread_cancel( index->threadVec[i] );
			}
			*/
			pthread_cond_broadcast( &(index->fullCond) );
			delete fileWriter;
			printf("thread scan_hash_table exited\n");
#ifdef HASH_PERF
			hash_collision.print_stat();
#endif
			pthread_exit(0);
		}

		printf("start exporting timeout flows, exporter size = %d\n", totalRecordNum);
		if( totalRecordNum > POOL_PACKETS_UPBOUNDER ) 
			export_timeout_flows( 5 );
		else export_timeout_flows();
		index->readCode = (index->indexFlag);
		pthread_mutex_unlock( &(index->queueLock) );
#ifdef DEBUG
		printf("%d packets need to be export!\n", index->linkNodePool.size());
#endif
		fflush( stdout );
		pthread_cond_broadcast( &(index->fullCond) );

		/*
		// we will check the cancel condition when flow_table is empty
		if( totalRecordNum == 0 ) {
			pthread_testcancel();
		}
		*/
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
	//index = new Index( SRCIP|DSTIP );
	index = new Index( SRCIP|DSTIP|SPORT|DPORT );

	// set the attribute of scanning thread.
	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	param.sched_priority = 20;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create( &scanThread, &attr, scan_thread_start, this);
	pthread_attr_destroy(&attr);

	// initialization for exit-signal
	exit_signal = false;
	pthread_mutex_init( &signal_mutex, NULL );

	uint16_t fileID = FileManager::apply_file_ID(); 
	stringstream stream;
	string dir = FileManager::get_directory_name(), fileName;
	stream<<dir<<fileID<<DATA_FILE_SUFFIX;
	stream>>fileName;
	fileWriter = new FileWriter(fileName );
	if( !fileWriter ) {
		ERROR_INFO("cannot create FileWriter\n",pthread_exit(0));
	};
	file_is_full = false;
	
#ifdef HASH_PERF
	hash_collision = PerfMeasure( "hash_collision" );
#endif
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
	// TODO: need to add a mutex for it?
	last_packet_timeval = rec->get_time_tv();

	if( current == NULL ) {
		ERROR_INFO("can't allocate memory for LinkNode\n",return -1 );
	}

	pthread_mutex_lock( &bucket_mutex[ bucketID ] );
	uint16_t	pktLen = rec->get_packet_length();
	packet_num[ bucketID ].first++;
	if( hash_table[bucketID] == NULL ) {
		hash_table[bucketID] = current;
		current->tail = current;

#ifdef		HASH_PERF
		bucket_size[bucketID] = 1;
		hash_collision.addRecord(0);
#endif
	} else {
		LinkNode  *ptr = NULL;

		bool flag = false;
		int  step = 1;
		for( ptr = hash_table[bucketID] ; ptr ; ptr = ptr->nextlink, ++step ) {
			if( ptr->data->equals( rec ) ) {
				if( ptr->data->get_file_offset() + pktLen <= CUTOFF ) {
					ptr->tail->nextdata = current;
					ptr->tail = current;
					ptr->data->inc_file_offset( pktLen );
					flag = true;
					break;
				} else {
					// this packet is cutoff
					collector.set_resource_free(rec->get_packet_pointer());
					pthread_mutex_unlock( &bucket_mutex[bucketID] );
					return 0;		
				}
			}
		}

#ifdef		HASH_PERF
		++bucket_size[bucketID];
		hash_collision.addRecord( step );
#endif

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
	FILE *fp = fopen("flows.txt","ab+");
	ptr->data->write2file( fp );
	fclose(fp);

	while( chain != NULL ) {
		chain = ptr->nextdata;
		assert( ptr->data != NULL );

#ifdef	WRITE_PACKETS
		if( file_handle ) {
			// write packet to disk
			fileOffset = file_handle->get_file_size();
			file_handle->write_file( ptr->data->get_packet_buffer(), ptr->data->get_packet_length() );
		}
#endif
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
	struct timeval tv = last_packet_timeval;
	//gettimeofday(&tv, NULL);

	// scan the hash table to export record and free resource
	for( int i = 0 ; i < BUCKETSIZE && !file_is_full; ++i ) {
		pthread_mutex_lock( &bucket_mutex[i] );

		LinkNode *ptr = hash_table[i] , *pre = NULL;
		int pre_record_num = totalRecordNum;

		while( ptr != NULL ) {
			// if the time period between now and the last packet of this flow arrived is more than
			// FLOW_TIME_OUT, we should export this record.

#if 0
			assert( ptr->tail != NULL );
			if( ptr->tail->data == NULL ) {
				printf("address = %p\n",ptr->tail );
				for( LinkNode *iter = ptr; iter ; iter = iter->nextdata ) {
					printf("%p ",iter);
					iter->data->display();
				}
			}
			assert( ptr->tail->data != NULL);
#endif

			//if( tv.tv_sec - ptr->tail->data->get_time_second() >= FLOW_TIMEOUT ) {
			if( tv.tv_sec - ptr->data->get_time_second() >= timeout ) {
				// current file could store all packets of this flow or not ?
				if( fileWriter->get_file_size() + ptr->data->get_file_offset() > MAX_FILE_SIZE ) {
					file_is_full = true;
					break;
				}

				if( ptr == hash_table[i] ) {
					hash_table[i] = ptr->nextlink;
				} else {
					/*
					if( pre == NULL ) {
						printf("ptr = %p, hash_table[i] = %p\n", ptr, hash_table[i] );
						for( LinkNode *iter = hash_table[i]; iter ; iter = iter->nextlink) {
							printf("%p ",iter);
						}
					}
					*/
					assert( pre != NULL );
					pre->nextlink = ptr->nextlink;
				}

				LinkNode *next = ptr->nextlink;
				if( pre == NULL ) assert( next == hash_table[i]);
				export_chain( ptr, fileWriter );
				ptr = next;
			} else {
				pre = ptr;
				assert( ptr != NULL );
				ptr = ptr->nextlink;
			}
		}

		packet_num[i].first -= (pre_record_num-totalRecordNum);
		pthread_mutex_unlock( &bucket_mutex[i]);
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
