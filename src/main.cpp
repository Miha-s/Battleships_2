#include <stdio.h>
#include "../Battleships/server.hpp"

static int port = 7777;

int main()
{
    EventSelector *selector = new EventSelector;
    Server *serv = Server::Start(selector, port);
    if(!serv) {
        perror("sever");
        return 1;
    }

    selector->Run();
    return 0;
}
