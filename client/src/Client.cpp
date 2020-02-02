#include <Client.h>
#include <File_Socket.h>
#include <signal.h>

bool Client::need_stop;

void Client::wait_first_console_input()
{
    plfds [poll_input].fd = input_sock->get_fd();
    plfds [poll_input].events = POLLIN;

    plfds [poll_srv].fd = -1;
    plfds [poll_srv].events = 0;
}

void Client::wait_server_output()
{
    plfds [poll_input].events = 0;
    plfds [poll_srv].fd = srv_sock->get_fd();
    plfds [poll_srv].events |= POLLOUT | POLLERR | POLLHUP;
}

void Client::wait_server_input()
{
    plfds [poll_input].events = POLLIN;
    plfds [poll_srv].events = POLLIN | POLLERR | POLLHUP;
}

void Client::wait_console_input()
{
    plfds [poll_input].events = POLLIN | POLLHUP;

    plfds [poll_srv].fd = -1;
    plfds [poll_srv].events = 0;
}

void Client::signal_handle(int signal)
{
    if (signal == SIGINT) {
        Client::need_stop = true;
    }
}

int Client::make_socket(const char *ip, unsigned short port, ECHO_PROTO proto)
{
    input_sock.reset (new File_Socket);
    input_sock->load_socket(STDIN_FILENO);

    (proto == ECHO_PROTO::UDP) ? srv_sock.reset (new Client_UDP_socket) : srv_sock.reset (new Client_TCP_socket);

    if (srv_sock->save_data (ip, port) != 0) {
        std::cerr << "[Client::make_socket] failed create sockadd\n";
        return 1;
    }

    return 0;
}

int Client::do_routine()
{
    std::unique_ptr<Socket> dummy;

    wait_first_console_input ();

    do {
        int res = poll (plfds, poll_total, poll_timeout);

        if (res == 0 || (res == -1 && errno == EINTR)) {
            continue;
        }

        if (res == -1) {
            std::cerr << "[Client::do_routine] poll : " << strerror(errno) << "\n";
            break;
        }

        if ((plfds[poll_input].events & POLLHUP) && (plfds[poll_input].revents & POLLHUP)) {
            std::cout << "[Client::do_routine] poll hup\n";
            break;
        }

        if (plfds[poll_input].revents & POLLIN) {
            read_len = buffer_size - 1;

            switch (input_sock->handle_incoming_data (dummy, in_buf, read_len)) {

            case ECHO_SOCKET_ACTION::KEEP_TRYING:
                continue;

            case ECHO_SOCKET_ACTION::ONE_SHOOT_ACTION:
                if (srv_sock->run_socket() != 0) {
                    srv_sock->stop_socket();
                    std::cerr << "[Client::do_routine] run_socket failed\n";
                    continue;
                }
                wait_server_output ();
                break;

            default:
                //eof or stdin error
                need_stop = 1;
            };
        }

        if (plfds[poll_srv].revents & (POLLERR | POLLHUP)) {
            std::cout << "[Client::do_routine] srv socket incorrect state. reset\n";
            srv_sock->stop_socket();
            wait_console_input ();
            continue;
        }

        if (plfds[poll_srv].revents & POLLOUT) {
            if (read_len != 0) {
                if (srv_sock->write_into_socket (in_buf, read_len) != 0) {
                    std::cerr << "[Client::do_routine] write_into_socket failed\n";
                    /* close and rest */
                    srv_sock->stop_socket();
                    wait_console_input ();
                }
                wait_server_input();
                read_len = 0;
            }
        }

        if (plfds[poll_srv].revents & POLLIN) {
            size_t srv_len = buffer_size - 1;
            switch (srv_sock->handle_incoming_data(dummy, srv_buf, srv_len)) {
            case ECHO_SOCKET_ACTION::ONE_SHOOT_ACTION:
                //tcp
                srv_buf [srv_len] = 0;
                std::cout << "[Client::do_routine] recv from server " << srv_len << " bytes : " << srv_buf << "\n";
                //tcp wil be closed by server, udp isn't require closing
                break;

            case ECHO_SOCKET_ACTION::SAVE_SOCKET_AND_DATA:
                //udp
                srv_buf [srv_len] = 0;
//                std::cout << "[Client::do_routine] recv from server " << srv_len << " bytes : " << srv_buf << "\n";
                break;

            case ECHO_SOCKET_ACTION::ANSWER_AND_DELETE:
                //last udp
                srv_buf [srv_len] = 0;
//                std::cout << "[Client::do_routine] recv last from server " << srv_len << " bytes : " << srv_buf << "\n";

                srv_sock->stop_socket();
                wait_console_input ();
                break;

            case ECHO_SOCKET_ACTION::DELETE_SOCKET:
                //tcp
                std::cout << "[Client::do_routine] server close connection\n";
                srv_sock->stop_socket();
                wait_console_input ();
                break;

            default:
                break;
            };
        }
    }while (!need_stop);

    srv_sock->stop_socket();
    return 0;
}
