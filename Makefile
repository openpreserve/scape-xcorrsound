OBJECT_FILES := my_utils.o
CPP_FLAGS = -Wall -O2
CC=g++
all: xcorrSound

clean:
	rm -rf *.o xcorrSound

$(OBJECTS_FILES): %.o : %.cpp
	$(CC) -c $(CPP_FLAGS) $< -o $@

cross_correlation: cross_correlation.h
	$(CC) $(CPP_FLAGS) cross_correlation_test.cpp -o cross_correlation_test -L. -lfftw3

xcorrSound.o: xcorrSound.cpp
	$(CC) -c xcorrSound.cpp -L. -lfftw3 $(CPP_FLAGS) -o xcorrSound.o

logstream.o: logstream.h logstream.cpp
	$(CC) -c logstream.cpp -o logstream.o

xcorrSound: $(OBJECT_FILES) xcorrSound.o logstream.o
	$(CC) $(CPP_FLAGS) logstream.o my_utils.o xcorrSound.o -o xcorrSound -L. -lfftw3

AudioFile: AudioFile.h AudioFile.cpp
	$(CC) $(CPP_FLAGS) -c AudioFile.cpp -o AudioFile.o

soundMatch: AudioFile sound_match.cpp
	$(CC) $(CPP_FLAGS) AudioFile.o sound_match.cpp -o soundMatch -L . -lfftw3

test_cross: test_cross.cpp
	$(CC) $(CPP_FLAGS) test_cross.cpp -o test_cross -L . -lfftw3


.PHONY: all clean