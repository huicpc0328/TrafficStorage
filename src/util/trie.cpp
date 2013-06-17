#include "trie.h"
#include <cstdio>
#include <queue>
using std::queue;

#ifdef	NODE_POOL
template<typename KEY>
ResourcePool<InterTrieNode*,POOL_MAX> Trie<KEY>::interNodePool;
template<typename KEY>
ResourcePool<LeafTrieNode*,POOL_MAX>  Trie<KEY>::leafNodePool;

// must call this function before we use trie tree for indexing in NODE_POOL mode
template<typename KEY>
void Trie<KEY>::initPool( uint32_t num ) {
	assert( num <= POOL_MAX );
	for( int i = 0; i < num; ++i ) {
		Trie<KEY>::interNodePool.push_resource( new InterTrieNode() );
		Trie<KEY>::leafNodePool.push_resource( new LeafTrieNode() );
	}
}

// must delete all memory of pool when trie node would not be used.
template<typename KEY>
void Trie<KEY>::clearPool() {
	TrieNode* nodePtr;
	while( nodePtr = Trie<KEY>::interNodePool.get_free_resource() ) {
		delete nodePtr;
	}
	while( nodePtr = Trie<KEY>::leafNodePool.get_free_resource() ) {
		delete nodePtr;
	}
}
#endif


template<typename KEY>
Trie<KEY>::~Trie() {
	if( root ) {
		printf("nodeCount = %d\n",nodeCount);
		queue< TrieNode* > Q;
		Q.push( root );
		while( !Q.empty() ) {
			TrieNode* cur = Q.front();
			Q.pop();

			for( int i = 0 ; !cur->isLeafNode() && i < cur->getElemCount(); ++i ) {
					Q.push( ((InterTrieNode*)cur)->getChildren( cur->getOnePosition(i)) );
			}

#ifdef	NODE_POOL
			cur->clear();
			if( cur->isLeafNode() ) {
				Trie<KEY>::leafNodePool.set_resource_free( (LeafTrieNode*)cur );
			} else {
				Trie<KEY>::interNodePool.set_resource_free( (InterTrieNode*)cur );
			}
#else
			delete cur;
#endif
			--nodeCount;
		}
		printf("finished trie tree destruction!\n");

		assert( nodeCount == 0 );
	}
}


template<typename KEY>
int Trie<KEY>::addItem( const KEY& key, const VALUE& value ) {
	int num = (sizeof(KEY)<<3)/BITS_PER_NODE, rsnum = (num-1)*BITS_PER_NODE;
	assert( root != NULL );
	TrieNode* current = root;
	KEY k = key;

	for(int i = 1; i < num; ++i ) {
		uint8_t tmp = (uint8_t)( (k>>rsnum)&0xff );
		if( !current->hasChildren(tmp) ) {
#ifndef	NODE_POOL
			if( i != num-1 ) ((InterTrieNode*)current)->addItem( tmp, new InterTrieNode());
			else 			((InterTrieNode*)current)->addItem( tmp, new LeafTrieNode());
#else
			if( i != num-1 ) ((InterTrieNode*)current)->addItem( tmp, Trie<KEY>::getInterNode());
			else 			((InterTrieNode*)current)->addItem( tmp, Trie<KEY>::getLeafNode());
#endif
			++nodeCount;
		}
		current = ((InterTrieNode*)current)->getChildren(tmp);
		assert( current != NULL );
		k <<= BITS_PER_NODE;
	}
	uint8_t tmp = (uint8_t)( (k>>rsnum)&0xff );
	((LeafTrieNode*)current)->addItem( tmp, value );
}

