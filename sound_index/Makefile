fftw_includedir = /home/jsn/Documents/abekat/scape-xcorrsound/trunk/fftw/include
fftw_libdir = /home/jsn/Documents/abekat/scape-xcorrsound/trunk/fftw/lib
BOOST_DIR = /usr/include/boost
BOOST_LIB = /usr/lib/
DEBUG_FLAGS = -ggdb
PRODUCTION_FLAGS = -O2
CPPFLAGS = -I$(fftw_includedir) -I $(BOOST_DIR)
#CXXFLAGS = -Wall $(DEBUG_FLAGS)
CXXFLAGS = $(PRODUCTION_FLAGS)
LDFLAGS = -L$(fftw_libdir) -L$(BOOST_LIB) $(FFTW3_LIBS)
FFTW3_LIBS = -lfftw3
BOOST_LIBS = -lboost_program_options
STATIC = -static-libgcc -static-libstdc++ -static
CXX=g++

OBJECT_FILES = ../my_utils.o ../AudioFile.o my_fingerprint.o my_database.o my_query.o

all : testfingerprint

clean :
	rm -rf *.o testfingerprint test_my_fingerprint

my_database.o : my_database.h my_database.cpp my_fingerprint.h ../AudioFile.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_database.cpp $(LDFLAGS)

my_fingerprint.o : my_fingerprint.cpp my_fingerprint.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_fingerprint.cpp $(FFTW3_LIBS) $(LDFLAGS)

my_query.o : my_query.cpp my_query.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_query.cpp $(FFTW3_LIBS) $(LDFLAGS)

testfingerprint : testfingerprint.cpp fingerprint.h ../AudioFile.o ../my_utils.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) testfingerprint.cpp -o testfingerprint $(FFTW3_LIBS) $(LDFLAGS)

test_my_fingerprint : my_database.o my_fingerprint.o my_query.o test_my_fingerprint.cpp ../AudioFile.o ../my_utils.o my_fingerprint.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) test_my_fingerprint.cpp -o test_my_fingerprint $(FFTW3_LIBS) $(LDFLAGS)

.PHONY : clean