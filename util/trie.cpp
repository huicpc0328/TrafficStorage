#include "trie.h"
#include <queue>

Trie::~Trie() {
	if( root ) {
		queue< TrieNode* > Q;
		Q.push( root );
		while( !Q.empty() ) {
			TrieNode* cur = Q.front();
			Q.pop();

			for( uint8_t i = 0 ; !cur->isLeafNode() && i < MAX_PER_NODE; ++i ) {
				if( cur->hasChildren(i) ) 
					Q.push( ((InterTrieNode*)cur)->getChildren(i) );
			}
			delete cur;
			--nodeCount;
		}

		assert( nodeCount == 0 );
	}
}


int Trie::addItem( const KEY& key, const VALUE& value ) {
	KEY k = key;
	int num = (sizeof(KEY)<<3)/BITS_PER_NODE, rsnum = (num-1)*BITS_PER_NODE;

	assert( root != NULL );
	TrieNode* current = root;

	for(int i = 1; i < num; ++i ) {
		uint8_t tmp = (uint8_t)( (((k<<BITS_PER_NODE)>>BITS_PER_NODE)^k)>>rsnum );
		if( !current->hasChildren(tmp) ) {
			if( i != num-1 ) ((InterTrieNode*)current)->setChildren( tmp, new InterTrieNode() );
			else 			((InterTrieNode*)current)->setChildren( tmp, new LeafTrieNode());
			current->setBit( tmp );
			++nodeCount;
		}
		current = ((InterTrieNode*)current)->getChildren(tmp);
	}
	uint8_t tmp = (uint8_t)( (((k<<BITS_PER_NODE)>>BITS_PER_NODE)^k)>>rsnum );
	((LeafTrieNode*)current)->addItem( tmp, value );
}

int Trie::write2file( const char *filename ) {

}


