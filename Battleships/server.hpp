#ifndef SERVER_HPP_SENTRY
#define SERVER_HPP_SENTRY
#include <list>

#include "sockets.hpp"


enum {
    max_line_length = 1023,
    qlen_for_listen = 16
};

class Server;

class ChatSession : FdHandler {
    friend class Server;

    char buffer [max_line_length + 1];
    int buf_used;
    bool ignoring;

    int id;

    Server *the_master;

    ChatSession(Server *a_master, int fd);
    ~ChatSession();

    void Send(const char *msg);

    virtual void Handle(bool r, bool w);

    void ReadAndIgnore();
    void ReadAndCheck();
    void CheckMessage();
};

class Server : public FdHandler {
    EventSelector *the_selector;
    std::list <ChatSession*> sessions;


    Server(EventSelector *sel, int fd);
public:

    static Server *Start(EventSelector *sel, int port);
    void RemoveSession(ChatSession *s);
    ~Server();
private:
    virtual void Handle(bool r, bool w);
};

#endif
