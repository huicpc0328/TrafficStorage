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
		int8_t 	isLeaf;		/* flag to represent current Node is leaf or internal node */
		Bitmap  bitmap;     // the ith bit indicate the ith children is NULL or not
		uint8_t	elemCount;
		uint32_t*			fileOffArray; // stored the offset-array of file, used when write TrieNode into file.

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
template<typename VALUE>
class LeafTrieNode: TrieNode{
	private:

		LinkList<VALUE>*	resultArray[MAX_PER_NODE]; /* the matched result-list of each key */

	public:
		LeafTrieNode() : TrieNode( 1 ) {
			for( int i = 0 ; i < MAX_PER_NODE; ++i ) resultArray[i] = NULL;
		}

		~LeafTrieNode() {
			for( int i = 0 ; i < this->elemCount; ++i ) {
				assert( resultArray[i] != NULL );
				if( resultArray[i] ) 
					delete resultArray[i];
			}
		}

		int addItem( const KEY& pos, const VALUE& value ) {
			if( resultArray[pos] == NULL ) {
				resultArray[pos] = new LinkList<VALUE>();
				if( !resultArray[pos] ) {
					ERROR_INFO("cannot alloc memory for linklist", return -1);
				}
				resultArray[pos]->clear();
			}
			resultArray[pos]->addElement(value);
			++this->elemCount;
			return 0;
		}

		LinkList<VALUE>* getResult( const uint8_t idx ) {
			assert( idx >= 0 && idx <= MAX_PER_NODE );
			assert( resultArray[idx] != NULL );
			return resultArray[idx];
		}

		/* layout of storage in file
		 * |-------------------------|
		 * |   isLeaf	             |
		 * |   elemCount             |
		 * |   1st key 				 | 
		 * |   1st values' linklist  |
		 * |   2nd key    			 | 
		 * |   2nd values' linklist  |
		 * |   .........             |
		 * |   .........             |
		 * |   last key              |
		 * |   last values' linklist |
		 * |-------------------------|
		 */

		int write2file( FILE * fp) {
			if( !fp ) {
				ERROR_INFO("NULL file pointer fp",return -1);
			}

			fwrite( &(this->isLeaf), sizeof(this->isLeaf), 1, fp);
			this->bitmap.dump2file( fp );
			fwrite( &(this->elemCount), sizeof(this->elemCount), 1, fp);
			for( int i = 0 ; i < this->elemCount ; ++i ) {
				fwrite( &(this->keyArray[i]), sizeof( KEY ), 1, fp );
				if( resultArray[i]->write2file( fp ) == -1 ) {
					return -1;
				}
			}
			return 0;	
		}

		int readFromFile( FILE * fp) {
			if( !fp ) {
				ERROR_INFO("NULL file pointer fp",return -1);
			}
			//we have readed isLeaf value before we call this function
			//read( fd, &(this->isLeaf), sizeof(this->isLeaf));
			fread( &(this->elemCount), sizeof(this->elemCount), 1, fp);
			for( int i = 0 ; i < this->elemCount ; ++i ) {
				fread( &(this->keyArray[i]), sizeof( KEY ), 1, fp );
				assert( resultArray[i] == NULL );
				resultArray[i] = new LinkList<VALUE>();
				if( resultArray[i]->readFromFile( fp ) == -1 ) {
					return -1;
				}
			}
			return 0;
		}	

		// get the total size that we writed this LeafNode Object in file
		int32_t getSizeOfFile() {
			int32_t ret = sizeof( this->elemCount ) + sizeof( this->isLeaf);
			ret += this->elemCount * sizeof( KEY ) ;
			
			for( int i = 0 ; i < this->elemCount ; ++i ) {
				ret += resultArray[i]->getSizeOfFile();
			}
			return ret;
		}

};

#endif

