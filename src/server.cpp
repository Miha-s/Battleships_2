#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <chrono>


#include "../Battleships/server.hpp"

#define DEBUGGING

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

void Server::processCookie(ChatSession* ses, Headers& user, Headers& serv)
{
    if(user.cookies.empty()) {
        serv.cookies = "id=";
        serv.cookies += std::to_string(initial_id);
        serv.cookies += "; Expires=";
        auto now = std::chrono::system_clock::now();
        now += std::chrono::hours(1);
        long c_time = std::chrono::system_clock::to_time_t(now);
        std::string expires = ctime(&c_time);
        expires.pop_back();
        serv.cookies += expires;

        ses->id = initial_id;
        initial_id++;
    } else {
        ses->id = get_id(user.cookies);
    }
}


void Server::ProcessMessage(char *str, ChatSession* ses)
{
    // Process request
#ifdef DEBUGGING
    std::cout << str;
	std::cout << ses->GetFd();
#endif
    Headers user_heads;
    Headers serv_heads;

    get_headers(str, user_heads);
    processCookie(ses, user_heads, serv_heads);

    if(gms.findGameByPid(ses->id) != -1)
        ses->in_game = true;

    if(user_heads.method == "POST") {
        if(ses->in_game) {
            shot(ses, user_heads);
        } else {
            registerPlayer(ses, user_heads);
        }
        return ;
    }

    if(user_heads.file == "/") {
        if(gms.findGameByPid(ses->id) != -1) {
            gms.removeGame(ses->id);
        }
    }

    int size;
    fillResponse(user_heads, serv_heads);
    int fd = open(serv_heads.file.c_str(), O_RDONLY);
	size = strtol(serv_heads.contentLength.c_str(), nullptr, 10);
    
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
        ses->current = true;
    } else {
        gms.addPlayer(first_player_id, ses->id);
        std::string field = get_post_data(user_heads.file);
        gms.setField(field, ses->id);
        ses->in_game = true;
        
        std::string body = "Y";  // None shot
        send(first_player_id, body);
        ses->current = true;
        findCurrent(first_player_id)->current = false;

        first_player_id = 0;
    }
}

void Server::processShot(ChatSession* ses)
{
    int gm = gms.findGameByPid(ses->id);
    std::string body;
    body = gms.games[gm].coords;
    gms.setReady(ses->id, true);
    int opid = gms.getOtherPid(ses->id);
    ses->current = true;

    int winer;
    if((winer = gms.gameEnded(ses->id))) {
        processEnd(winer, ses->id, body);
        return ;
    }

    send(opid, body);
    findCurrent(opid)->current = false;

}

void Server::shot(ChatSession* ses, Headers& user_heads)
{
    std::string coords = get_post_data(user_heads.file);
    char g = coords[0];
    int gm = gms.findGameByPid(ses->id);
    
	// if player, who have shot, is ready to get message
    if(g == 'g' && gms.playerTurn(gms.getOtherPid(ses->id)) 
                && !gms.playerReady(ses->id)) 
    {
        processShot(ses);
        return;
    } else if(g == 'g' || !gms.playerReady(ses->id)) {
        sendN(ses);
        return;
    }

    int y = coords[0] - '0';
    int x = coords[1] - '0';

    int res = gms.hit(y, x, ses->id);
    if(res == -1) {
        // not your turn
        sendN(ses);
        return;
    } 
    // we save coords for later, when player send another
    // "listening" request, we send these coords to other player
    gms.games[gm].coords = coords;
    gms.setReady(ses->id, false);
    
    std::string body;
    if(res)
        body += "+";
    else 
        body += "-";

    send(ses, body);
}

void Server::sendN(ChatSession* ses)
{
    std::string body = "N";
    sendMes(ses->GetFd(), body);
}

void Server::send(ChatSession* ses, const std::string& body)
{
	sendMes(ses->GetFd(), body);
}

void Server::send(int id, const std::string& body)
{
	int fd = findCurrent(id)->GetFd();
	sendMes(fd, body);
}

void Server::sendMes(int sd, const std::string& body)
{
    Headers serv;
    serv.contentLength = std::to_string(body.size());
    
    std::string response_header = set_headers(serv);
#ifdef DEBUGGING
    std::cout << sd << std::endl;
    std::cout << response_header;
    std::cout << body << std::endl;
#endif

    const char* buf = response_header.c_str();
    write(sd, buf, strlen(buf));
    buf = body.c_str();
    int err = write(sd, buf, strlen(buf));

#ifdef DEBUGGING
    if(err == -1)
        std::cout << "Something gone wrong";
#endif
}

void Server::processEnd(int winer, int pid, std::string& coords)
{
    int opid = gms.getOtherPid(pid);

    std::string mes;
    if(pid == winer)
        mes = "W";
    else 
        mes = "L";
    send(pid, mes);
    mes = coords + "\n";
    if(opid == winer)
        mes += "W";
    else 
        mes += "L";
    send(opid, mes);
    gms.removeGame(pid);
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

#ifdef DEBUGGING
    auto user_addr = addr.sin_addr.s_addr;
    std::string adress(std::move(inet_ntoa(addr.sin_addr)));
    std::cout << adress << std::endl;
#endif

    sessions.push_back(new ChatSession(this, sd));
    
    the_selector->Add(sessions.back());
}
    
