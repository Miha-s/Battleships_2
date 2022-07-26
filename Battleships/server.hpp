#ifndef SERVER_HPP_SENTRY
#define SERVER_HPP_SENTRY
#include <list>

#include "sockets.hpp"
#include "game_session.hpp"
#include "../Battleships/httpmessage.hpp"


enum {
    max_mes_length = 2047,
    qlen_for_listen = 16    // size of queue for listening socket
};

class Server;

class ChatSession : FdHandler {
    friend class Server;

    char buffer [max_mes_length + 1];
    int buf_used;
    bool ignoring;
	bool in_game;
	bool current;  // to mark the descriptor waiting for response

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
    Games gms;
    int initial_id = 1;
    int first_player_id; // this player will be waiting for other player to come

    Server(EventSelector *sel, int fd);
	void registerPlayer(ChatSession* ses, Headers& user);
	void sendMes(int fd, const std::string& mes);
	void send(ChatSession* ses, const std::string& body);
	void send(int id, const std::string& body);
    void sendN(ChatSession* ses);

	ChatSession* findCurrent(int id);
	void shot(ChatSession* ses, Headers& user_heads);
    void processShot(ChatSession* ses);

    void processCookie(ChatSession* ses, Headers& user, Headers& serv);

	void processEnd(int winer, int pid, std::string& coords);
public:
	void ProcessMessage(char *str, ChatSession* ses);
    static Server *Start(EventSelector *sel, int port);
    void RemoveSession(ChatSession *s);
    ~Server();
private:
    virtual void Handle(bool r, bool w);
};

#endif
