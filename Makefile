fftw_includedir = fftw/include
fftw_libdir = fftw/lib

DEBUG_FLAGS = -ggdb
PRODUCTION_FLAGS = -O2
CPPFLAGS = -I$(fftw_includedir)
CXXFLAGS = -Wall $(DEBUG_FLAGS)
LDFLAGS = -L$(fftw_libdir)
FFTW3_LIBS = -lfftw3
STATIC = -static-libgcc -static-libstdc++
CXX=g++

OBJECT_FILES := my_utils.o

all: xcorrSound

clean:
	rm -rf *.o xcorrSound test_cross soundMatch

my_utils.o :
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c my_utils.cpp -o my_utils.o

cross_correlation : cross_correlation.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(FFTW3_LIBS) $(LDFLAGS) $(STATIC) cross_correlation_test.cpp -o cross_correlation_test 

xcorrSound.o : xcorrSound.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(STATIC) -m64 -c xcorrSound.cpp -o xcorrSound.o

logstream.o : logstream.h logstream.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c logstream.cpp -o logstream.o

xcorrSound : $(OBJECT_FILES) xcorrSound.o logstream.o
	$(CXX) $(CXXFLAGS) $(STATIC) $(LDFLAGS) $(FFTW3_LIBS) -m64 logstream.o my_utils.o xcorrSound.o -o xcorrSound 

AudioFile.o : AudioFile.h AudioFile.cpp AudioStream.h my_utils.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) -c AudioFile.cpp -o AudioFile.o

soundMatch : AudioStream.h AudioFile.o sound_match.cpp my_utils.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OBJECT_FILES) AudioFile.o sound_match.cpp -o soundMatch $(LDFLAGS) $(FFTW3_LIBS)

test_cross : test_cross.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) test_cross.cpp -o test_cross $(LDFLAGS) $(FFTW3_LIBS)


.PHONY : all clean
