#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "Socket.h"
#include <map>

class UDP_socket : public Socket
{
private:


    /* https://ru.wikipedia.org/wiki/UDP
     * Поле, задающее длину всей датаграммы (заголовка и данных) в байтах.
     * Минимальная длина равна длине заголовка — 8 байт. Теоретически, максимальный размер поля — 65535 байт для UDP-датаграммы (8 байт на заголовок и 65527 на данные).
     * Фактический предел для длины данных при использовании IPv4 — 65507 (помимо 8 байт на UDP-заголовок требуется ещё 20 на IP-заголовок).
    */
    static constexpr size_t udp_frame_size = 65507;
    static constexpr size_t data_size = udp_frame_size - sizeof(udp_header_t);
    char common_buffer [Data_Container::max_data_size];
    std::map <addr_entry, std::shared_ptr<Data_Container>> saved_data;

    int internal_write_chunk (const char *buf, size_t size);

public:
    virtual ~UDP_socket() override;

    virtual int save_data (const char *ip, in_port_t port) override;

    virtual int run_as_server () override;
    virtual int run_as_client () override;

    virtual ECHO_RET_CODE handle_incoming_data (std::unique_ptr<Socket>& src_sock, std::shared_ptr<Data_Container>& throwing_data) override;
    virtual ECHO_RET_CODE write_into_socket (std::shared_ptr<Data_Container> outcome_data) override;
};


#endif //UDP_SOCKET_H
