#include <Socket.h>

Socket::Socket()
{
    stage = SOCKET_STAGE::UNDEFINED;
    fd = -1;
    protocol = -1;
}

int Socket::stop_socket()
{
    int res = 0;
    if (stage == SOCKET_STAGE::RUNNED) {
        std::cout << "[Socket::internal_stop_socket]\n";
//        res = shutdown (fd, SHUT_RDWR);
        res = close (fd);
    }

    fd = -1;
    stage = SOCKET_STAGE::DATA_SAVED;
    return res;
}

void Socket::load_socket(int f, SOCKET_TYPE t, sockaddr_in *src_addr) {
    if (stage == SOCKET_STAGE::UNDEFINED) {
        fd = f;
        if (src_addr) {
            memcpy (&addr, src_addr, sizeof(struct sockaddr_in));
        }
        protocol = -1;
        stage = SOCKET_STAGE::RUNNED;
        type = t;
    }
}

int Socket::internal_save_data(const char *ip, in_port_t port, int proto)
{
    int res;
    protocol = proto;

    memset (&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons (port);

    if (strcmp (ip, "any") == 0) {
        addr.sin_addr.s_addr = htonl (INADDR_ANY);
        res = 0;
    }
    else {
        res = (inet_pton (AF_INET, ip, &addr.sin_addr) > 0 ? 0 : 2);
    }

    stage = SOCKET_STAGE::DATA_SAVED;
    return res;
}

int Socket::create_socket()
{
    if (stage != SOCKET_STAGE::DATA_SAVED) {
        return 1;
    }

    if ((fd = socket (AF_INET, protocol, 0)) == -1) {
        std::cerr << "[Socket::create_socket] socket : " << strerror(errno) << "\n";
        return 2;
    }

    int settings = fcntl (fd, F_GETFL, 0);
    fcntl (fd, F_SETFL, settings | O_NONBLOCK);
    stage = SOCKET_STAGE::CREATED;
    return 0;
}

int Socket::get_fd () const
{
    return fd;
}
