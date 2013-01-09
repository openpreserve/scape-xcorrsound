#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <stdint.h>
#include <iostream>

class logstreambuffer : public std::basic_streambuf<char, std::char_traits<char> > {
private:
    const static size_t buf_sz = 2048;
    char buf[buf_sz];
    int32_t level;
    int32_t print_level;
    FILE * logfile;

public:
    logstreambuffer(int32_t level, const char *filename);
    ~logstreambuffer();
    
    void flush();
    int overflow(int c = std::char_traits<char>::eof());
    int sync();
    void set_level(int32_t level);

    friend class logstream;
};

class logstream : public std::ostream {
private:
    logstreambuffer buffer;
public:
    logstream(int32_t level, const char *filename) : std::ostream(&buffer), buffer(level, filename) {};
    void set_level(int32_t _level);
    void set_print_level(int32_t level);
    static const int32_t FATAL = 1;
    static const int32_t ERROR = 2;
    static const int32_t INFORMATION = 3;
    static const int32_t WARNING = 4;
    static const int32_t DEBUG = 5;    

};

/**
 * logging manipulators
 */

struct log_level {
public:
    int32_t n;
    log_level(int32_t _n) : n(_n) {}
};

extern logstream ls;

log_level log_debug();
log_level log_information();
log_level log_fatal();
log_level log_error();
log_level log_warning();

logstream& operator<<(logstream &ls, log_level ll);

#endif
