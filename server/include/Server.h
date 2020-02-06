#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <poll.h>
#include <iostream>
#include <map>
#include <signal.h>
#include <list>
#include <iterator>
#include <Socket.h>

class Server
{
private:
    struct list_node_t {
        std::unique_ptr <Socket> sock;
        bool is_temporary;

        list_node_t (Socket *p_sock, bool temp) : sock (p_sock), is_temporary(temp){}
    };
    
    static uint8_t need_stop;

    using list_iter_t = std::list <list_node_t>::iterator;

    std::list <list_node_t> sock_list;
    std::vector <struct pollfd> poll_list;

    static constexpr int poll_timeout = 1000;
    
    void sync_two_lists ();
    void remove_socket (list_iter_t iter);

public:
    static void signal_handle (int signal);
    
    int make_socket (const char *ip, in_port_t port);
    int do_routine ();
};

#endif // SERVER_H
