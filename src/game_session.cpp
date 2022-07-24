#include "../Battleships/game_session.hpp"

int Games::hit(int y, int x, int pid)
{
    int res, pos;
	pos = findGameByPid(pid);

    // check hit
    if(pid == games[pos].p1id && games[pos].p1turn) {
        res = games[pos].p2field[y][x];
        games[pos].p2field[y][x] = 0;
		games[pos].p1turn = false;
        if(empty(games[pos].p2field)) {
            games[pos].game_end = true;
            games[pos].p1winner = true;
        }
    } else if(pid == games[pos].p2id && !games[pos].p1turn) {
        res = games[pos].p1field[y][x];
        games[pos].p1field[y][x] = 0;
		games[pos].p1turn = true;
        if(empty(games[pos].p1field)) {
            games[pos].game_end = true;
            games[pos].p1winner = false;
        }
    } else
        return -1;

    return res;
}

bool Games::empty(int (*arr)[10])
{
    for(int i = 0; i < 10; i++)
        for(int j = 0; j < 10; j++)
            if(arr[i][j] != 0)
                return false;
    return true;
}

void Games::addGame(int p1id, int p2id)
{
    GameSession tmp(p1id, p2id);
    tmp.ready = true;
    games.push_back(tmp);
}

int Games::removeGame(int pid)
{
    int gm = findGameByPid(pid);
	if(gm == -1)
		return -1;
    auto iter = games.begin();
    games.erase(iter + gm);

    return 0;
}

int Games::gameEnded(int pid) 
{
    int gm = findGameByPid(pid);
 
    if(games[gm].game_end && games[gm].p1winner)
        return games[gm].p1id;
    else if(games[gm].game_end)
        return games[gm].p2id;

    return 0;
}

int Games::findGameByPid(int pid)
{
    for(int i = 0; i < games.size(); i++)
        if(games[i].p1id == pid || games[i].p2id == pid)
            return i;
    return -1;
}

void Games::setField(std::string field, int pid)
{
	// the field string is in format of string of numbers of the field
	// from left to right, from top to bottom
    int gm = findGameByPid(pid);
    int (*f)[10] = (pid == games[gm].p1id) ? games[gm].p1field : games[gm].p2field;
    for(int i = 0; i < 10; i++)
        for(int j = 0; j < 10; j++) 
            f[i][j] = field[i*10+j] - '0';
}

void Games::addPlayer(int p1id, int p2id)
{
    int gm = findGameByPid(p1id);
    games[gm].p2id = p2id;
}

int Games::getOtherPid(int pid)
{
    int gm = findGameByPid(pid);
    if(games[gm].p1id == pid)
        return games[gm].p2id;
    else
        return games[gm].p1id;
}

bool Games::playerTurn(int pid)
{
    int gm = findGameByPid(pid);
    if(games[gm].p1id == pid && games[gm].p1turn)
        return true;
    else if(games[gm].p2id == pid && !games[gm].p1turn)
        return true;
    else 
        return false;
}

bool Games::playerReady(int pid)
{
    int gm = findGameByPid(pid);
    return games[gm].ready;
}

void Games::setReady(int pid, bool ready)
{
    int gm = findGameByPid(pid);
    games[gm].ready = ready;
}

