#include "collector.h"
#include "exporter.h"
#include "parser.h"
#include <iostream>

using namespace std;

//const char *uri = "int:wlan0";
const char *uri = "pcap:/home/hth/desktop/1.pcap";

Collector *collector = new Collector(uri);
Exporter ipv4_exporter("ipv4", *collector, NULL );
#define NUM 1

int main(int argc, char **argv) {
	Parser *parser[NUM];

	for( int i = 0 ; i < NUM; i++ ) {
		parser[i] = new Parser( *collector );
		parser[i]->start();
	}

	collector->collect();

	printf("We will cancel threads of parser!\n");
	// cancel the threads of parser
	for( int i = 0 ; i < NUM; ++i ) {
		pthread_t	pid = parser[i]->get_thread();
		pthread_cancel( pid );
		pthread_join( pid, NULL);
	}
	printf("canceling threads of parser finished!\n");

	for( int i = 0 ; i < NUM; ++i ) {
		delete parser[i];
	}

	printf("We will cancel threads of exporter!\n");
	// cancel all threads of exporter include indexing threads and scan thread
	// cancel the scan thread firstly
	pthread_t pid = ipv4_exporter.get_scan_thread();
	/*
	pthread_cancel( pid );
	*/
	ipv4_exporter.set_exit_signal();
	pthread_join( pid, NULL );
	printf("canceling threads of exporter finished!\n");
	
	vector<pthread_t> pvec = ipv4_exporter.get_index_threads();

	for( int i = 0; i < pvec.size(); ++i ) {
		pid = pvec[i];
		//pthread_cancel( pid );
		printf("wait thread %d to exit!\n", (int)pid );
		pthread_join( pid, NULL );
		printf("canceling one thread of indexing finished!\n");
	}

	delete collector;
	return 0;
}