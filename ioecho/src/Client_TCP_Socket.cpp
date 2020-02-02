#include <Client_TCP_Socket.h>

int Client_TCP_socket::run_socket()
{
    switch (stage) {
    case SOCKET_STAGE::DATA_SAVED:
        create_socket();
        //no break, it's ok

    case SOCKET_STAGE::CREATED:
        if (connect (fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1 && errno != EINPROGRESS) {
            std::cerr << "[Client_TCP_socket::run_socket] connect : " << strerror(errno) << "\n";
            return 2;
        }
        stage = SOCKET_STAGE::RUNNED;
        return 0;

    case SOCKET_STAGE::RUNNED:
        //do nothing
        return 0;

    default:
        std::cerr << "[Client_TCP_socket::run_socket] wrong stage  \n";
        return 1;
    };
}

ECHO_SOCKET_ACTION Client_TCP_socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, char *buf, size_t &len)
{
    ssize_t rr;
    
    rr = read (fd, buf, len);
    
    switch (rr) {
    case -1:
        std::cerr << "[Client_TCP_socket::handle_incoming_data] read : " << strerror(errno) << "\n";
        len = 0;
        return ECHO_SOCKET_ACTION::DELETE_SOCKET;
        
    case 0:
        std::cout << "[Client_TCP_socket::handle_incoming_data] clnt close\n";
        len = 0;
        return ECHO_SOCKET_ACTION::DELETE_SOCKET;
        
    default:
        std::cout << "[Client_TCP_socket::handle_incoming_data] read " << rr << "\n";
        len = static_cast<size_t>(rr);
        return ECHO_SOCKET_ACTION::ONE_SHOOT_ACTION;
    };
}

int Client_TCP_socket::stop_socket()
{
    return internal_stop_socket();
}
