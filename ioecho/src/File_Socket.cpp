#include "File_Socket.h"

File_Socket::File_Socket()
{
    input_data.reset (new Data_Container);
}

ECHO_RET_CODE File_Socket::handle_incoming_data(std::unique_ptr<Socket> &src_sock, std::shared_ptr<Data_Container> &throwing_data)
{
    ssize_t rr;
    rr = read (fd, input_data->pool, input_data->get_size_to_read());
    if (rr == -1 && errno == EINTR) {
        return ECHO_RET_CODE::IN_PROGRESS;
    }

    if (rr == -1 || rr == 0) {
        return ECHO_RET_CODE::CLOSE;
    }

    input_data->actual_size += static_cast<size_t> (rr);
    std::cout << "[File_Socket::handle_incoming_data] read " << input_data->actual_size << " bytes\n";
    throwing_data = input_data;
    src_sock.reset (nullptr);
    return ECHO_RET_CODE::OK;
}
