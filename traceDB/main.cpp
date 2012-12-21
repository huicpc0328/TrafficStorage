#include "collector.h"
#include "exporter.h"
#include "parser.h"
#include <iostream>

using namespace std;

Collector *collector = new Collector("pcap:/host/trace/hth3.pcap");
Exporter ipv4_exporter("ipv4", *collector, NULL );
#define NUM 1

int main(int argc, char **argv) {
	Parser *parser[NUM];

	for( int i = 0 ; i < NUM; i++ ) {
		parser[i] = new Parser( *collector );
		parser[i]->start();
	}

	collector->collect();
	for( int i = 0 ; i < NUM; ++i ) {
		delete parser[i];
	}
	delete collector;
	return 0;
}
