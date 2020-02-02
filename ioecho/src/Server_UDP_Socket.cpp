#include <Server_UDP_Socket.h>
#include <Client_UDP_Socket.h>
Server_UDP_socket::~Server_UDP_socket()
{
    internal_stop_socket();
}

int Server_UDP_socket::run_socket()
{
    switch (stage) {
    case SOCKET_STAGE::DATA_SAVED:
        create_socket();
        //no break, it's ok

    case SOCKET_STAGE::CREATED:
        if (bind (fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1) {
            std::cerr << "[Server_UDP_socket::run_socket] bind : " << strerror(errno) << "\n";
            return 1;
        }

        stage = SOCKET_STAGE::RUNNED;
        return 0;

    default:
        std::cerr << "[Server_UDP_socket::run_socket] wrong stage  \n";
        return 3;
    };
}

ECHO_SOCKET_ACTION Server_UDP_socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, char *buf, size_t &len)
{
    struct sockaddr_in new_addr;
    socklen_t local_len = sizeof(struct sockaddr);

    udp_header hdr;

    ssize_t rr;

    rr = recvfrom (fd, buf, len, 0, reinterpret_cast<struct sockaddr*>(&new_addr), &local_len);

    if (rr <= 0) {
        std::cerr << "[Server_UDP_socket::handle_incoming_data] read data failed : " << strerror(errno) << "\n";
        src_sock.reset (nullptr);
        len = 0;
        return ECHO_SOCKET_ACTION::NONE;
    }

    len = static_cast<size_t>(rr);

    if (len <= sizeof(hdr)) {
        std::cerr << "[Server_UDP_socket::handle_incoming_data] not enough data\n";
        src_sock.reset (nullptr);
        len = 0;
        return ECHO_SOCKET_ACTION::NONE;
    }

    memcpy (&hdr, buf, sizeof(hdr));
    memmove (buf, buf + sizeof(hdr), len - sizeof(hdr));
    len -= sizeof(hdr);

    src_sock.reset (new Client_UDP_socket);
    src_sock->load_socket(fd, &new_addr);

    std::cout << "[Server_UDP_socket::handle_incoming_data] read header : " << hdr.is_last << "\n";
    std::cout << "[Server_UDP_socket::handle_incoming_data] read " << len << " bytes\n";

    return (hdr.is_last) ? ECHO_SOCKET_ACTION::ANSWER_AND_DELETE : ECHO_SOCKET_ACTION::SAVE_SOCKET_AND_DATA;
}
