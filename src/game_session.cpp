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
    } else if(pid == games[pos].p2id && !games[pos].p1turn) {
        res = games[pos].p1field[y][x];
        games[pos].p1field[y][x] = 0;
		games[pos].p1turn = true;
    } else
        return -1;

    return res;
}

void Games::addGame(int p1id, int p2id)
{
    GameSession tmp(p1id, p2id);
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
