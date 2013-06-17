
#ifndef _TRIE_H
#define _TRIE_H

#include "trie_node.h"
#include "linklist.h"
#include "../resourcePool.h"

#define POOL_MAX 500000

template<typename KEY>
class Trie {
	private:
		TrieNode*   root;
		uint32_t	nodeCount;
		typedef 	uint32_t VALUE;
		#define		MASK	(MAX_PER_NODE-1)
#ifdef	NODE_POOL
		static		ResourcePool<InterTrieNode*,POOL_MAX> interNodePool;
		static		ResourcePool<LeafTrieNode*,POOL_MAX>  leafNodePool; 
#endif

	public:

#ifdef	NODE_POOL
		static void initPool( uint32_t );

		static void clearPool();

		static inline InterTrieNode* getInterNode() {
			return Trie<KEY>::interNodePool.get_free_resource();
		}

		static inline LeafTrieNode* getLeafNode() {
			return Trie<KEY>::leafNodePool.get_free_resource();
		}
#endif

		Trie() {
#ifdef	NODE_POOL
			root = Trie<KEY>::getInterNode(); 
#endif
			if( sizeof(KEY) > 1 ) root = new InterTrieNode();
			else	root = new LeafTrieNode();
			nodeCount = 1 ;
		}

		~Trie();

		int 	addItem( const KEY& key, const VALUE& value);
		int		readFromFile( const char *);
		int 	write2file( const char *);
		TrieNode*	readTrieNode( FILE* );
		LinkList<VALUE>* queryFromFile( const char *name, const KEY& begin, const KEY& end );
};

#include "trie.cpp"

#endif
