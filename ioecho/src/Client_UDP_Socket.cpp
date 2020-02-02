#include "Client_UDP_Socket.h"

int Client_UDP_socket::run_socket()
{
    if (stage == SOCKET_STAGE::DATA_SAVED) {
        create_socket ();
    }

    stage = SOCKET_STAGE::RUNNED;
    return 0;
}

ECHO_SOCKET_ACTION Client_UDP_socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, char *buf, size_t &len)
{
    udp_header hdr;
    ssize_t rr;

    rr = recvfrom (fd, buf, len, 0, nullptr, nullptr);

    if (rr <= 0) {
        std::cerr << "[Client_UDP_socket::handle_incoming_data] read data failed : " << strerror(errno) << "\n";
        len = 0;
        return ECHO_SOCKET_ACTION::NONE;
    }

    std::cout << "[Client_UDP_socket::handle_incoming_data] read " << rr << " bytes\n";

    len = static_cast<size_t>(rr);

    if (len <= sizeof(hdr)) {
        std::cerr << "[Client_UDP_socket::handle_incoming_data] not enough data\n";
        src_sock .reset (nullptr);
        len = 0;
        return ECHO_SOCKET_ACTION::NONE;
    }

    memcpy (&hdr, buf, sizeof(hdr));
    memmove (buf, buf + sizeof(hdr), len - sizeof(hdr));
    len -= sizeof(hdr);

    return (hdr.is_last) ? ECHO_SOCKET_ACTION::ANSWER_AND_DELETE : ECHO_SOCKET_ACTION::SAVE_SOCKET_AND_DATA;
}

int Client_UDP_socket::stop_socket()
{
    return internal_stop_socket();
}
