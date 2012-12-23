#all: testcollector testparser testexporter
all: traceDB

CC=g++
CPPFLAGS=-O2 -g

DEPF = global.h collector.h dbHandle.h exporter.h ./include/JsonBox.h \
		collector.o parser.o record.o fileWriter.o index.o exporter.o

DEPO = -ltrace collector.o parser.o record.o index.o exporter.o \
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

exporter.o: exporter.cpp
	$(CC) -c $(CPPFLAGS) exporter.cpp

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
	
clean:
	rm *.o
