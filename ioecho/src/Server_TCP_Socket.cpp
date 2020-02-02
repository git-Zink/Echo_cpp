#include <Server_TCP_Socket.h>
#include <Client_TCP_Socket.h>

Server_TCP_socket::~Server_TCP_socket()
{
    internal_stop_socket();
}

int Server_TCP_socket::run_socket()
{
    int reuseaddr = 1;

    switch (stage) {
    case SOCKET_STAGE::DATA_SAVED:
        if (create_socket() != 0) {
            std::cerr << "[Server_TCP_socket::run_socket] create_socket failed\n";
            return 4;
        }
        //no break, it's ok

    case SOCKET_STAGE::CREATED:
        if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) != 0) {
            std::cerr << "[Server_TCP_socket::run_socket] setsockopt : " << strerror(errno) <<"\n";
        }

        if (bind (fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1) {
            std::cerr << "[Server_TCP_socket::run_socket] bind : " << strerror(errno) << "\n";
            return 1;
        }

        if (listen (fd, 10) == -1) {
            std::cerr << "[Server_TCP_socket::run_socket] listen : " << strerror(errno) << "\n";
            return 2;
        }

        stage = SOCKET_STAGE::RUNNED;
        return 0;

    default:
        std::cerr << "[Server_TCP_socket::run_socket] wrong stage  \n";
        return 3;
    };
}

ECHO_SOCKET_ACTION Server_TCP_socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, char *buf, size_t &len)
{
    struct sockaddr_in addr;
    int new_fd;
    socklen_t addr_len = sizeof(struct sockaddr);
    
    if ((new_fd = accept (fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len)) == -1) {
        std::cerr << "[Server_TCP_socket::handle_incoming_data] accept : " << strerror(errno) << "\n";
        return ECHO_SOCKET_ACTION::NONE;
    }
    
    src_sock.reset (new Client_TCP_socket);
    src_sock->load_socket(new_fd, &addr);
    
    std::cout << "[Server_TCP_socket::handle_incoming_data]\n";
    return ECHO_SOCKET_ACTION::ADD_NEW_SOCKET;
}
