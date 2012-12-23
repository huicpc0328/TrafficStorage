#include "a.h"


int main() {

	CA<int>* p = (CA<int> *)new CB<int,int>();
	p->print();
	delete p;
	p = new CA<int>();
	p->print();

	return 0;
}
