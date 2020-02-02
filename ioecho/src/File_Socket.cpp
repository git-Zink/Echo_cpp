#include "File_Socket.h"

ECHO_SOCKET_ACTION File_Socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, char *buf, size_t &len)
{
    ssize_t rr;
    rr = read (fd, buf, len);

    if (rr == -1 && errno == EINTR) {
        return ECHO_SOCKET_ACTION::KEEP_TRYING;
    }

    if (rr == -1 || rr == 0) {
        return ECHO_SOCKET_ACTION::DELETE_SOCKET;
    }

    len = static_cast<size_t> (rr);
    return ECHO_SOCKET_ACTION::ONE_SHOOT_ACTION;
}
