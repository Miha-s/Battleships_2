#ifndef HTTPMESSAGE_HPP_SENTRY
#define HTTPMESSAGE_HPP_SENTRY
#include <string>



struct Headers {
    std::string method;
    std::string file;
    std::string code;
    std::string connection;
    std::string contentType;
    std::string contentLength;
    std::string cookies;
    std::string body;
};


void get_headers(char *mes, Headers& h);
std::string set_headers(Headers& h);
void fillResponse(Headers& user, Headers& serv);
string get_post_data(const string& str);


int get_id(const string& str);
std::string get_type(const string& str);

#endif
