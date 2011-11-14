fftw_includedir = fftw/include
fftw_libdir = fftw/lib

CPPFLAGS = -I$(fftw_includedir)
CXXFLAGS = -Wall -O2
LDFLAGS = -L$(fftw_libdir)
FFTW3_LIBS = -lfftw3
STATIC = -static-libgcc -static-libstdc++
CXX=g++

OBJECT_FILES := my_utils.o

all: xcorrSound

clean:
	rm -rf *.o xcorrSound test_cross soundMatch

$(OBJECT_FILES): %.o : %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

cross_correlation: cross_correlation.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) cross_correlation_test.cpp -o cross_correlation_test $(LDFLAGS) $(FFTW3_LIBS)

xcorrSound.o: xcorrSound.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(STATIC) -m64 -c xcorrSound.cpp -o xcorrSound.o

logstream.o: logstream.h logstream.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c logstream.cpp -o logstream.o

xcorrSound: $(OBJECT_FILES) xcorrSound.o logstream.o
	$(CXX) $(CXXFLAGS) $(STATIC) -m64 logstream.o my_utils.o xcorrSound.o -o xcorrSound $(LDFLAGS) $(FFTW3_LIBS)

AudioFile: AudioFile.h AudioFile.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c AudioFile.cpp -o AudioFile.o

soundMatch: AudioFile sound_match.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) AudioFile.o sound_match.cpp -o soundMatch $(LDFLAGS) $(FFTW3_LIBS)

test_cross: test_cross.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) test_cross.cpp -o test_cross $(LDFLAGS) $(FFTW3_LIBS)


.PHONY: all clean
