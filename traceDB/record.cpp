#include "record.h"
#include <cstdio>

using libtrace::IPV4HEADER;
using libtrace::IPV6HEADER;

void Record::set_file_name(std::string s ){
	file_name = s;
}

void Ipv4Record::set_srcip() {
	IPV4HEADER *ipheader = trace_get_ip(packet);
	if( ipheader != NULL ) {
		srcip = ipheader->ip_src;
	}
}

void Ipv4Record::set_dstip() {
	IPV4HEADER *ipheader = trace_get_ip(packet);
	if( ipheader != NULL ) {
		dstip = ipheader->ip_dst;
	}
}

void Ipv4Record::set_srcport() {
	srcport = trace_get_source_port(packet);
}

void Ipv4Record::set_dstport() {
	dstport = trace_get_destination_port( packet );
}

void Ipv4Record::set_proto() {
	uint32_t remaining;
	void *tmp = trace_get_transport(packet, &proto, &remaining );
	if( tmp == NULL || remaining == 0 ) proto = 0;
}

uint32_t Ipv4Record::hash() {
	return srcip.s_addr ^ dstip.s_addr ^ srcport ^ dstport ^ proto;
}

/*
std::string Ipv4Record::get_sql_string() {
	char str[512];
	uint32_t src = ntohl(srcip.s_addr);
	uint32_t dst = ntohl(dstip.s_addr);

	snprintf(str, 512, "(%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s',%d,%d)",\
			tv.tv_sec,BYTE1(src),BYTE2(src),BYTE3(src),BYTE4(src),\
			BYTE1(dst),BYTE2(dst),BYTE3(dst),BYTE4(dst),\
			srcport, dstport, proto, file_name.c_str(), packets_begin, packets_end );
	return  (std::string)str;
}
*/

bool Ipv4Record::equals( Record *rec ) {
	return *this == *(Ipv4Record *)rec;
}

void Ipv4Record::display() {
	std::cout<<*this<<"\n";
}


void Ipv6Record::set_srcip() {
	IPV6HEADER* ipv6 = trace_get_ip6( packet );
	if( ipv6 != NULL ) {
		srcip6 = ipv6->ip_src;
	}
}

void Ipv6Record::set_dstip() {
	IPV6HEADER* ipv6 = trace_get_ip6( packet );
	if( ipv6 != NULL ) {
		dstip6 = ipv6->ip_dst;
	}
}

void Ipv6Record::set_proto() {
	IPV6HEADER* ipv6 = trace_get_ip6( packet );
	if( ipv6 != NULL ) {
		proto = ipv6->nxt;
	}
}

void Ipv6Record::set_srcport() {
	srcport = trace_get_source_port(packet);
}

void Ipv6Record::set_dstport() {
	dstport = trace_get_destination_port( packet );
}

uint32_t Ipv6Record::hash() {
	uint32_t ret = 0;
	for( int i = 0 ; i < 4; ++i ) {
		ret ^= srcip6.__in6_u.__u6_addr32[i];
		ret ^= dstip6.__in6_u.__u6_addr32[i];
	}
	return ret ^ srcport ^ dstport ^ proto;
}

bool Ipv6Record::equals( Record *rec ) {
	return *this == *(Ipv6Record *)rec;
}

void Ipv6Record::display() {
	std::cout<<*this<<"\n";
}


std::string Ipv6Record::get_sql_string() {
	/*
	char str[512];
	uint16_t *src = srcip6.__in6_u.__u6_addr16;
	uint16_t *dst = dstip6.__in6_u.__u6_addr16;

	snprintf(str, 512, "(%ld,%d,%d,%d,%d,%d,%d,%d,%d,\
			%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s',%d,%d)",\
			tv.tv_sec,src[0],src[1],src[2],src[3],src[4],src[5],src[6],src[7],\
			dst[0],dst[1],dst[2],dst[3],dst[4],dst[5],dst[6],dst[7],\
			srcport, dstport, proto, file_name.c_str(), packets_begin, packets_end );
	return  (std::string)str;
	*/
	return "";
}

