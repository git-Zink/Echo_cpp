#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "Socket.h"

class UDP_socket : public Socket
{
protected:
    struct udp_header {
        bool is_last;
    };

private:
    /* https://ru.wikipedia.org/wiki/UDP
     * Поле, задающее длину всей датаграммы (заголовка и данных) в байтах.
     * Минимальная длина равна длине заголовка — 8 байт. Теоретически, максимальный размер поля — 65535 байт для UDP-датаграммы (8 байт на заголовок и 65527 на данные).
     * Фактический предел для длины данных при использовании IPv4 — 65507 (помимо 8 байт на UDP-заголовок требуется ещё 20 на IP-заголовок).
    */
    static constexpr size_t udp_frame_size = 65507;
    static constexpr size_t data_size = udp_frame_size - sizeof(udp_header);

    int internal_write_chunk (const char *buf, size_t size);

public:
    virtual ~UDP_socket() override = default;

    virtual int save_data (const char *ip, in_port_t port) override {
        return internal_save_data (ip, port, SOCK_DGRAM);
    }

    virtual int write_into_socket (const char *buf, size_t size) override;
};


#endif //UDP_SOCKET_H
