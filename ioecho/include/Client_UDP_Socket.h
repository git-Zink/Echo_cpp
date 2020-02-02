#ifndef CLIENT_UDP_SOCKET_H
#define CLIENT_UDP_SOCKET_H

#include "UDP_Socket.h"

class Client_UDP_socket : public UDP_socket
{
public:
    virtual ~Client_UDP_socket() override = default;
    virtual int run_socket () override;
    virtual ECHO_SOCKET_ACTION handle_incoming_data (std::unique_ptr<Socket>& src_sock, char *buf, size_t &len) override;
    virtual int stop_socket () override;
};

#endif //CLIENT_UDP_SOCKET_H
