#include "../Battleships/httpmessage.hpp"


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
    for(; mes[i] != '\n';i++) {}
    next = mes+i+1;
    return 1;
}