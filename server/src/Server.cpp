#include <Server.h>
#include <Client_TCP_Socket.h>
#include <set>

uint8_t Server::need_stop;

void work_with_numbers (const char *str, size_t size)
{
    static std::map <char, uint8_t> values {
        {'0', 0},
        {'1', 1},
        {'2', 2},
        {'3', 3},
        {'4', 4},
        {'5', 5},
        {'6', 6},
        {'7', 7},
        {'8', 8},
        {'9', 9}
    };

    std::multiset <int> numbers;
    int sum = 0;

    for (size_t i = 0; i < size; ++i) {
        auto find = values.find(str[i]);
        if (find != values.end()) {
            numbers.insert(find->second);
            sum += find->second;
        }
    }

    std::cout << "\n**********************************************************\n";

    if (numbers.size() != 0) {
        std::cout << "[work_with_numbers] sum = " << sum << "\n";
        std::cout << "[work_with_numbers] sequence: ";
        for (auto iter : numbers) {
            std::cout << iter << " ";
        }
        std::cout << "\n";

        std::cout << "[work_with_numbers] max = " << *numbers.crbegin() << " min = " << *numbers.cbegin() << "\n";
    }
    else {
        std::cout <<"[work_with_numbers] sequence is empty\n";
    }

    std::cout << "**********************************************************\n\n";
}

void Server::sync_two_lists ()
{
    poll_list.resize (sock_list.size());
    
    size_t i = 0;
    auto iter = sock_list.begin();

    for (; i < sock_list.size(); ++i, ++iter) {
        if (poll_list[i].fd != (*iter)->get_fd()) {
            poll_list[i].fd = (*iter)->get_fd();
            poll_list[i].events = POLLIN;
        }
    }
}

void Server::remove_socket(sock_list_iter iter)
{
    sock_list.erase (iter);
}

void Server::handle_server_tcp_data(std::unique_ptr<Socket>& sock)
{
    sock_list.emplace_back (sock.release());
}

void Server::handle_server_udp_data(std::unique_ptr<Socket>& sock, char *buf, size_t len, bool is_last)
{
    if (!is_last) {
        saved_data [static_cast<addr_entry>(*sock)].append(buf, len);
    }
    else {
        auto iter = &saved_data [static_cast<addr_entry>(*sock)];

        iter->append(buf, len);
        work_with_numbers (iter->c_str(), iter->length());
        sock->write_into_socket(iter->c_str(), iter->length());

        saved_data.erase(static_cast<addr_entry>(*sock));
    }
}

void Server::handle_client_tcp_data(sock_list_iter iter, char *buf, size_t len)
{
    work_with_numbers (buf, len);
    (*iter)->write_into_socket(buf, len);
    (*iter)->stop_socket();
    remove_socket (iter);
}

void Server::signal_handle(int signal)
{
    if (signal == SIGINT) {
        Server::need_stop = 1;
    }
}

int Server::make_socket (const char *ip, in_port_t port)
{
    sock_list.emplace_back (new Server_TCP_socket);
    if (sock_list.back()->save_data (ip, port) != 0) {
        std::cerr << "[Server::make_socket] fail create tcp socket\n";
        return 1;
    }

    sock_list.emplace_back (new Server_UDP_socket);
    if (sock_list.back()->save_data (ip, port) != 0) {
        std::cerr << "[Server::make_socket] fail create udp socket\n";
        return 2;
    }
    
    return 0;
}

int Server::do_routine ()
{
    struct pollfd poll_item;
    
    for (auto &iter : sock_list) {
        if (iter->run_socket() != 0) {
            std::cerr << "[Server::do_routine] run_socket failed. skip\n";
            continue;
        }

        poll_item.fd = iter->get_fd();
        poll_item.events = POLLIN;

        poll_list.push_back (poll_item);
    }

    if (poll_list.size() == 0) {
        std::cerr << "[Server::do_routine] no socket has been run. interrupt\n";
        return 1;
    }

    while (need_stop == 0) {
        int res = poll (&poll_list[0], poll_list.size(), poll_timeout);

        if (res == 0 || (res == -1 && errno == EINTR)) {
            continue;
        }

        if (res == -1) {
            std::cerr << "[Server::do_routine] poll : " << strerror(errno) << "\n";
            break;
        }

        auto sock_iter = sock_list.begin();
        for (size_t i = 0; i < poll_list.size(); ++i, ++sock_iter) {
            if ((poll_list[i].revents & POLLIN) != 0) {

                std::unique_ptr<Socket> sock_arg;
                size_t rr = buffer_size - 1;

                switch ((*sock_iter)->handle_incoming_data(sock_arg, buffer, rr)) {
                case ECHO_SOCKET_ACTION::NONE:
                case ECHO_SOCKET_ACTION::KEEP_TRYING:
                    //server socket error = ignore OR EAGAIN + EWOULDBLOCK
                    break;

                case ECHO_SOCKET_ACTION::ADD_NEW_SOCKET:
                    handle_server_tcp_data (sock_arg);
                    break;

                case ECHO_SOCKET_ACTION::ONE_SHOOT_ACTION:
                    handle_client_tcp_data (sock_iter, buffer, rr);
                    break;

                case ECHO_SOCKET_ACTION::SAVE_SOCKET_AND_DATA:
                    handle_server_udp_data (sock_arg, buffer, rr, false);
                    break;

                case ECHO_SOCKET_ACTION::ANSWER_AND_DELETE:
                    handle_server_udp_data (sock_arg, buffer, rr, true);
                    break;

                case ECHO_SOCKET_ACTION::DELETE_SOCKET:
                    remove_socket (sock_iter);
                    break;
                };
            }
        }

        sync_two_lists ();
    }
    
    for (auto &iter : sock_list) {
        iter->stop_socket();
    }
    
    return 0;
}
