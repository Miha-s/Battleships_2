#include "../Battleships/httpmessage.hpp"
#include <unistd.h>
#include <sys/stat.h>

using std::string;

// get header and first option, skip the rest of line
// mes - the string
// header - return header
// data   - return header parameters
// next - return next line
int read_line(char* mes, char* &header, char* &data, char* &next) 
{
    if(!*mes)
        return 0;

    int i = 0;
	// getting the header name
    for(; mes[i] != ':' && mes[i] != ' '; ++i) 
		if(mes[i] == 0)
			return 0;
	
    mes[i] = 0;
    header = mes;
    if(mes[i+1] == '/') {  // if this is the method line
        i += 1;
    } else {
        i += 2;
    }

    data = mes + i;
    for(; mes[i] != ',' && mes[i] != '\r' && mes[i] != ' ' && mes[i] != ';';i++)
		if(mes[i] == 0)
			return 0; 
    mes[i] = 0;
    i++;
    for(; mes[i] != '\n';i++) 
		if(mes[i] == 0)
			return 0;
    next = mes+i;
    return 1;
}

const string connection = "Connection";
const string cookie = "Cookie";
const string contentLength = "Content-Length";
const string head_response = "HTTP/1.1 200 OK";
const string set_cookie = "Set-Cookie";
const string contentType = "Content-Type";


void get_headers(char *mes, Headers& h)
{
    char *header, *data;

    read_line(mes, header, data, mes);
    h.method = header;
    h.file = data;
    while(read_line(mes, header, data, mes)) {
        if(header == cookie)
            h.cookies = data;
        else if(header == contentLength)
            h.contentLength = data;
    }
}

void add_header(string& resp, const string& header, const string& val)
{
    resp += header + ": " + val + "\r\n";
}

string set_headers(Headers& h)
{
    string responce;
    responce += head_response + "\r\n";
    if(h.connection.length())
        add_header(responce, connection, h.connection);
    if(h.cookies.length())
        add_header(responce, set_cookie, h.cookies);
    if(h.contentLength.length())
        add_header(responce, contentLength, h.contentLength);
    if(h.contentType.length())
        add_header(responce, contentType, h.contentType);
    
    responce += "\r\n";
    return responce;
}

void fillResponse(Headers& user, Headers& serv)
{
    if(user.file == "/") user.file = "/page.html";
    serv.contentType = get_type(user.file);
	char* path = get_current_dir_name();
    serv.file += path;
	serv.file += user.file;
    serv.connection = "Keep-Alive";

	struct stat buf;
	int err = stat(serv.file.c_str(), &buf);
    int size;
    if(err)
        size = 0;
    else
        size = buf.st_size;
    
    serv.contentLength = std::to_string(size);

	free(path);
}

int get_id(const string& str)
{
    int i = 0;
    const char* cookie = str.c_str();
    for(;cookie[i] != '=';i++) 
		if(cookie[i] == 0)
			return 0;
    return strtol(cookie+i+1, nullptr, 10);
}

string get_type(const string& str)
{
    int i = 0;
    const char* buf = str.c_str();
	string type;
    for(;buf[i]!='.';i++) 
		if(buf[i] == 0) {
			type = "txt";
		}
	if(!type.length())
		type = buf + i + 1;

    string tmp;
    tmp += "text/";
    tmp += type;
    tmp += "; charset=utf-8";
    return tmp;
}

string get_post_data(const string& str)
{
    const char* tmp = str.c_str();
    int i = 0;
    for(;tmp[i] != '?'; i++) 
		if(tmp[i] == 0)
			return string("");
    i++;
    string data(tmp + i);
    return data;
}