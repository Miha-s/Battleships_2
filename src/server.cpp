#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#include "../Battleships/server.hpp"

#ifdef DEBUGGING
#include <iostream>
#endif

ChatSession::ChatSession(Server *a_master, int fd) 
    : FdHandler(fd, true), buf_used(0), ignoring(false),
	in_game(false), the_master(a_master)
{ 
	
}

ChatSession::~ChatSession()
{
}

void ChatSession::Send(const char *msg)
{
    write(GetFd(), msg, strlen(msg));
}

void ChatSession::Handle(bool r, bool w)
{ 
    if(!r)
        return;
    if(buf_used >= (int)sizeof(buffer)) {
        buf_used = 0;
        ignoring = true;
    }
    if(ignoring)
        ReadAndIgnore();
    else
        ReadAndCheck();
}

void ChatSession::ReadAndIgnore()
{
    int rc = read(GetFd(), buffer, sizeof(buffer));
    if(rc < 1) {
        the_master->RemoveSession(this);
        return;
    }
    int i;
	// skip to the next message
	// here we suppose that all messages are without body
    for(i = 0; i < rc; i++)
        if(buffer[i] == '\n' && buffer[i-2] == '\n') { 
            int rest = rc - i - 1;
            if(rest > 0)
                memmove(buffer, buffer + i + 1, rest);
            buf_used = rest;
            ignoring = 0;
            CheckMessage();
        }
}

void ChatSession::ReadAndCheck()
{
    int rc = read(GetFd(), buffer+buf_used, sizeof(buffer)-buf_used);
    if(rc < 1) {
        the_master->RemoveSession(this);
        return ;
    }
    buf_used += rc;
    CheckMessage();
}

void ChatSession::CheckMessage()
{
    if(buf_used <= 0)
        return;
    int i;

    for(i = 0; i < buf_used; i++)
        if(buffer[i] == '\n' && buffer[i-2] == '\n') {
            buffer[i-1] = 0;
            the_master->ProcessMessage(buffer, this);
            int rest = buf_used - i - 1;
            memmove(buffer, buffer + i + 1, rest); 
            buf_used = rest;
            CheckMessage();
            return;
        }
}

void Server::ProcessMessage(char *str, ChatSession* ses)
{
    // Process request
#ifdef DEBUGGING
    std::cout << str;
	std::cout << ses->GetFd();
#endif
    int id = initial_id;
    Headers user_heads;
    Headers serv_heads;

    get_headers(str, user_heads);
    if(user_heads.cookies.empty()) {
        serv_heads.cookies = "id=";
        serv_heads.cookies += std::to_string(initial_id);
        ses->id = initial_id;
        initial_id++;
    } else {
        ses->id = get_id(user_heads.cookies);
    }

    if(gms.findGameByPid(ses->id) != -1)
        ses->in_game = true;

    if(user_heads.method == "POST") {
        if(ses->in_game) {
            shot(ses, user_heads);
        } else {
            registerPlayer(ses, user_heads);
        }
    }

    int size;
    int fd = open(serv_heads.file.c_str(), O_RDONLY);
    fillResponse(user_heads, serv_heads);

    std::string response_header = set_headers(serv_heads);
#ifdef DEBUGGING
    std::cout << response_header;
#endif

    const char* buf = response_header.c_str();
    int sd = ses->GetFd();
    write(sd, buf, strlen(buf));
    sendfile(sd, fd, NULL, size);
    close(fd);
}

void Server::registerPlayer(ChatSession* ses, Headers& user_heads) 
{
    if(!first_player_id) {
        first_player_id = ses->id;
        std::string field = get_post_data(user_heads.file);
        gms.addGame(ses->id, 0);
        gms.setField(field, ses->id);
        ses->in_game = true;
    } else {
        gms.addPlayer(first_player_id, ses->id);
        std::string field = get_post_data(user_heads.file);
        gms.setField(field, ses->id);
        ses->in_game = true;
        
        std::string body = "N";  // None shot
        send(first_player_id, body);

        first_player_id = 0;
    }
}

ChatSession* Server::findCurrent(int id)
{
	for(auto elem : sessions) {
        if(elem->id == id && elem->current)
            return elem;
    }
    return 0;
}

/////// Sever part ///////

Server::Server(EventSelector *sel, int fd)
    : FdHandler(fd, true), the_selector(sel)
{
    the_selector->Add(this);
}

Server::~Server()
{
    for(auto el : sessions)
        the_selector->Remove(el);
    the_selector->Remove(this);
}

Server *Server::Start(EventSelector *sel, int port)
{
    int ls, opt, res;
    struct sockaddr_in addr;

    ls = socket(AF_INET, SOCK_STREAM, 0);
    if(ls == -1)
        return 0;
    opt = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// creating listening socket
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    res = bind(ls, (struct sockaddr*) &addr, sizeof(addr));
    if(res == -1)
        return 0;

    res = listen(ls, qlen_for_listen);
    if(res == -1)
        return 0;

    return new Server(sel, ls);
}

void Server::RemoveSession(ChatSession *s)
{
    the_selector->Remove(s);
	delete s;
    sessions.remove(s);
}

void Server::Handle(bool r, bool w)
{
    if(!r)
        return;
    int sd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    sd = accept(GetFd(), (struct sockaddr*) &addr, &len);
    if(sd == -1)
        return;

    sessions.push_back(new ChatSession(this, sd));
    
    the_selector->Add(sessions.back());
}
    
