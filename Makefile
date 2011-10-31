OBJECT_FILES := my_utils.o
CPP_FLAGS = -Wall -O2

all: xcorrSound

clean:
	rm -rf *.o xcorrSound

$(OBJECTS_FILES): %.o : %.cpp
	g++ -c $(CPP_FLAGS) $< -o $@

cross_correlation: cross_correlation.h
	g++ $(CPP_FLAGS) cross_correlation_test.cpp -o cross_correlation_test -L. -lfftw3

xcorrSound.o: xcorrSound.cpp
	g++ -c xcorrSound.cpp -L. -lfftw3 $(CPP_FLAGS) -o xcorrSound.o

logstream.o: logstream.h logstream.cpp
	g++ -c logstream.cpp -o logstream.o

xcorrSound: $(OBJECT_FILES) xcorrSound.o logstream.o
	g++ $(CPP_FLAGS) logstream.o my_utils.o xcorrSound.o -o xcorrSound -L. -lfftw3

AudioFile: AudioFile.h AudioFile.cpp
	g++ $(CPP_FLAGS) -c AudioFile.cpp -o AudioFile.o

soundMatch: AudioFile sound_match.cpp
	g++ $(CPP_FLAGS) AudioFile.o sound_match.cpp -o soundMatch -L . -lfftw3

.PHONY: all clean