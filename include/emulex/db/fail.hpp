//
//  fail.hpp
//  boost-utils
//
//  Created by Centny on 1/23/17.
//
//

#ifndef fail_hpp
#define fail_hpp
#include <exception>
#include <string>
#include <stdarg.h>
#include <stdio.h>

/*
 the common fail exception by code/message.
 */
class Fail_ : public std::exception {
   public:
    int code;
    char* err;

   public:
    Fail_(int code) : code(code), err(0) {}

    Fail_(size_t bsize, const char* fmt, ...) : code(0) {
        err = new char[bsize];
        va_list args;
        va_start(args, fmt);
        vsprintf(err, fmt, args);
        va_end(args);
    }

    Fail_(size_t bsize, int code, const char* fmt, ...) : code(code) {
        err = new char[bsize];
        va_list args;
        va_start(args, fmt);
        vsprintf(err, fmt, args);
        va_end(args);
    }

    const char* what() const throw() { return err; }

    virtual ~Fail_() throw() {
        if (err) {
            delete err;
        }
        err = 0;
    }
};

#define Fail(fmt, args...) Fail_(256, fmt, args)
#define LFail(bsize, fmt, args...) Fail_(bsize + 1024, 0, fmt, args)
#define CFail(code) Fail_(code)
#define LCFail(bsize, code, fmt, args...) Fail_(bsize, code, fmt, args)

#endif /* fail_hpp */
