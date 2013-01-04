/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-11-29 13:50
#
# Filename: trie_node.h
#
# Description: 
#
===========================================================================*/

#ifndef _TRIE_NODE_H
#define _TRIE_NODE_H

#include <errno.h>
#include <string.h>
#include <stdint.h>
#include "../global.h"
#include "bitmap.h"
#include "linklist.h"

#define BITS_PER_NODE 4
#define MAX_PER_NODE (1<<BITS_PER_NODE)

typedef uint8_t	KEY;

class TrieNode {
	protected:
		int8_t 		isLeaf;		/* flag to represent current Node is leaf or internal node */
		Bitmap  	bitmap;     // the ith bit indicate the ith children is NULL or not
		uint8_t		elemCount;
		uint32_t*	fileOffArray; // stored the offset-array of file, used when write TrieNode into file.

	public:
		TrieNode( int8_t  _leaf = 0 ): isLeaf( _leaf ) {
			bitmap = Bitmap(MAX_PER_NODE);
		}

		/* deconstructor function should be virtual */
		virtual ~TrieNode()  {
		}

		virtual int  write2file( FILE * fp )=0; 
		virtual int  readFromFile( FILE * fp )= 0;
		virtual int  getSizeOfFile() = 0; 

		inline int getElemCount() {
			return elemCount;
		}

		inline bool isLeafNode() {
			return isLeaf != 0;
		}

		inline bool hasChildren( uint8_t & pos ) {
			assert( pos >= 0 && pos < MAX_PER_NODE );
			return bitmap.getBit( pos );
		}

		inline void setBit( uint8_t& pos ) {
			bitmap.setBit( pos );
		}

		// we must call this function before we calling function write2file 
		inline void setFileOffset( uint32_t *ptr ) {
			assert( ptr != NULL );
			fileOffArray = ptr;
		}	
};

// implementation of internal btree node 
class InterTrieNode: public TrieNode{
	private:
		typedef TrieNode*	TrieNodePtr;
		TrieNodePtr 		childPtr[MAX_PER_NODE];

	public:
		InterTrieNode() : TrieNode(0) {
			fileOffArray = NULL;
		}

		~InterTrieNode() {
		}

		int addItem( const KEY& pos, TrieNodePtr child) ;

		int write2file( FILE *fp );

		int32_t getSizeOfFile();

		int readFromFile( FILE * fp);

		inline	TrieNodePtr getChildren( const KEY& idx ) {
			assert( idx >= 0 && idx < MAX_PER_NODE );
			return childPtr[ idx ];
		}

		inline void setChildren( const KEY& idx, TrieNodePtr ptr) {
			assert( idx >= 0 && idx < MAX_PER_NODE );
			childPtr[idx] = ptr;
		}

};


// implementation of leaf b-tree node 
typedef uint32_t VALUE;
class LeafTrieNode: public TrieNode{
	private:

		LinkList<VALUE>*	resultArray[MAX_PER_NODE]; /* the matched result-list of each key */

	public:
		LeafTrieNode(); 

		~LeafTrieNode();

		int addItem( const KEY& pos, const VALUE& value );

		LinkList<VALUE>* getResult( const uint8_t idx );

		int write2file( FILE * fp);

		int readFromFile( FILE * fp);
	
		// get the total size that we writed this LeafNode Object in file
		int32_t getSizeOfFile();

};

#endif

