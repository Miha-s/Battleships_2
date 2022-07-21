#ifndef GAME_SESSION_HPP_SENTRY
#define GAME_SESSION_HPP_SENTRY
#include <vector>

struct GameSession {
    int p1id;
    int p2id;
    int p1field [10][10]; // 0 means nothing, 1 there is a ship
    int p2field [10][10];
    bool p1turn = true;
    bool game_end = false;
    bool p1winner;
    GameSession(int _p1id, int _p2id) :
        p1id(_p1id), p2id(_p2id) {}
};
 
// class to manipulate all games silmutaneously   
class Games {
    int last_game_id;
public:
    Games() : last_game_id(0) {}
    std::vector<GameSession> games;
    int hit(int y, int x, int pid);
    // -1 - not your turn
    // 0  - miss
    // 1  - strike

    void addGame(int p1id, int p2id); 
    int removeGame(int pid);
	int findGameByPid(int pid);

    int gameEnded(int pid); 
    // 0 - game hasn't ended
    // <id> - id of player who won
};



#endif
