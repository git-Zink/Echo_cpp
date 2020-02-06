#ifndef CLIENT_H
#define CLIENT_H

#include <TCP_Socket.h>
#include <UDP_Socket.h>

#include <string>
#include <poll.h>
#include <vector>
#include <memory>

enum class CLIENT_STAGE {
    NOT_STARTED = 0,
    WAIT_FOR_INPUT,
    WAIT_FOR_SOCKET,
    WAIT_FOR_ANSWER,
    ERROR
};

class Client
{
private:
    static bool need_stop;

    std::unique_ptr <Socket> srv_sock;
    std::unique_ptr <Socket> input_sock;

    static constexpr int poll_timeout = 1000;
    static constexpr uint8_t poll_input = 0;
    static constexpr uint8_t poll_srv = 1;
    static constexpr uint8_t poll_total = 2;
    struct pollfd plfds [poll_total];


    std::shared_ptr <Data_Container> p_data;

    void wait_first_console_input ();
    void wait_server_output ();
    void wait_server_input ();
    void wait_console_input ();

public:
    static void signal_handle (int signal);

    int make_socket (const char *ip, unsigned short port, ECHO_PROTO proto);
    int do_routine ();
};

#endif // CLIENT_H
