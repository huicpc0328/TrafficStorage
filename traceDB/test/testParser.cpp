#include "parser.h"
#include "collector.h"
#include "global.h"


int main() {
	Collector c;

	Parser *paser[3];
	for( int i = 0 ;i < 3;i++ ) {
		paser[i] = new Parser( c );
		paser[i]->start();
	}

	c.collect();
	GlobalVar::clean();
	return 0;
}
