#ifndef CLIENT_TCP_SOCKET_H
#define CLIENT_TCP_SOCKET_H

#include "TCP_Socket.h"

class Client_TCP_socket : public TCP_socket
{
public:
    virtual ~Client_TCP_socket() override = default;
    virtual int run_socket () override;
    virtual ECHO_SOCKET_ACTION handle_incoming_data (std::unique_ptr<Socket>& src_sock, char *buf, size_t &len) override;
    virtual int stop_socket () override;
};

#endif //CLIENT_TCP_SOCKET_H
