#ifndef SOCKET_H
#define SOCKET_H

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

#include <types.h>
#include <memory>

enum class SOCKET_STAGE {
    UNDEFINED,
    DATA_SAVED, // = CLOSED
    CREATED,
    RUNNED
};

using addr_entry = std::pair <in_addr_t, unsigned short>;

class Socket
{
protected:
    int protocol;
    struct sockaddr_in addr;

    SOCKET_STAGE stage;

    int fd;

    int internal_save_data (const char *ip, in_port_t port, int proto);
    virtual int create_socket () final;
    int internal_stop_socket ();

public:
    Socket();
    virtual ~Socket() = default;

    virtual void load_socket (int f, struct sockaddr_in *src_addr = nullptr) final;

    virtual int save_data (const char *ip, in_port_t port) = 0;
    virtual int run_socket () = 0;
    virtual int stop_socket () = 0;

    virtual int get_fd () const final;

    virtual ECHO_SOCKET_ACTION handle_incoming_data (std::unique_ptr<Socket>& src_sock, char *buf, size_t& len) = 0;
    virtual int write_into_socket (const char *buf, size_t size) = 0;

    explicit operator addr_entry() const;
};

#endif // SOCKET_H
