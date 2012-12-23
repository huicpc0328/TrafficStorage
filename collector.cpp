#include "collector.h"
#include <cstdio>
#include <cstring>
#include <assert.h>
#include "exporter.h"

extern 	Exporter ipv4_exporter;
//extern	Exporter ipv6_exporter;

Collector::Collector( const char *uri ):trace( NULL ) {
	strncpy( inputURI, uri, sizeof(inputURI) );
	while( !pkt_queue.empty() ) pkt_queue.pop();

	// initialize all the packet object in pool, this is very important
	int cap = pool.capacity();
#ifdef DEBUG
	printf("Capacity = %d\n",cap);
#endif
	for( int i = 0; i < cap; ++i ) {
		pool.set_resource_free( trace_create_packet() );
	}

	pthread_mutex_init( &packet_queue_mutex, NULL );
	pthread_mutex_init( &pool_resource_mutex, NULL );
}

Collector::~Collector() {
	if( NULL != trace ) {
		trace_destroy( trace );
	}
	PACKET * packet = NULL;
	// wait all the packets in queue to be processed.
	while( 1 ) {
		pthread_mutex_lock( &pool_resource_mutex );
		if( pool.size() == pool.capacity() ) break;
		printf("Pool size = %d capcity = %d\n", pool.size(), pool.capacity());
		pthread_mutex_unlock( &pool_resource_mutex );
		sleep(3);
	}
	while( (packet=pool.get_free_resource()) != NULL ) {
		trace_destroy_packet( packet );
	}
	/*
	while( (packet=get_packet()) != NULL ) {
		trace_destroy_packet( packet );
	}
	*/

	pthread_mutex_destroy( &packet_queue_mutex );
	pthread_mutex_destroy( &pool_resource_mutex );
}

// must be called at the program beginning.
void Collector::open_trace( char * uri ) {
	trace = trace_create( uri );
	
	if( trace_is_err( trace ) ) {
		trace_perror( trace, "creating trace is error");
		trace_destroy( trace );
		trace = NULL;
		return ;
	}

	if( -1 == trace_start( trace ) ) {
		trace_perror( trace, "opening trace is error");
		trace_destroy( trace );
		trace = NULL;
		return ;
	}
	/*
	for( int i = 0 ; i < QUEUE_MAX_PACKETS; ++i ) {
		pkt_queue[i] = trace_create_packet();
	}
	*/
}

// -1 means that read packet failed, 0 successed.
int Collector::read_packet( PACKET * packet ) {
	if( NULL == trace ) return -1;
	
	return trace_read_packet( trace, packet ) > 0 ? 0 : -1;
}

PACKET* Collector::get_packet() {
	pthread_mutex_lock( &packet_queue_mutex );
	if( pkt_queue.empty() ) {
		pthread_mutex_unlock( &packet_queue_mutex );
		return NULL;
	}
	PACKET *ptr = pkt_queue.front();
	pkt_queue.pop();
	pthread_mutex_unlock( &packet_queue_mutex );
	return ptr;
}

void Collector::set_resource_free( PACKET * ptr ) {
	pthread_mutex_lock( &pool_resource_mutex );
	pool.set_resource_free( ptr );
	pthread_mutex_unlock( &pool_resource_mutex );
}

void Collector::collect() {

	printf("Begin to collect packets!\n");
	
	open_trace( inputURI );
	assert( trace != NULL );

	if( NULL == trace ) {
		fprintf( stderr , "Cannot open the trace %s\n", inputURI );
		return ;
	}

#ifdef DEBUG
	printf("Create trace successfully!\n");
#endif

	PACKET *packet = NULL;
	uint32_t	pkt_num = 0;
	while( 1 ) {
		pthread_mutex_lock( &pool_resource_mutex );
		packet = pool.get_free_resource();
#ifdef DEBUG
		//if( pkt_num % 100 == 0 )
			//printf("pool size=%d, ipv4 Buffer=%d, ipv6 Buffer=%d\n",pool.size(),ipv4_exporter.size(),ipv6_exporter.size());
#endif
		pthread_mutex_unlock( &pool_resource_mutex );

		if( pool.size() < POOL_PACKETS_LOWBOUNDER ) {
			ipv4_exporter.export_timeout_flows();
			//ipv6_exporter.export_timeout_flows();

			if( pool.size() < POOL_PACKETS_LOWBOUNDER ) {
				ipv4_exporter.export_longer_chains();
				//ipv6_exporter.export_longer_chains();
			}
		}

		// TODO: we should signal the module exporter to store packets to hard disk and return the
		//	memory to receive next packets from device when no available free resource in pool.
		if( packet == NULL ) {
#ifdef DEBUG
			printf("No available free source exist!\n");
#endif
			continue;
		}

		if( -1 != read_packet( packet ) ) {
			++pkt_num;
			pthread_mutex_lock( &packet_queue_mutex );
			pkt_queue.push( packet );
			pthread_mutex_unlock( &packet_queue_mutex );
		} else {
			pool.set_resource_free( packet );
#ifdef DEBUG
			printf("Cannot read packet from trace!\n");
#endif
			break;
		}
		packet = NULL;
	}
}
