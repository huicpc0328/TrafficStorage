#include "trie_node.h"

/* layout of storage in file
 * |-------------------------|
 * |   isLeaf	             |
 * |   elemCount             |
 * |   1st key 				 | 
 * |   2nd key    			 | 
 * |   .........             |
 * |   last key              |
 * |   1st child offset      |
 * |   2nd child offset      |
 * |   .........             |
 * |   last child offset     |
 * |-------------------------|
 */
// we must have given a realistic array address to the variable fileOffArray 
// before we calling the funtion write2file, because we cannot get the writing offset 
// of each child TrieNode in file in real-time. So we must calculate the offset of each child TrieNode
// and store this value into an array in advance .
// NOTE: size of single BTree file must be less than 4G because offset is int32_t type 

// file IO
int InterTrieNode::write2file( FILE * fp) {
	if( !fp ) {
		ERROR_INFO("NULL file pointer fp",return -1);
	}

	fwrite( &(this->isLeaf), sizeof(this->isLeaf) , 1, fp);
	fwrite( &(this->elemCount), sizeof(this->elemCount), 1, fp);

	// we need to calculate the children's offset of file in advance. 
	assert( fileOffArray != NULL );
	fwrite( fileOffArray, sizeof( uint32_t ) * this->elemCount , 1, fp );
	fileOffArray = NULL; // make sure that each calling of this function should call function setFileOffset firstly.
	return 0;
}

int InterTrieNode::readFromFile( FILE *fp ) {
	if( !fp ) {
		ERROR_INFO("NULL file pointer fp",return -1);
	}
	//we have readed isLeaf value before we call this function
	//read( fd, &(this->isLeaf), sizeof(this->isLeaf));
	fread( &(this->elemCount), sizeof(this->elemCount), 1, fp);
	return 0;
}

// get the total size that we writed this InternalNode Object in file
int32_t InterTrieNode::getSizeOfFile() {
	int32_t ret = sizeof( this->elemCount ) + sizeof(this->isLeaf);
	ret += this->elemCount * sizeof( KEY ) ;
	ret += this->elemCount * sizeof( uint32_t ) ;
	return ret;
}


int InterTrieNode::addItem( const KEY& pos, TrieNodePtr child) {
	if( this->elemCount >= MAX_PER_NODE ||
			pos < 0 || pos >= MAX_PER_NODE ) {
		return -1;
	}

	childPtr[pos] = child;
	++this->elemCount;
	return 0;
}


