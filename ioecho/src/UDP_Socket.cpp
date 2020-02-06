#include <UDP_Socket.h>

int UDP_socket::save_data(const char *ip, in_port_t port) {
    return internal_save_data (ip, port, SOCK_DGRAM);
}

int UDP_socket::run_as_server()
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

int UDP_socket::run_as_client()
{
    if (stage == SOCKET_STAGE::DATA_SAVED) {
        create_socket ();
    }

    stage = SOCKET_STAGE::RUNNED;
    return 0;
}

addr_entry get_entry (struct sockaddr_in addr)
{
    return {ntohl (addr.sin_addr.s_addr), ntohs (addr.sin_port)};
}

#if 0
int UDP_socket::internal_write_chunk(const char *buf, size_t size, bool is_last)
{
    ssize_t rw;
    udp_header hdr;

    hdr.is_last = is_last;

    if ((rw = sendto (fd, &hdr, sizeof(udp_header), MSG_MORE, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr))) == -1) {
        std::cerr << "[UDP_socket::internal_write_chunk] sendto header : " << strerror(errno) << "\n";
        return 1;
    }

    if (static_cast<size_t>(rw) != sizeof(udp_header)) {
        std::cerr << "[UDP_socket::internal_write_chunk] sendto write header less\n";
        return 2;
    }

    if ((rw = sendto (fd, buf, size, 0, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr))) == -1) {
        std::cerr << "[UDP_socket::internal_write_chunk] sendto data : " << strerror(errno) << "\n";
        return 3;
    }

    if (static_cast<size_t>(rw) != size) {
        std::cerr << "[UDP_socket::internal_write_chunk] sendto write data less : " << strerror(errno) << "\n";
        return 4;
    }

    std::cout << "sendto " << sizeof(udp_header) << " + " << rw << " bytes\n";

    return 0;
}
#endif

int UDP_socket::internal_write_chunk(const char *buf, size_t size)
{
    ssize_t rw;

    if ((rw = sendto (fd, buf, size, 0, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr))) == -1) {
        std::cerr << "[UDP_socket::internal_write_chunk] sendto data : " << strerror(errno) << "\n";
        return 3;
    }

    if (static_cast<size_t>(rw) != size) {
        std::cerr << "[UDP_socket::internal_write_chunk] sendto write data less : " << strerror(errno) << "\n";
        return 4;
    }

    std::cout << "[internal_write_chunk] sendto " << rw << " bytes\n";
    return 0;
}

UDP_socket::~UDP_socket()
{
    std::cout << "[UDP_socket::~UDP_socket]\n";
}

ECHO_RET_CODE UDP_socket::handle_incoming_data(std::unique_ptr<Socket>& src_sock, std::shared_ptr<Data_Container> &throwing_data)
{
    struct sockaddr_in new_addr;
    socklen_t local_len = sizeof(struct sockaddr);

    ssize_t rr;
    size_t len;
    
    rr = recvfrom (fd, common_buffer, Data_Container::max_data_size, 0, reinterpret_cast<struct sockaddr*>(&new_addr), &local_len);

    if (rr <= 0) {
        std::cerr << "[UDP_socket::handle_incoming_data] read data failed : " << strerror(errno) << "\n";
        src_sock.reset (nullptr);
        throwing_data.reset ();
        return ECHO_RET_CODE::FAIL;
    }

    len = static_cast<size_t>(rr);
    std::cout << "[UDP_socket::handle_incoming_data] read " << len << " bytes\n";

    if (len <= sizeof(udp_header_t)) {
        std::cerr << "[UDP_socket::handle_incoming_data] not enough data\n";
        src_sock.reset (nullptr);
        throwing_data.reset ();
        return ECHO_RET_CODE::FAIL;
    }
    
    len -= sizeof(udp_header_t);


    auto& data_iter = saved_data [get_entry(new_addr)];
    
    if (data_iter == nullptr) {
        std::cout << "[UDP_socket::handle_incoming_data] new udp client\n";
        data_iter.reset (new Data_Container);
    }

    if (data_iter->get_size_to_read() < len) {
        std::cerr << "[UDP_socket::handle_incoming_data] Achtung! There are more data than allocated\n";
        len = data_iter->get_size_to_read();
    }

    memcpy (data_iter->get_place_to_read(), common_buffer + sizeof(udp_header_t), len);
    data_iter->actual_size += len;

    src_sock.reset (new UDP_socket);
    src_sock->load_socket(fd, SOCKET_TYPE::CLIENT, &new_addr);
    throwing_data = data_iter;

    return (reinterpret_cast<udp_header_t*>(common_buffer)->is_last) ? ECHO_RET_CODE::OK : ECHO_RET_CODE::IN_PROGRESS;
}

ECHO_RET_CODE UDP_socket::write_into_socket(std::shared_ptr<Data_Container> outcome_data)
{
    char chunk [udp_frame_size];
    
    size_t count = outcome_data->actual_size / data_size + ((outcome_data->actual_size % data_size) ? 1 : 0);
    size_t iter;

    iter = 0;
    
    ((udp_header_t*)chunk)->is_last = 0;
    for (size_t  i = 0; i < count - 1; ++i, iter += data_size) {
        memcpy (chunk + sizeof(udp_header_t), &outcome_data->pool[iter], data_size);
        if (internal_write_chunk (chunk, udp_frame_size) != 0) {
            return ECHO_RET_CODE::FAIL;
        }
    }

    ((udp_header_t*)chunk)->is_last = 1;
    memcpy (chunk + sizeof(udp_header_t), &outcome_data->pool[iter], outcome_data->actual_size - iter);
    internal_write_chunk (chunk, outcome_data->actual_size - iter + sizeof(udp_header_t));
    return ECHO_RET_CODE::OK;
}
