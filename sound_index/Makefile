fftw_includedir = /home/jsn/Documents/abekat/scape-xcorrsound/trunk/fftw/include
fftw_libdir = /home/jsn/Documents/abekat/scape-xcorrsound/trunk/fftw/lib
BOOST_DIR = /usr/include/boost
BOOST_LIB = /usr/lib/
DEBUG_FLAGS = -ggdb
PRODUCTION_FLAGS = -O2
CPPFLAGS = -I$(fftw_includedir) -I $(BOOST_DIR)
CXXFLAGS = -Wall $(DEBUG_FLAGS)
LDFLAGS = -L$(fftw_libdir) -L$(BOOST_LIB) $(FFTW3_LIBS)
FFTW3_LIBS = -lfftw3
BOOST_LIBS = -lboost_program_options
STATIC = -static-libgcc -static-libstdc++ -static
CXX=g++

OBJECT_FILES = ../my_utils.o ../AudioFile.o

all : testfingerprint

clean :
	rm -rf *.o testfingerprint

testfingerprint : testfingerprint.cpp fingerprint.h ../AudioFile.o ../my_utils.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) testfingerprint.cpp -o testfingerprint $(FFTW3_LIBS) $(LDFLAGS)

.PHONY : clean