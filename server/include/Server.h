#ifndef SERVER_H
#define SERVER_H

#include "Server_TCP_Socket.h"
#include "Server_UDP_Socket.h"

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

class Server
{
private:
    static uint8_t need_stop;

    using sock_list_node = std::unique_ptr<Socket>;
    using sock_list_iter = std::list <sock_list_node>::iterator;

    std::list <sock_list_node> sock_list;
    std::vector <struct pollfd> poll_list;
    std::map <addr_entry, std::string> saved_data;

    static constexpr int poll_timeout = 1000;
    static constexpr size_t buffer_size = 70000;
    char buffer [buffer_size];
    
    void sync_two_lists ();
    void remove_socket (sock_list_iter iter);

    inline void handle_server_tcp_data(std::unique_ptr<Socket> &sock);
    void handle_server_udp_data(std::unique_ptr<Socket> &sock, char *buf, size_t len, bool is_last);
    void handle_client_tcp_data(sock_list_iter iter, char *buf, size_t len);
    
public:
    static void signal_handle (int signal);
    
    int make_socket (const char *ip, in_port_t port);
    int do_routine ();
};

#endif // SERVER_H
