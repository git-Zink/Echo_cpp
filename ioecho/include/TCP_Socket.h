#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "Socket.h"

class TCP_socket : public Socket
{
public:
    virtual ~TCP_socket() override = default;

    virtual int save_data (const char *ip, in_port_t port) override {
        return internal_save_data (ip, port, SOCK_STREAM);
    }
    
    virtual int write_into_socket (const char *buf, size_t size) override {
        ssize_t rr;
        
        rr = write (fd, buf, size);
        
        if (rr == -1) {
            std::cerr << "[TCP_socket::write_into_socket] : " << strerror(errno) << "\n";
            return 1;
        }
        
        if (rr != size) {
            std::cerr << "[TCP_socket::write_into_socket] write less (" << rr << " < " << size << ")\n";
        }
        
        return 0;
    }
};


#endif //TCP_SOCKET_H
