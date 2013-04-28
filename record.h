/* Base class of record.
 * Each record type such as ipv4, ipv6, arp should extend it and
 * implement the virtual function hash().
 *
 * Create by hth on 28/08/12.
 */

#ifndef RECORD_H_
#define	RECORD_H_

#include "global.h"
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <assert.h>

#define  BYTE1(x)	(uint8_t)(((x)&0xff000000)>>24)
#define  BYTE2(x)	(uint8_t)(((x)&0x00ff0000)>>16)
#define  BYTE3(x)	(uint8_t)(((x)&0x0000ff00)>>8)
#define  BYTE4(x)	(uint8_t)((x)&0x000000ff)

using libtrace::PACKET;

class Record {
protected:
	struct timeval tv;
	std::string	file_name;		/** name of file that stored all the packets contents, file type is pcap for default */
	uint32_t	file_offset;
	PACKET*		packet;
public:
	Record( PACKET* pkt );

	// packet need to be delete by who create
	virtual ~Record() {}
	virtual uint32_t hash() = 0;
	
	virtual void write2file(FILE*) {
	}

	virtual std::string get_sql_string() {
		return "";
	}

	virtual bool equals( Record * )= 0;

	virtual void display() = 0;

	inline	void set_file_offset(uint32_t offset ) {
		file_offset = offset;
	}

	inline  void inc_file_offset( uint32_t size ) {
		file_offset += size;
	}

	inline uint32_t get_file_offset() {
		return file_offset;
	}

	void set_file_name(std::string s);

	inline timeval get_time_tv() {
		return tv;
	}

	inline __time_t get_time_second() {
		return 	tv.tv_sec;
	}

	inline PACKET*	get_packet_pointer() {
		return packet;
	}

	inline uint16_t get_packet_length() {
		return trace_get_capture_length( packet );
	}

	inline void * get_packet_buffer(){
		uint32_t	remaining;
		libtrace::LINKTYPE linktype;
		return trace_get_packet_buffer( packet, &linktype, &remaining );
	}
};

class Ipv4Record: public Record {
private:
	struct in_addr 	srcip;		/** Source Address */
	struct in_addr 	dstip;		/** DST Address */
	uint8_t			proto;		/** protocal ID */
	uint16_t		srcport;	/** src port, 0 for icmp, igmp */
	uint16_t		dstport;	/** dst port, 0 for icmp, igmp */

public:
	Ipv4Record( PACKET* pkt );

	~Ipv4Record() {
	}

	inline uint32_t get_srcip() {
		return ntohl( srcip.s_addr );	
	}

	inline uint32_t get_dstip() {	
		return ntohl( dstip.s_addr );
	}

	inline uint16_t get_sport() {
		return ntohs( srcport );
	}

	inline uint16_t get_dport() {
		return ntohs( dstport );
	}
	
	inline uint16_t get_proto() {
		return proto;
	}

	uint32_t hash();

	bool equals( Record * );

	void write2file(FILE *fp);

	void display();

	bool operator == ( const Ipv4Record &rec ) const {
		return srcip.s_addr == rec.srcip.s_addr && \
				dstip.s_addr == rec.dstip.s_addr && \
				proto == rec.proto && \
				srcport == rec.srcport && dstport == rec.dstport;
	}

	friend std::ostream& operator << ( std::ostream &out, const Ipv4Record& rec ) {
		out<<	inet_ntoa(rec.srcip)<<"," ;
		flush(out);
		out<<	inet_ntoa(rec.dstip)<<"," \
			<<	rec.srcport<<","\
			<< 	rec.dstport<<","\
			<< 	(uint8_t)rec.proto;
		return out;
	}
};


class Ipv6Record: public Record {
private:
	struct in6_addr  srcip6;
	struct in6_addr  dstip6;
	uint8_t			proto;		/** next header or protocal ID */
	uint16_t		srcport;	/** src port, 0 for icmp, igmp */
	uint16_t		dstport;	/** dst port, 0 for icmp, igmp */

public:
	Ipv6Record( PACKET* pkt );

	~Ipv6Record() {
	}

	uint32_t hash();

	std::string get_sql_string();

	bool equals( Record * );

	void display();

	bool operator == ( const Ipv6Record &rec ) const {
		for( int i = 0 ; i < 4; ++i )
			if( srcip6.__in6_u.__u6_addr32[i] != rec.srcip6.__in6_u.__u6_addr32[i] )
				return false;

		for( int i = 0 ; i < 4;++i )
			if( dstip6.__in6_u.__u6_addr32[i] != rec.dstip6.__in6_u.__u6_addr32[i] )
				return false;

		return	proto == rec.proto && \
				srcport == rec.srcport && dstport == rec.dstport;
	}

	friend std::ostream& operator << ( std::ostream &out, const Ipv6Record& rec ) {
		char buf[128];
		inet_ntop( AF_INET6, (void*)&(rec.srcip6), buf, 128);
		out<< buf << "|";
		inet_ntop( AF_INET6, (void*)&(rec.dstip6), buf, 128);
		out<< buf << "|" \
			<<	rec.srcport<<"|"\
			<< 	rec.dstport<<"|"\
			<< 	(uint8_t)rec.proto;
		return out;
	}

};

#endif