template<typename KEY>
int Trie<KEY>::write2file( const char *fileName ) {
	if( !root ) {
		ERROR_INFO("This is an empty Tree",return 0);
	}

	FILE *fileHandle = fopen( fileName, "wb");
	if( !fileHandle ) {
		ERROR_INFO("Open file failed",return -1);
	}
	// calculate the offsets of file in advance. 
	uint32_t  pos = root->getSizeOfFile();
	queue< TrieNode* >BQ;
	BQ.push( root );

	uint32_t* offsetArray = new uint32_t[ MAX_PER_NODE ];

	while( !BQ.empty() ) {

		TrieNode* cur = BQ.front(); BQ.pop(); 
		if( cur->isLeafNode() ) {
			cur->write2file( fileHandle );
			continue;
		}

		for( int i = 0 ; i < cur->getElemCount(); ++i ) {
			TrieNode* tmp = ((InterTrieNode*)cur)->getChildren( cur->getOnePosition(i) );
			BQ.push( tmp );
			offsetArray[i] = pos;
			pos += tmp->getSizeOfFile();
		}
		((InterTrieNode*)cur)->setFileOffset( offsetArray );
		cur->write2file( fileHandle );
	}
	fclose( fileHandle );
	delete[] offsetArray;
	return 0;
}


template<typename KEY>
//this function will only be used in queryFromFile
TrieNode* Trie<KEY>::readTrieNode( FILE* fp ) {
	int8_t  isLeafNode;

	// read a Node of trie tree
	if( fread( &isLeafNode, sizeof(int8_t), 1, fp ) != 1 ) {
		ERROR_INFO("Read file error", return NULL);
	}
	TrieNode* nodePtr = NULL;
	if( isLeafNode ) {
		nodePtr = new LeafTrieNode();
	} else {
		nodePtr = new InterTrieNode();
	}
	nodePtr->readFromFile( fp );
	return nodePtr;
}

template<typename KEY>
LinkList<VALUE>* Trie<KEY>::queryFromFile( const char *fileName, const KEY& begin, const KEY& end){
	FILE * fp = fopen( fileName , "rb" );
	if( !fp ) {
		fprintf( stderr, "Open file %s failed at file %s, in function %s, line %d\n", fileName, __FILE__, __func__, __LINE__);
		return NULL;
	}

	LinkList<VALUE>* ret = NULL, *leafRes = NULL;
	TrieNode*	 nodePtr = NULL, *nextPtr = NULL;
	uint32_t*	offsetArray = NULL, *linklistOff = NULL;
	uint8_t 	depth = (sizeof(KEY)<<3)/BITS_PER_NODE ;
	// bit number of left shift and right shift
	uint8_t rsnum = (depth-1)*BITS_PER_NODE;
	queue<TrieNode*> TQ[2];
	queue<uint32_t>	 prefix[2];
	uint32_t		prefixValue, currentQueue, currentValue;
	uint32_t		prefixBegin, prefixEnd;
	int pos = 0 ;

	// read root of trie tree
	if( (nodePtr = readTrieNode(fp)) == NULL) {
		ERROR_INFO("Read trie node error", goto ERROR);
	}

	TQ[1].push( nodePtr );
	prefix[1].push( 0 );
	ret = new LinkList<VALUE>();

	for( int i = 1; i < depth; ++i ) {
		currentQueue = (i&1);
		prefixBegin = begin>>rsnum;
		prefixEnd = end>>rsnum; 
		//prefixBegin = (((begin<<lsnum)>>lsnum)^begin)>>rsnum;
		//prefixEnd = (((end<<lsnum)>>lsnum)^end)>>rsnum;
		rsnum -= BITS_PER_NODE;

		if( TQ[currentQueue].empty() ) goto ERROR;

		while( !TQ[currentQueue].empty() ) {
			nodePtr = TQ[currentQueue].front(); 
			assert( !prefix[currentQueue].empty() );
			prefixValue = prefix[currentQueue].front()<<BITS_PER_NODE; 
			offsetArray = nodePtr->getFileOffset();
			assert( offsetArray != NULL );

			// TODO: find start position of prefixBegin-prefixValue by binary-search
			for( int j = 0 ; j < nodePtr->getElemCount(); ++j ) {
				currentValue = nodePtr->getOnePosition(j) + prefixValue;
				if( currentValue >= prefixBegin ) {
					if( currentValue > prefixEnd ) break;
					else {
						if( fseek( fp, offsetArray[j], SEEK_SET ) == -1 ) {
							ERROR_INFO("Seeking file position failed", goto ERROR);
						}
						if( (nextPtr=readTrieNode(fp)) == NULL ) {
							ERROR_INFO("Read trie node error", goto ERROR);
						}
						if( !nextPtr->isLeafNode() ){
							TQ[currentQueue^1].push( nextPtr );
							prefix[currentQueue^1].push( currentValue );
						} else {
							if( i != depth-1 ) printf(" i = %d, depth = %d\n",i, depth );
							assert( i == depth-1 );
							// now we need to process leaf trie nodes 
							linklistOff = nextPtr->getFileOffset();
							currentValue <<= BITS_PER_NODE; 
						    // TODO: using binary-search;	
							for( pos = 0 ; pos < nextPtr->getElemCount(); ++pos) {
								if( currentValue + nextPtr->getOnePosition(pos)\
										>= begin ) break;
							}
							if( fseek( fp, linklistOff[pos], SEEK_CUR ) == -1 ) {
								ERROR_INFO("Seeking file position failed", goto ERROR);
							}
							while( pos < nextPtr->getElemCount() ) {
								if( currentValue + nextPtr->getOnePosition(pos)\
										> end ) break;
								leafRes = new LinkList<VALUE>();
								leafRes->readFromFile( fp );
								ret->merge( *leafRes );
								//assert( leafRes->size() == 0);
								delete leafRes;
								leafRes = NULL;
								++pos;
							}
							delete nextPtr;
						}
					}
				}
			}
			TQ[currentQueue].pop();
			prefix[currentQueue].pop();
			delete nodePtr;
			offsetArray = NULL;
		}
	}

	fclose( fp );
	return ret;
ERROR:
	fclose( fp );
	for( int i = 0 ; i < 2; ++i ) {
		while( !TQ[i].empty() ) {
			delete TQ[i].front();
			TQ[i].pop();
		}
	}
	delete ret;
	return NULL;
}

