
#include "collector.h"
#include <stdio.h>
#include <unistd.h>

pthread_t thread1;

void * test( void * arg ) {
	Collector *ptr = (Collector *)arg;
	printf("Thread %d have been started!\n", (int)pthread_self());
	while( 1 ) {
		/*
		pthread_mutex_lock( &GlobalVar::collector_mutex );
		pthread_cond_wait( &GlobalVar::collector_cond, &GlobalVar::collector_mutex);
		*/
		PACKET *packet = ptr->get_packet();
		if( packet )
			printf("We have received this message !\n" );
		else printf("No packet now!\n");

		sleep(5);
		//getchar();

		//pthread_mutex_unlock( &GlobalVar::collector_mutex );
	}
}

int main() {
	Collector c ;
	pthread_create( &thread1, NULL, test, (void *)(&c) );
	printf("To collect!\n");
	c.collect();
	return 0;
}
