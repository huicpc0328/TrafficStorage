
#ifndef _TRIE_H
#define _TRIE_H

#include "trie_node.h"
#include "linklist.h"

template<typename VALUE>
class Trie {
	private:
		InterTrieNode   root;
		int32_t			nodeCount;
		#define		MASK	(MAX_PER_NODE-1)

	public:
		Trie() {
			root = new InterTrieNode();
			nodeCount = 1 ;
		}

		~Trie() {
			if( root ) {
				queue< TrieNode* > Q;
				Q.push( root );
				while( !Q.empty() ) {
					TrieNode* cur = Q.front();
					Q.pop();

					for( int i = 0 ; !cur->isLeafNode() && i < MAX_PER_NODE; ++i ) {
						if( cur->hasChildren(i) ) 
							Q.push( ((InternalNode*)cur)->getChildren(i) );
					}
					delete cur;
					--nodeCount;
				}

				assert( nodeCount == 0 );
			}
		}
		int 	addItem( const KEY& key, const VALUE& value);
		int		readFromFile( const char *name );
		int 	write2file( const char *name );
		LinkList<VALUE>* queryFromFile( const char *name, const KEY& begin, const KEY& end );
};

template<typename VALUE>
{
	int Trie<VALUE>::addItem( const KEY& key, const VALUE& value ) {
		KEY k = key;
		int num = (sizeof(KEY)<<3)/BITS_PER_NODE, rsnum = (num-1)*BITS_PER_NODE;
		
		assert( root != NULL );
		TrieNode* current = root;

		for(int i = 1; i < num; ++i ) {
			uint8_t tmp = (uint8_t)( (((k<<BITS_PER_NODE)>>BITS_PER_NODE)^k)>>rsnum );
			if( !current->hasChildren(tmp) ) {
				if( i != num-1 ) current->setChildren( tmp, new InterTrieNode() );
				else 			current->setChildren( tmp, new LeafTrieNode<VALUE>());
				current->setBit( tmp );
				++nodeCount;
			}
			current = current->getChildren(tmp);
		}
		uint8_t tmp = (uint8_t)( (((k<<BITS_PER_NODE)>>BITS_PER_NODE)^k)>>rsnum );
		current->addItem( tmp, value );
	}

	int Trie<VALUE>::write2file( const char *filename ) {

	}
}


#endif