template<typename KEY>
int Trie<KEY>::readFromFile( const char *fileName ) {
	FILE * fp = fopen( fileName, "rb");
	if( !fp ) {
		ERROR_INFO("open file failed", return -1);
	}

	int8_t  isLeafNode;
	size_t	isLeafSize = sizeof( isLeafNode );
	TrieNode* curNode = NULL, *newNode = NULL;
	queue< TrieNode *>TQ;

	if( fread( &isLeafNode, isLeafSize, 1, fp ) != 1 ) {
		ERROR_INFO("Read file error", goto ERROR);
	}
	// this tree must be an empty tree when we called this function readFromFile 
	assert( root != NULL );

	root->readFromFile( fp );
	if( !isLeafNode ) {
		TQ.push( root );
	}

	while( !TQ.empty() ) {
		curNode = TQ.front(); TQ.pop(); 

		for( int i = 0 ; i < curNode->getElemCount(); ++i ) {
			fread( &isLeafNode, isLeafSize, 1, fp );
#ifndef	NODE_POOL
			if( isLeafNode ) newNode = new LeafTrieNode();
			else	newNode = new InterTrieNode();
#else
			if( isLeafNode ) newNode = Trie<KEY>::getLeafNode(); 
			else	newNode = Trie<KEY>::getInterNode();
#endif
			nodeCount++;

			if( newNode->readFromFile( fp ) == -1 ) {
				ERROR_INFO("Something wrong in readFromFile", goto ERROR);
			}

			((InterTrieNode*)curNode)->setChildren( curNode->getOnePosition(i), newNode );
			if( !isLeafNode ) {
				TQ.push( newNode );
			} else {
				((LeafTrieNode*)newNode)->readLinklist( fp );	
			}
		}
	}
	fclose( fp );
	return 0;
ERROR:
	fclose( fp );
	return -1;
}
