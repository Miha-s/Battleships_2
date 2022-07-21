#ifndef HTTPMESSAGE_HPP_SENTRY
#define HTTPMESSAGE_HPP_SENTRY
#include <string>

using std::string;

struct Headers {
    string method;
    string file;
    string code;
    string connection;
    string contentType;
    string contentLength;
    string cookies;
    string body;
};


int get_headers(char *mes, Headers& h);
string set_headers(Headers h);

int get_id(const string& str);
string get_type(const string& str);

#endif
