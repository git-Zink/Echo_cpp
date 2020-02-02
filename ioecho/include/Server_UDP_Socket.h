#ifndef SERVER_UDP_SOCKET_H
#define SERVER_UDP_SOCKET_H

#include "UDP_Socket.h"

class Server_UDP_socket : public UDP_socket
{
public:
    virtual ~Server_UDP_socket() override;
    virtual int run_socket () override;
    virtual ECHO_SOCKET_ACTION handle_incoming_data (std::unique_ptr<Socket>& src_sock, char *buf, size_t &len) override;
    virtual int stop_socket () override {return 0;} //do nothing. close srv socket only in destructor
    virtual int save_data (const char *ip, in_port_t port) override {
        return internal_save_data (ip, port, SOCK_DGRAM);
    }
};

#endif //SERVER_UDP_SOCKET_H
