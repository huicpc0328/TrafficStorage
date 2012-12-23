/*==========================================================================
#
# Author: hetaihua - hetaihua@ict.ac.cn
#
# Last modified: 2012-11-28 09:26
#
# Filename: linklist.h
#
# Description: 
#
===========================================================================*/

#ifndef _LINKLIST_H
#define _LINKLIST_H

#include <cstdio>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#ifdef DEBUG
#include <iostream>
using namespace std;
#endif

// T must be a type that can use operation sizeof to get its size.
template<typename T>
class LinkList {
	private:
		class LinkNode {
			public:
				T data;
				LinkNode *next;

				LinkNode(){
					next = NULL;
				}

				LinkNode(T _data): data(_data), next( NULL ) {
				}

				~LinkNode() {
					next = NULL;
				}
		};

		LinkNode* 	head;
		LinkNode* 	tail;
		uint32_t	nodeCount;

	public:
		
		LinkList() {
			head = tail = NULL;
			nodeCount = 0;
		}

		~LinkList() {
			LinkNode *pre = NULL;
			for( LinkNode* ptr = head; ptr != NULL; ) {
				pre = ptr->next;
				delete ptr;
				ptr = pre;
				nodeCount --;
			}

			if( nodeCount != 0 ) {
				fprintf( stderr, "There are Something wrong in LinkList at file %s, line %d\n", __FILE__, __LINE__);
			}
		}

		// initialization of linklist
		void clear() {
			head = tail = NULL;
			nodeCount = 0;
		}

		// -1 for failed, 0 for successed.
		int 	addElement( const T& data ) {
			if( head == NULL ) {
				head = tail = new LinkNode( data );
				if( !head ) {
					fprintf( stderr, "Cannot alloc memory for LinkNode at file %s, line %d\n", __FILE__ , __LINE__ );
					return -1;
				}
			} else {
				tail = tail->next = new LinkNode( data );
				if( !tail ) {
					fprintf( stderr, "Cannot alloc memory for LinkNode at file %s, line %d\n", __FILE__ , __LINE__ );
					return -1;
				}
			}
			nodeCount++;
			return 0;
		}

		inline uint32_t size() {
			return nodeCount;
		}

		inline bool empty() {
			return head == NULL;
		}

		// elements need be ascending ordered after adding elements of another list
		int 	merge( LinkList<T>& list ) {
			if( list.empty() ) {
				return 0;
			}
			
			LinkNode *cur = list.begin() ;
			if( head == NULL || cur->data < head->data ) {
				swap( cur, head );
			}
			LinkNode *pre = NULL , *ptr = head;

			assert( ptr != NULL );
			// TODO: we nned update tail pointer of current linklist
			while( 1 ) {
				// find a position to insert this element that pointer cur point to.
				if( NULL == cur ) break;
				while( ptr->next &&  ptr->next->data < cur->data ) ptr = ptr->next;
				if( NULL == ptr->next ){
					ptr->next = cur;
					break;
				}
				// insert element that cur points to 
				pre = cur->next;
				cur->next = ptr->next;
				ptr->next = cur;
				cur = pre;
			}

			nodeCount += list.size();
			// clear the list because all nodes had been transfered into current one 
			list.clear();
			return 0;
		}

		inline LinkNode* begin() {
			return head;
		}

		// TODO: we can use skip-list to accelerate the speed of merging operation when the 
		// querying speed is too slow to tolerate 
		// the elements of two LinkLists must be ascending ordered.
		// only the common elements in two linklist will be reserved in the linklist
		void	intersection( const LinkList& list ) {
			LinkNode *pre = NULL , *cur = list.begin(), *temp = NULL;
			LinkNode *ptr = head ;
			head = NULL;
			while( ptr != NULL ) {
				while( cur && ptr->data > cur->data ) cur = cur->next;

				if( !cur ) break;
				if( cur->data == ptr->data ) {
					// modify the LinkList's head after merging
					if( NULL == pre ) pre = head = cur;
					else pre = pre->next = cur;
				} else {
					temp = ptr->next;
					delete ptr;
					ptr = temp;
				}
			}
		}

