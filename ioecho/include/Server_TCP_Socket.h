#ifndef SERVER_TCP_SOCKET_H
#define SERVER_TCP_SOCKET_H

#include "TCP_Socket.h"

class Server_TCP_socket : public TCP_socket
{
public:
    virtual ~Server_TCP_socket() override;
    virtual int run_socket () override;
    virtual ECHO_SOCKET_ACTION handle_incoming_data (std::unique_ptr<Socket>& src_sock, char *buf, size_t &len) override;
    virtual int stop_socket () override {return 0;} //do nothing. close srv socket only in destructor
};

#endif //SERVER_TCP_SOCKET_H
