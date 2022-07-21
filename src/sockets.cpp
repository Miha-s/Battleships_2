#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "../Battleships/sockets.hpp"

EventSelector::~EventSelector()
{
    if(fd_array)
        delete[] fd_array;
}

void EventSelector::Add(FdHandler *h)
{
    int i;
    int fd = h->GetFd();
	// creating array of file descriptors
    if(!fd_array) {
        fd_array_len = fd > 15 ? fd + 1 : 16;
        fd_array = new FdHandler*[fd_array_len];
        for(i = 0; i < fd_array_len; i++)
            fd_array[i] = 0;
        max_fd = -1;
    }
    // resizing the array of file descriptors (if needed)
    if(fd_array_len <= fd) {
        FdHandler **tmp = new FdHandler*[fd+1];
        for(i = 0; i <= fd; i++)
            tmp[i] = i < fd_array_len ? fd_array[i] : 0;
        fd_array_len = fd + 1;
        delete[] fd_array;
        fd_array = tmp;
    }
    if(fd > max_fd)
        max_fd = fd;
    fd_array[fd] = h;
}

bool EventSelector::Remove(FdHandler *h)
{
    int fd = h->GetFd();
    if(fd >= fd_array_len || fd_array[fd] != h)
        return false;
    fd_array[fd] = 0;
    if(fd == max_fd) {
        while(max_fd >= 0 && !fd_array[max_fd])
            max_fd--;
    }
    return true;
}