		/* layout of storage in file
		 * |-------------------------|
		 * |   nodeCount             |
		 * |   1st node's data       |
		 * |   2nd node's data       |
		 * |    ...........          |
		 * |   last node's data      |
		 * |-------------------------|
		 */
		int 	write2file( int fd, int32_t offset = 0 ) {
			// we will seek from current position defaultly.
		    assert( fd >= 0 );	
			off_t pos = lseek( fd, offset, SEEK_CUR );
			if( pos < 0 ) {
				fprintf( stderr, "reseeking file handler position failed at file %s, line %d\n", __FILE__,__LINE__);
				return -1;
			}

			write( fd, &nodeCount, sizeof( nodeCount ) ) ;

			int elemSize = sizeof( T );
			T * array = new T[nodeCount];
			int i = 0;
			// copy elements to an array and then use only once system_call "write" to write data into file
			for( LinkNode *ptr = head ; ptr ; ptr = ptr->next ) {
				*( array + i++ ) = ptr->data;
			}

			assert( i == nodeCount );
			write( fd, array, elemSize * nodeCount );
			delete[] array;
			return 0;
		}

		// get the total size that we wrote this Linklist object in file
		int32_t	getSizeOfFile() {
			return sizeof( nodeCount ) + nodeCount * sizeof(T);
		}

		// read data from file and load it to current LinkList object.
		int		readFromFile( int fd, int32_t offset = 0 ) {
		    assert( fd >= 0 );	
			// we will seek from current position defaultly.
			off_t pos = lseek( fd, offset, SEEK_CUR );
			if( pos < 0 ) {
				fprintf( stderr, "reseeking file handler position failed at file %s, line %d\n", __FILE__,__LINE__);
				return -1;
			}

			read( fd, &nodeCount, sizeof( nodeCount ));

			int elemSize = sizeof( T );
			T * array = new T[nodeCount];
			read( fd, array, elemSize * nodeCount );

			int i = 0;
			while( i < nodeCount ) {
				if( i == 0 ) head = tail = new LinkNode( array[i] );
				else tail = tail->next = new LinkNode( array[i] );
				++i;
			}

			delete[] array;
			return 0;
		}

		// file IO
		int 	write2file( FILE * fp ) {
			if( !fp ) {
				fprintf( stderr, "NULL pointer fp in file %s at line %d\n", __FILE__, __LINE__ );
				return -1;
			}

			fwrite( &nodeCount, sizeof( nodeCount ), 1 , fp ) ;

			int elemSize = sizeof( T );
			for( LinkNode *ptr = head ; ptr ; ptr = ptr->next ) {
				fwrite( &(ptr->data), elemSize , 1 , fp );
			}

			return 0;
		}
	
		// read data from file and load it to current LinkList object.
		int		readFromFile( FILE *fp ) {
			if( !fp ) {
				fprintf( stderr, "NULL pointer fp in file %s at line %d\n", __FILE__, __LINE__ );
				return -1;
			}

			fread( &nodeCount, sizeof( nodeCount ), 1, fp);

			int elemSize = sizeof( T );
			T * array = new T[nodeCount];
			fread( array, elemSize * nodeCount, 1, fp );

			int i = 0;
			while( i < nodeCount ) {
				if( i == 0 ) head = tail = new LinkNode( array[i] );
				else tail = tail->next = new LinkNode( array[i] );
				++i;
			}

			delete[] array;
			return 0;
		}

#ifdef DEBUG
		// print detailed Information of linklist for debug
		void outInfo() {
			cout<<"Informations of Linklist, has "<<nodeCount<<" elements"<<endl;
			for( LinkNode* ptr = head; ptr ; ptr = ptr->next ) {
				cout<< ptr->data <<" ";
			}
			cout<<endl;
		}
#endif
};

#endif
