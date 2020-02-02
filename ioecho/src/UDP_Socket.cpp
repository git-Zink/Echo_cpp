#include <UDP_Socket.h>
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


int UDP_socket::write_into_socket (const char *buf, size_t size)
{
    char chunk [udp_frame_size];

    size_t count = size / data_size + ((size % data_size) ? 1 : 0);
    size_t iter;

    iter = 0;

    ((udp_header*)chunk)->is_last = 0;
    for (size_t  i = 0; i < count - 1; ++i, iter += data_size) {
        memcpy (chunk + sizeof(udp_header), &buf[iter], data_size);
        if (internal_write_chunk (chunk, udp_frame_size) != 0) {
            return 1;
        }
    }

    ((udp_header*)chunk)->is_last = 1;
    memcpy (chunk + sizeof(udp_header), &buf[iter], size - iter);
    return internal_write_chunk (chunk, size - iter + sizeof(udp_header));
}
