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
    input_sock->load_socket(STDIN_FILENO, SOCKET_TYPE::OTHER);

    (proto == ECHO_PROTO::UDP) ? srv_sock.reset (new UDP_socket) : srv_sock.reset (new TCP_socket);

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
            switch (input_sock->handle_incoming_data (dummy, p_data)) {
            case ECHO_RET_CODE::IN_PROGRESS:
                continue;

            case ECHO_RET_CODE::OK:
                if (srv_sock->run_as_client() != 0) {
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
            if (p_data->actual_size != 0) {
                switch (srv_sock->write_into_socket (p_data)) {
                case ECHO_RET_CODE::OK:
                    std::cerr << "[Client::do_routine] write_into_socket OK\n";
                    wait_server_input();
                    p_data->reset();
                    break;
                    
                case ECHO_RET_CODE::IN_PROGRESS:
                    std::cerr << "[Client::do_routine] write_into_socket IN_PROGRESS\n";
                    //nothing
                    break;
                    
                default:
                    std::cerr << "[Client::do_routine] write_into_socket failed\n";
                    /* close and rest */
                    srv_sock->stop_socket();
                    wait_console_input ();
                };
            }
        }

        if (plfds[poll_srv].revents & POLLIN) {
            switch (srv_sock->handle_incoming_data(dummy, p_data)) {
            case ECHO_RET_CODE::IN_PROGRESS:
                std::cout << "[Client::do_rountine] IN_PROGRESS\n";
                break;

            case ECHO_RET_CODE::OK:
                std::cout << "[Client::do_rountine] OK\n";
                p_data->pool [p_data->actual_size] = 0;
                std::cout << "[Client::do_rountine] get server answer " << p_data->actual_size - p_data->header_size << " bytes\n";
                p_data->reset();
                break;

            case ECHO_RET_CODE::CLOSE:
                std::cout << "[Client::do_rountine] socket was closed\n";
                srv_sock->stop_socket();
                wait_console_input ();
                break;

            default:
                std::cerr << "[Client::do_rountine] strange error\n";
            };
        }
    }while (!need_stop);

    srv_sock->stop_socket();
    return 0;
}
