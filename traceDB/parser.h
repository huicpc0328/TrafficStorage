/* This is the code of Module Parser.
 * It obtain packets from collector parallel, so we should pay attention on everywhere
 * that can lead to thread-safe or thread-synchronization issues.
 *
 * Create by hth on 26/08/12.
 */

#ifndef PARSER_H_
#define	PARSER_H_

#include "global.h"
#include "collector.h"
#include <pthread.h>


class Parser {

private:
	pthread_t  		workThread;  /* thread to process packet */
	Collector& 		collector;	/* because we will obtain PACKET object from collector */

public:
	Parser( Collector& c );
	Parser( const Parser& parser );
	~Parser();
	inline  pthread_t get_thread() {
		return workThread;
	}
	void 	start();        /* set up the thread */
	friend  void * thread_func( void * );
	void 	packet_process();
	void 	process_ethernet( PACKET *); /* process ethernet packets*/
};

#endif
