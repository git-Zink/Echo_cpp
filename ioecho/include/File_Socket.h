#ifndef FILE_SOCKET_H
#define FILE_SOCKET_H

#include "Socket.h"

class File_Socket : public Socket
{
private:
    std::shared_ptr<Data_Container> input_data;

public:
    File_Socket();
    virtual ~File_Socket () = default;

    virtual int save_data (const char *ip, in_port_t port) {return -1;}
    virtual int run_socket () {return -1;}
    virtual int run_as_server () {return -1;}
    virtual int run_as_client () {return 1;}

    virtual ECHO_RET_CODE handle_incoming_data (std::unique_ptr<Socket>& src_sock, std::shared_ptr<Data_Container>& throwing_data);
    virtual ECHO_RET_CODE write_into_socket (std::shared_ptr<Data_Container> outcome_data) {return ECHO_RET_CODE::FAIL;}
};

#endif // FILE_SOCKET_H
