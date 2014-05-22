#include "logstream.h"
#include "stdint.h"
#include <iostream>
#include <cstdio>

using namespace std;

/**
 * logstream buffer start
 */

logstreambuffer::logstreambuffer(int32_t _level, const char *filename) {
    setp(buf, buf+buf_sz);
    this->print_level = _level;
    logfile = fopen(filename, "a");
    if (logfile == NULL) {
        logfile = stderr;
        std::cerr << "Could not open file '" << filename << "' for logging."
                  << std::endl << "Using stderr for logging instead" << std::endl;
    }
}

logstreambuffer::~logstreambuffer() {
    flush();
    sync();
    fclose(logfile);
}

void logstreambuffer::flush() {
    if (this->print_level >= this->level) {
	fwrite(buf, sizeof(char), pptr()-buf, logfile);
    }
    setp(buf, buf+buf_sz);
}

int logstreambuffer::overflow(int c) {
    flush();
    *pptr() = c;
    pbump(1);
    return 0;
}

int logstreambuffer::sync() {
    flush();
    return 0;
}

void logstreambuffer::set_level(int32_t level) {
    if (this->level == level) return;
    sync();
    this->level = level;
}

/**
 * logstream start
 */
void logstream::set_level(int32_t n) {
    buffer.set_level(n);
}

void logstream::set_print_level(int32_t n) {
    buffer.print_level = n;
}

/**
 * logging manipulators
 */

log_level log_debug() {
    return log_level(logstream::DEBUG);
}

log_level log_information() {
    return log_level(logstream::INFORMATION);
}

log_level log_fatal() {
    return log_level(logstream::FATAL);
}

log_level log_error() {
    return log_level(logstream::ERROR);
}

log_level log_warning() {
    return log_level(logstream::WARNING);
}

logstream& operator<<(logstream &ls, log_level ll) {
    ls.set_level(ll.n);
    switch (ll.n) {
    case logstream::DEBUG:
        ls << std::endl << " --- DEBUG --- " << std::endl;
        break;
    case logstream::INFORMATION:
        ls << std::endl << " --- INFORMATIONAL ---" << std::endl;
        break;
    case logstream::FATAL:
        ls << std::endl << " --- FATAL ---" << std::endl;
        break;
    case logstream::ERROR:
        ls << std::endl << " --- ERROR ---" << std::endl;
        break;
    case logstream::WARNING:
        ls << std::endl << " --- WARNING ---" << std::endl;
        break;
    }
    return ls;
}
