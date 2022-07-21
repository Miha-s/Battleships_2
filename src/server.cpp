#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../Battleships/server.hpp"

ChatSession::ChatSession(Server *a_master, int fd) 
    : FdHandler(fd, true), buf_used(0), ignoring(false),
    the_master(a_master)
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
            buffer[i-3] = 0;
            // serv->ProcessMessage
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
    
