#include "trie_node.h"

/* layout of storage in file
 * |-------------------------|
 * |   isLeaf	             |
 * |   elemCount             |
 * |   bitmap				 |
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
	this->bitmap.dump2file( fp );

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
	//fread( &(this->isLeaf), sizeof(this->isLeaf) , 1, fp);
	fread( &(this->elemCount), sizeof(this->elemCount), 1, fp);
	this->bitmap.readFromFile( fp );
	return 0;
}

// get the total size that we writed this InternalNode Object in file
int32_t InterTrieNode::getSizeOfFile() {
	int32_t ret = sizeof( this->elemCount ) + sizeof(this->isLeaf);
	ret += this->bitmap.getSizeOfFile();
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

LeafTrieNode::LeafTrieNode() : TrieNode( 1 ) {
	for( int i = 0 ; i < MAX_PER_NODE; ++i ) resultArray[i] = NULL;
}

LeafTrieNode::~LeafTrieNode() {
	for( int i = 0 ; i < MAX_PER_NODE; ++i )
		if( resultArray[i] ) 
			delete resultArray[i];
}

int LeafTrieNode::addItem( const KEY& pos, const VALUE& value ) {
	if( resultArray[pos] == NULL ) {
		resultArray[pos] = new LinkList<VALUE>();
		if( !resultArray[pos] ) {
			ERROR_INFO("cannot alloc memory for linklist", return -1);
		}
		resultArray[pos]->clear();
		++this->elemCount;
	}
	resultArray[pos]->addElement(value);
	return 0;
}

LinkList<VALUE>* LeafTrieNode::getResult( const uint8_t idx ) {
	assert( idx >= 0 && idx <= MAX_PER_NODE );
	assert( resultArray[idx] != NULL );
	return resultArray[idx];
}

/* layout of storage in file
 * |-------------------------|
 * |   isLeaf	             |
 * |   elemCount             |
 * |   bitmap                |
 * |   offset Array          |
 * |   1st values' linklist  |
 * |   2nd values' linklist  |
 * |   .........             |
 * |   last values' linklist |
 * |-------------------------|
 */

int LeafTrieNode::write2file( FILE * fp) {
	if( !fp ) {
		ERROR_INFO("NULL file pointer fp",return -1);
	}

	fwrite( &(this->isLeaf), sizeof(this->isLeaf), 1, fp);
	fwrite( &(this->elemCount), sizeof(this->elemCount), 1, fp);
	this->bitmap.dump2file( fp );
	uint32_t* 	bitPosition = new uint32_t[ elemCount ];
	bitmap.getOnePosition( bitPosition );
	uint32_t offset = 0;
	// write file-offset of linklist
	for( int i = 0 ; i < this->elemCount ; ++i ) {
		fwrite( &offset, sizeof(offset), 1 , fp );
		assert( resultArray[ bitPosition[i] ] != NULL );
		offset += resultArray[ bitPosition[i] ]->getSizeOfFile();
	}
	// write result-linklist
	for( int i = 0 ; i < this->elemCount ; ++i ) {
		if( resultArray[ bitPosition[i] ]->write2file( fp ) == -1 ) {
			return -1;
		}
	}
	return 0;	
}

int LeafTrieNode::readFromFile( FILE * fp) {
	if( !fp ) {
		ERROR_INFO("NULL file pointer fp",return -1);
	}
	//we have read isLeaf value before we call this function
	//read( fd, &(this->isLeaf), sizeof(this->isLeaf));
	fread( &(this->elemCount), sizeof(this->elemCount), 1, fp);
	this->bitmap.readFromFile( fp );
	fileOffArray = new uint32_t[ elemCount ];
	//read file-offset of linklist
	fread( fileOffArray, sizeof(uint32_t), elemCount, fp );
	delete fileOffArray;
	fileOffArray = NULL;

	if( elemCount >= (MAX_PER_NODE>>1) ) {
		// get one-bit positions of bitmap
		uint32_t* 	bitPosition = new uint32_t[ elemCount ];
		bitmap.getOnePosition( bitPosition );

		for( int i = 0 ; i < this->elemCount ; ++i ) {
			int pos = bitPosition[ i ];
			resultArray[pos] = new LinkList<VALUE>();
			if( resultArray[pos]->readFromFile( fp ) == -1 ) {
				return -1;
			}
		}
	} else {
		for( int i = 0 ; i < MAX_PER_NODE; ++i ) {
			if( bitmap.getBit(i) ) {
				resultArray[i] = new LinkList<VALUE>();
				if( resultArray[i]->readFromFile( fp ) == -1 ) {
					return -1;
				}
			}
		}
	}
	
	return 0;
}	

// get the total size that we writed this LeafNode Object in file
int32_t LeafTrieNode::getSizeOfFile() {
	int32_t ret = sizeof( this->elemCount ) + sizeof( this->isLeaf);
	ret += bitmap.getSizeOfFile();
	// for offset array size
	ret += elemCount * sizeof(uint32_t);

	if( elemCount >= (MAX_PER_NODE>>1) ) {
		// get one-bit positions of bitmap
		uint32_t* 	bitPosition = new uint32_t[ elemCount ];
		bitmap.getOnePosition( bitPosition );

		for( int i = 0 ; i < this->elemCount ; ++i ) {
			int pos = bitPosition[ i ];
			ret += resultArray[pos]->getSizeOfFile();
		}
	} else {
		for( int i = 0 ; i < MAX_PER_NODE; ++i ) {
			if( bitmap.getBit(i) ) {
				ret += resultArray[i]->getSizeOfFile();
			}
		}
	}

	return ret;
}


