#include "parser.h"
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <iostream>
#include "record.h"
#include "exporter.h"


extern 	Exporter ipv4_exporter;
/*
extern	Exporter<1024> arp_exporter;
extern	Exporter ipv6_exporter;
*/

Parser::Parser( Collector &c ): collector(c){}

Parser::Parser( const Parser& parser ): collector(parser.collector) {
	workThread = parser.workThread;
}

Parser::~Parser(){
	pthread_cancel( workThread );
}


void * thread_func( void * ptr) {
	Parser * parser = ( Parser * )ptr;
	parser->packet_process();
}

void Parser::start() {
	pthread_create( &workThread, NULL, thread_func, this);
}

void Parser::packet_process() {
/*
	int begin , end ;
	sscanf( (char *)arg, "%d %d", &begin, &end );
*/

#ifdef DEBUG
	printf("This thread will process packets\n");
#endif

	PACKET * packet;
	while( 1 ) {
		// we don't need to consider thread synchronization here because it have been processed in the function get_packet.
		packet = collector.get_packet();
		if( packet == NULL ) continue;

		LINKTYPE linktype;
		uint32_t remaining;

#ifdef DEBUG
#endif
		void * l2 = trace_get_layer2( packet, &linktype, &remaining );

		if( l2 == NULL || remaining == 0 ) {
			collector.set_resource_free( packet );
			continue;
		}

		bool processed = false;
		switch( linktype ) {
			case TRACE_TYPE_HDLC_POS:
				break; /**< HDLC over POS */
			case TRACE_TYPE_ETH:
				process_ethernet( packet );
				processed = true;
				break; /**< 802.3 style Ethernet */
			case TRACE_TYPE_ATM:		/**< ATM frame */
				break;
			case TRACE_TYPE_80211:	/**< 802.11 frames */
				break;
			case TRACE_TYPE_NONE:	/**< Raw IP frames */
				break;
			case TRACE_TYPE_LINUX_SLL:	/**< Linux "null" framing */
				break;
			case TRACE_TYPE_PFLOG:	/**< FreeBSD's PFlog */
				break;
			case TRACE_TYPE_80211_PRISM: /**< 802.11 Prism frames */
				break;
			case TRACE_TYPE_AAL5: /**< ATM Adaptation Layer 5 frames */
				break;
			case TRACE_TYPE_DUCK:  /**< Pseudo link layer for DUCK packets */
				break;
			case TRACE_TYPE_80211_RADIO:  /**< Radiotap + 802.11 */
				break;
			case TRACE_TYPE_LLCSNAP:    /**< Raw LLC/SNAP */
				break;
			case TRACE_TYPE_PPP:	     /**< PPP frames */
				break;
			case TRACE_TYPE_METADATA:    	/**< WDCAP-style meta-data */
				break;
			default:
				break;
		}
		if( !processed ) {
			collector.set_resource_free( packet );
		}
	}
}


void Parser::process_ethernet( PACKET * packet ) {
	uint16_t ethertype;
	uint32_t remaining;
	void *l3 = trace_get_layer3( packet , &ethertype, &remaining );

	if( l3 == NULL || remaining == 0 ) {
		collector.set_resource_free( packet );
		return ;
	}

	bool processed = false;

	switch( ethertype ) {
		case TRACE_ETHERTYPE_8021Q:
			break;
		case TRACE_ETHERTYPE_ARP:
			break;
		case TRACE_ETHERTYPE_IP: {
			Ipv4Record * rec = new Ipv4Record(packet);
			if( rec == NULL ) {
				printf("can't allocate memory for Ipv4Record\n");
			} else {
				ipv4_exporter.push_record( rec );
				processed = true;
			}
		}
			break;
		case TRACE_ETHERTYPE_IPV6: {
			/*
			Ipv6Record * rec = new Ipv6Record(packet);
			if( rec == NULL ) {
				printf("can't allocate memory for ipv6Record\n");
			} else {
				ipv6_exporter.push_record( rec );
				//rec->display();
				processed = true;
			}
			*/
		}
			break;
		case TRACE_ETHERTYPE_MPLS:
			break;
		case TRACE_ETHERTYPE_MPLS_MC:
			break;
		case TRACE_ETHERTYPE_PPP_DISC:
			break;
		case TRACE_ETHERTYPE_PPP_SES:
			break;
		case TRACE_ETHERTYPE_RARP:
			break;
		default:
			break;
	}

	if( !processed ) {
		collector.set_resource_free( packet );
	}
}

/*
void Parser::process_ipv4( PACKET * packet ) {
	uint8_t		proto;
	uint32_t	remaining;

	void * translayer = trace_get_transport(packet,&proto,&remaining);

	if( translayer == NULL || remaining == 0 ) return ;

	switch( proto ) {
		case	TRACE_IPPROTO_AH:
				break;
		case 	TRACE_IPPROTO_DSTOPTS:
				break;
		case	TRACE_IPPROTO_ESP:
				break;
		case	TRACE_IPPROTO_FRAGMENT:
				break;
		case	TRACE_IPPROTO_GRE:
				break;
		case	TRACE_IPPROTO_ICMP:
				break;
		case	TRACE_IPPROTO_ICMPV6:
				break;
		case	TRACE_IPPROTO_IGMP:
				break;
		case	TRACE_IPPROTO_IP:
				break;
		case	TRACE_IPPROTO_IPIP:
				break;
		case	TRACE_IPPROTO_NONE:
				break;
		case	TRACE_IPPROTO_OSPF:
				break;
		case 	TRACE_IPPROTO_PIM:
				break;
		case	TRACE_IPPROTO_ROUTING:
				break;
		case	TRACE_IPPROTO_RSVP:
				break;
		case	TRACE_IPPROTO_SCTP:
				break;
		case	TRACE_IPPROTO_TCP:
				break;
		case	TRACE_IPPROTO_UDP:
				break;
		default: break;
	}
}
*/

