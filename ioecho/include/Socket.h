#ifndef SOCKET_H
#define SOCKET_H

#include <Data_Container.h>

#include <types.h>
#include <memory>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>

#include <iostream>

enum class SOCKET_STAGE {
    UNDEFINED,
    DATA_SAVED, // = CLOSED
    CREATED,
    RUNNED
};

enum class SOCKET_TYPE {
    SERVER,
    CLIENT,
    OTHER
};

using addr_entry = std::pair <in_addr_t, unsigned short>;

class Socket
{
protected:
    int fd;
    struct sockaddr_in addr;
    int protocol;

    SOCKET_TYPE type;
    SOCKET_STAGE stage;
    
    int internal_save_data (const char *ip, in_port_t port, int proto);
    virtual int create_socket () final;

public:
    Socket();
    virtual ~Socket() = default;

    virtual int save_data (const char *ip, in_port_t port) = 0;
    virtual int run_as_server () = 0;
    virtual int run_as_client () = 0;
    virtual int stop_socket () final;
    
    virtual void load_socket (int f, SOCKET_TYPE t, struct sockaddr_in *src_addr = nullptr) final;
    
    int get_fd() const;


    virtual ECHO_RET_CODE handle_incoming_data (std::unique_ptr<Socket>& src_sock, std::shared_ptr<Data_Container>& throwing_data) = 0;
    virtual ECHO_RET_CODE write_into_socket (std::shared_ptr<Data_Container> outcome_data) = 0;
};

#endif // SOCKET_H
