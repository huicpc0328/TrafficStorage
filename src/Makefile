#all: testcollector testparser testexporter
all: traceDB

CC=g++
INDEXTYPE=TRIE
#INDEXTYPE=BTREE
ifeq ($(INDEXTYPE),TRIE)
CPPFLAGS=-O2 -g -Wno-unused-result -DUSE_FILEIO -DHASH_PERF
else
CPPFLAGS=-O2 -g -Wno-unused-result -DUSE_FILEIO -DHASH_PERF -DBTREE
endif

DEPF = global.h collector.h dbHandle.h exporter.h ./include/JsonBox.h \
		collector.o parser.o record.o fileWriter.o index.o exporter.o fileManager.o perfMeasure.o trie_node.o bitmap.o

DEPO = -ltrace collector.o parser.o record.o index.o exporter.o fileManager.o perfMeasure.o trie_node.o bitmap.o \
		 fileWriter.o ./build/libJsonBox.a -lpthread -ltrace

#testcollector: testCollector.cpp $(DEPF)
#	$(CC) $(CPPFLAGS) -o testcollector testCollector.cpp $(DEPO) 

#testparser:testParser.cpp $(DEPF)
#	$(CC) $(CPPFLAGS) -o testparser testParser.cpp $(DEPO)
	
#testexporter: test/testExporter.cpp $(DEPO)
#	$(CC) -o test/testexporter $(CPPFLAGS) test/testExporter.cpp -lmapi $(DEPO)

traceDB: main.cpp $(DEPF)
	$(CC) -o traceDB $(CPPFLAGS) main.cpp $(DEPO)

index.o: index.cpp 
	$(CC) -c $(CPPFLAGS) index.cpp -lpthread

trie_node.o: util/trie_node.h
	$(CC) -c $(CPPFLAGS) util/trie_node.cpp

bitmap.o: util/bitmap.cpp
	$(CC) -c $(CPPFLAGS) util/bitmap.cpp

exporter.o: exporter.cpp
	$(CC) -c $(CPPFLAGS) exporter.cpp -DWRITE_PACKETS

collector.o: collector.cpp 
	$(CC) -c $(CPPFLAGS) collector.cpp -ltrace -lpthread

parser.o: parser.cpp 
	$(CC) -c $(CPPFLAGS) parser.cpp

record.o: record.cpp
	$(CC) -c $(CPPFLAGS) record.cpp
	
dbHandle.o: dbHandle.cpp -lmapi 
	$(CC) -c $(CPPFLAGS)  dbHandle.cpp -lmapi
	
fileWriter.o: fileWriter.cpp
	$(CC) -c $(CPPFLAGS) fileWriter.cpp -lmapi

fileManager.o: fileManager.cpp
	$(CC) -c $(CPPFLAGS) fileManager.cpp 

perfMeasure.o: perfMeasure.cpp
	$(CC) -c $(CPPFLAGS) perfMeasure.cpp 

clean:
	rm *.o
