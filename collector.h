/* This module is responsible for collecting packets and putting them
 * into a buffer queue. And the next module will read packets from this queue,
 * then do next other process.
 *
 * If we changed the dependent library (like "libtrace"), just override the function of
 * open_trace and read_packet or create a new class to extend the class Collector
 * and implement all the virtual functions.
 *
 * create by hth on 24/08/12
 */


#ifndef 	COLLECTOR_H_
#define 	COLLECTOR_H_

#include "global.h"
#include "resourcePool.h"
#include <queue>

#define MAX_PACKETS	1024*1024
/* if the number of packets in hash table more than this macro, we should export some packets to hard disk */
#define		POOL_PACKETS_UPBOUNDER 	MAX_PACKETS*3/4
/* if the number of packets in hash table less than this macro, we can stop exporting packets to disk. */
#define		POOL_PACKETS_LOWBOUNDER	MAX_PACKETS*1/4

#define DEBUG

using namespace libtrace;

class Collector {
private:
	TRACE *					trace;      /* collector packets from this trace */
	std::queue<PACKET *>			pkt_queue;	/* store packets read from device that haven't been parsed */
	ResourcePool<PACKET*,MAX_PACKETS>	pool;		/* memory to store packets and its contents, we should returned
													the used memory to pool when we store one packet to hard disk*/
	char 							inputURI[128]; /* device name, e.g. int:eth0 */

	pthread_mutex_t						pool_resource_mutex; /* control the operation of ResoucePool */
	pthread_mutex_t						packet_queue_mutex;	 /* control the read and writer operation of pkt_queue*/

public:

	Collector( const char *uri = "int:eth0" );
	~Collector();
	virtual void 	open_trace( char * uri );		/* create trace object and initialize it */
	virtual int 	read_packet( PACKET * packet );	/* read packets from trace */
	void 			collect();						/* collector trace from device */
	PACKET * 		get_packet();					/* get a pointer of packet that haven't been parsed */
	void			set_resource_free(PACKET *ptr );/* @argument: pointer ptr must be an element of resource pool */
};


#endif
