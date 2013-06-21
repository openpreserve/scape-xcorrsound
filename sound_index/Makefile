DEBUG_FLAGS = -g
PRODUCTION_FLAGS = -O2
CPPFLAGS = -m64
#CXXFLAGS = -Wall $(DEBUG_FLAGS)
CXXFLAGS = -Wall -pedantic $(PRODUCTION_FLAGS)
FFTW3_LIBS = -lfftw3
BOOST_LIBS = -lboost_program_options
LDFLAGS = -Wl,$(FFTW3_LIBS),$(BOOST_LIBS),-s,--as-needed
STATIC = -static-libgcc -static-libstdc++ -static
CXX=/usr/bin/g++

OBJECT_FILES = ../my_utils.o ../AudioFile.o my_fingerprint.o my_database.o my_query.o

all : testfingerprint

another : lets_try_again.cpp ../AudioFile.o ../my_utils.o ../my_utils.h db_wrapper.h db_wrapper_sqlite.o $(OBJECT_FILES) db_wrapper_postgres.o
#	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) db_wrapper_sqlite.o sqlite/sqlite3.o lets_try_again.cpp -o another $(FFTW3_LIBS) $(LDFLAGS) -lpthread -ldl -std=c++0x
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) db_wrapper_postgres.o lets_try_again.cpp -o another $(FFTW3_LIBS) $(LDFLAGS) -lpthread -ldl -std=c++0x -I/usr/lib -lpqxx -lpq 

experiment : experiment.cpp ../AudioFile.o ../my_utils.o ../my_utils.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) experiment.cpp -o experiment $(FFTW3_LIBS) $(LDFLAGS)

db_wrapper_sqlite.o : db_wrapper.h db_wrapper_sqlite.cpp sqlite/sqlite3.h
	$(CXX) $(CXXFLAGS) -c db_wrapper_sqlite.cpp -o db_wrapper_sqlite.o

db_wrapper_postgres.o : db_wrapper.h db_wrapper_postgres.cpp
	$(CXX) $(CXXFLAGS) -c db_wrapper_postgres.cpp -o db_wrapper_postgres.o -I/usr/lib -lpqxx -lpq 

ismir_impl : ../AudioFile.o ../my_utils.o ../my_utils.h ismir_impl.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) ismir_impl.cc -o ismir_impl $(FFTW3_LIBS) $(LDFLAGS)

most_significant : most_significant.cpp ../AudioFile.o ../my_utils.o ../my_utils.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) most_significant.cpp -o ms $(FFTW3_LIBS) $(LDFLAGS) -std=c++0x

my_database.o : my_database.h my_database.cpp my_fingerprint.h ../AudioFile.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_database.cpp $(LDFLAGS)

my_fingerprint.o : my_fingerprint.cpp my_fingerprint.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_fingerprint.cpp $(FFTW3_LIBS) $(LDFLAGS)

my_query.o : my_query.cpp my_query.h ../my_utils.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_query.cpp $(FFTW3_LIBS) $(LDFLAGS)

test : test.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) test.cc -o test $(FFTW3_LIBS) $(LDFLAGS)

test_my_fingerprint : my_database.o my_fingerprint.o my_query.o test_my_fingerprint.cpp ../AudioFile.o ../my_utils.o ../my_utils.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) test_my_fingerprint.cpp -o test_my_fingerprint $(FFTW3_LIBS) $(LDFLAGS)

testfingerprint : testfingerprint.cpp fingerprint.h ../AudioFile.o ../my_utils.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) testfingerprint.cpp -o testfingerprint $(FFTW3_LIBS) $(LDFLAGS)

clean :
	rm -rf *.o testfingerprint test_my_fingerprint another experiment ms test

.PHONY : clean
