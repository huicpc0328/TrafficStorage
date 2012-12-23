#include <iostream>
using namespace std;

void test2( int & );
void test( int &a ) {

	a = 5;
	cout<<a<<endl;
	test2( a );
}

void test2( int &a ) {
	a = 6;
	cout<<a<<endl;
	int tmp = a;

}

int & get( int & a ) {

	return a ;
}


struct node {
	char c;
	int idata;
};

node & getNode( node &a ) {

	return a;
}

int main() {
	int a = 10;
	test( a );
	cout<<a<<endl;
	int &tmp = get(a);
	tmp = 100;
	a = 1111;
	cout<<tmp<<endl;
	cout<<a<<endl;

	node nodea;
	nodea.idata = 88;
	node &b = getNode(nodea);
	b.idata = 99;
	cout<<b.idata<<endl;
	cout<<nodea.idata<<endl;

	return 0;
}
