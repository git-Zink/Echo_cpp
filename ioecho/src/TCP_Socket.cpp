#include "TCP_Socket.h"
#include <iostream>

int TCP_socket::save_data(const char *ip, in_port_t port)
{
    return internal_save_data (ip, port, SOCK_STREAM);
}

int TCP_socket::run_as_server()
{
    int reuseaddr = 1;

    switch (stage) {
    case SOCKET_STAGE::DATA_SAVED:
        if (create_socket() != 0) {
            std::cerr << "[TCP_socket::run_as_server] create_socket failed\n";
            return 4;
        }
        //no break, it's ok

    case SOCKET_STAGE::CREATED:
        if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) != 0) {
            std::cerr << "[TCP_socket::run_as_server] setsockopt : " << strerror(errno) <<"\n";
        }

        if (bind (fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1) {
            std::cerr << "[TCP_socket::run_as_server] bind : " << strerror(errno) << "\n";
            return 1;
        }

        if (listen (fd, 10) == -1) {
            std::cerr << "[TCP_socket::run_as_server] listen : " << strerror(errno) << "\n";
            return 2;
        }

        stage = SOCKET_STAGE::RUNNED;
        type = SOCKET_TYPE::SERVER;
        return 0;

    default:
        std::cerr << "[TCP_socket::run_as_server] wrong stage  \n";
        return 3;
    };
}

int TCP_socket::run_as_client()
{
    switch (stage) {
    case SOCKET_STAGE::DATA_SAVED:
        create_socket();
        //no break, it's ok

    case SOCKET_STAGE::CREATED:
        if (connect (fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr)) == -1 && errno != EINPROGRESS) {
            std::cerr << "[TCP_socket::run_as_client] connect : " << strerror(errno) << "\n";
            return 2;
        }
        stage = SOCKET_STAGE::RUNNED;
        type = SOCKET_TYPE::CLIENT;
        return 0;

    case SOCKET_STAGE::RUNNED:
        //do nothing
        return 0;

    default:
        std::cerr << "[TCP_socket::run_as_client] wrong stage  \n";
        return 1;
    };
}

ECHO_RET_CODE TCP_socket::accept_new_connection(std::unique_ptr<Socket> &src_sock)
{
    struct sockaddr_in addr;
    int new_fd;
    socklen_t addr_len = sizeof(struct sockaddr);
    
    if ((new_fd = accept (fd, reinterpret_cast<struct sockaddr*>(&addr), &addr_len)) == -1) {
        std::cerr << "[TCP_socket::accept_new_connection] accept : " << strerror(errno) << "\n";
        return ECHO_RET_CODE::FAIL;
    }
    
    src_sock.reset (new TCP_socket);
    src_sock->load_socket(new_fd, SOCKET_TYPE::CLIENT, &addr);
    
    std::cout << "[TCP_socket::accept_new_connection]\n";
    return ECHO_RET_CODE::NEW;
}

ECHO_RET_CODE TCP_socket::read_data(std::shared_ptr<Data_Container>& throwing_data)
{
    ssize_t rr;
    if (data->estimate_size == 0) {
        rr = read (fd, data->get_place_to_read(), sizeof(tcp_header_t) - data->actual_size);
    }
    else {
        rr = read (fd, data->get_place_to_read(), data->get_size_to_read());
    }
    
    if (rr == -1 && (errno == EINTR)) {
        return ECHO_RET_CODE::IN_PROGRESS;
    }
    else if (rr == 0 || rr == -1) {
        return ECHO_RET_CODE::CLOSE;
    }
    
    std::cout << "[Client_TCP_socket::handle_incoming_data] read " << rr << " bytes\n";
    
    data->actual_size += static_cast<size_t>(rr);
    
    if (data->estimate_size == 0) {
        if (data->actual_size >= sizeof(tcp_header_t)) {
            data->update_estimate((reinterpret_cast<tcp_header_t*>(data->pool))->message_length);
            std::cout << "[Client_TCP_socket::handle_incoming_data] read estimate_size = " << data->estimate_size << "\n";
            data->header_size = sizeof(tcp_header_t);
        }
    }
    throwing_data = data;
    return (data->is_data_complete_read()) ? ECHO_RET_CODE::OK : ECHO_RET_CODE::IN_PROGRESS;
}

TCP_socket::TCP_socket()
{
    data.reset (new Data_Container);
}

TCP_socket::~TCP_socket()
{
    std::cout << "[TCP_socket::~TCP_socket]\n";
    stop_socket();
}


ECHO_RET_CODE TCP_socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, std::shared_ptr<Data_Container>& throwing_data)
{
    switch (type) {
    case SOCKET_TYPE::SERVER:
        throwing_data.reset();
        return accept_new_connection (src_sock);
        
    case SOCKET_TYPE::CLIENT:
        src_sock.reset (nullptr);
        return read_data (throwing_data);

    case SOCKET_TYPE::OTHER:
        std::cerr << "[TCP_socket::handle_incoming_data] wrong socket type\n";
        return ECHO_RET_CODE::FAIL;
    };

    //damn compile warning
    return ECHO_RET_CODE::FAIL;
}

ECHO_RET_CODE TCP_socket::write_into_socket(std::shared_ptr<Data_Container> outcome_data)
{
    ssize_t rr;

    if (outcome_data != nullptr && outcome_data != data) {
        data = outcome_data;
    }
    
    if (data->header_size == 0) {
        //tcp header wasn't added
        tcp_header_t hdr;
        hdr.message_length = data->actual_size;
        
        rr = write (fd, &hdr, sizeof(tcp_header_t));

        if (rr == -1 && errno != EINPROGRESS && errno != EINTR) {
            std::cerr << "[TCP_socket::write_into_socket] wrtie header : " << strerror(errno) << "\n";
            return ECHO_RET_CODE::FAIL;
        }
        
        
        if (rr == -1) {
            std::cerr << "[TCP_socket::write_into_socket] write " << strerror(errno) << "\n";
            return ECHO_RET_CODE::IN_PROGRESS;
        }        
        
        
        if (static_cast<size_t>(rr) < sizeof(tcp_header_t)) {
            //please, write header in one shoot
            std::cerr << "[TCP_socket::write_into_socket] can't wrtie full header \n";
            return ECHO_RET_CODE::FAIL;
        }
        
        std::cout << "[TCP_socket::write_into_socket] write header " << rr << " bytes\n";
        data->header_size = sizeof(tcp_header_t);
    }
    
    if ((rr = write (fd, data->get_place_to_write(), data->get_size_to_write())) == -1) {
        if (errno == EINTR) {
            return ECHO_RET_CODE::IN_PROGRESS;
        }
        
        std::cerr << "[TCP_socket::write_into_socket] wrtie data : " << strerror(errno) << "\n";
        return ECHO_RET_CODE::FAIL;
    }
    
    std::cout << "[TCP_socket::write_into_socket] write " << rr << " bytes, exptected " << data->get_size_to_write() << "\n";
    data->write_index += static_cast<size_t>(rr);
    
    if (data->is_data_complete_write()) {
        data->reset();
        return ECHO_RET_CODE::OK;
    }
    
    return ECHO_RET_CODE::IN_PROGRESS;
}
