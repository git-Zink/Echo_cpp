#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "Socket.h"
#include <map>

class TCP_socket : public Socket
{
private:

    
    std::shared_ptr<Data_Container> data;
    
    ECHO_RET_CODE accept_new_connection (std::unique_ptr<Socket>& src_sock);
    ECHO_RET_CODE read_data (std::shared_ptr<Data_Container> &throwing_data);
    
public:
    TCP_socket();
    virtual ~TCP_socket() override;

    virtual int save_data (const char *ip, in_port_t port) override;
    
    virtual int run_as_server () override;
    virtual int run_as_client () override;
    
    virtual ECHO_RET_CODE handle_incoming_data (std::unique_ptr<Socket>& src_sock, std::shared_ptr<Data_Container> &throwing_data) override;
    virtual ECHO_RET_CODE write_into_socket (std::shared_ptr<Data_Container> outcome_data) override;
};


#endif //TCP_SOCKET_H
