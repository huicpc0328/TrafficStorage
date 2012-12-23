#include <cstdio>
#include <iostream>
using namespace std;

template<typename A>
class CA {
	protected:
		A data;
	public:

		CA(){
			data = 10;
		}

		CA( const A &a ) {
			data = a ;
		}
		~CA() {

		}
		
		virtual void print(){
			cout<<data<<endl;
		}
};


template<typename A, typename B>
class CB: CA<A> {
	private:
		B value;
	public:
		CB(){
			CA<A>::data = 20;
		}
		
		~CB(){
		}

		void print() {
			cout<<CA<A>::data<<endl;
		}
};


