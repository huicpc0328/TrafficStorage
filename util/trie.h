
#ifndef _TRIE_H
#define _TRIE_H

#include "trie_node.h"
#include "linklist.h"

class Trie {
	private:
		InterTrieNode*   root;
		int32_t			nodeCount;
		typedef 	uint32_t VALUE;
		#define		MASK	(MAX_PER_NODE-1)

	public:
		Trie() {
			root = new InterTrieNode();
			nodeCount = 1 ;
		}

		~Trie();

		int 	addItem( const KEY& key, const VALUE& value);
		int		readFromFile( const char *name );
		int 	write2file( const char *name );
		LinkList<VALUE>* queryFromFile( const char *name, const KEY& begin, const KEY& end );
};


#endif
