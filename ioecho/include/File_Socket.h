#ifndef FILE_SOCKET_H
#define FILE_SOCKET_H

#include "Socket.h"

class File_Socket : public Socket
{
private:

public:
    virtual ~File_Socket () = default;

    virtual int save_data (const char *ip, in_port_t port) {return -1;}
    virtual int run_socket () {return -1;}
    virtual int stop_socket () {return -1;}

    virtual ECHO_SOCKET_ACTION handle_incoming_data (std::unique_ptr<Socket>& src_sock, char *buf, size_t& len);
    virtual int write_into_socket (const char *buf, size_t size) {return -1;}
};

#endif // FILE_SOCKET_H
