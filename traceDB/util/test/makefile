test: test.o bitmap.o string_hash.o
	g++ -o test test.o bitmap.o string_hash.o

test.o: test.cpp
	g++ -o test.o -c test.cpp

bitmap.o: ../bitmap.cpp ../bitmap.h
	g++ -o bitmap.o -c ../bitmap.cpp

string_hash.o: ../string_hash.h ../string_hash.cpp
	g++ -o string_hash.o -c ../string_hash.cpp

clean:
	rm *.o
	rm test.dat
