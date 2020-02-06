#include <Server.h>
#include <TCP_Socket.h>
#include <UDP_Socket.h>

#include <set>

uint8_t Server::need_stop;

void work_with_numbers (std::shared_ptr<Data_Container>& data)
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

    for (size_t i = data->header_size; i < data->actual_size; ++i) {
        auto find = values.find(data->pool[i]);
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
        if (poll_list[i].fd != iter->sock->get_fd()) {
            poll_list[i].fd = iter->sock->get_fd();
            poll_list[i].events = POLLIN;
        }
    }
}

void Server::remove_socket(list_iter_t iter)
{
    if (iter->is_temporary) {
        std::cout << "[Server::remove_socket] erase socket\n";
        sock_list.erase (iter);
    }
}

void Server::signal_handle(int signal)
{
    if (signal == SIGINT) {
        Server::need_stop = 1;
    }
}

int Server::make_socket (const char *ip, in_port_t port)
{
    sock_list.emplace_back (list_node_t(new TCP_socket, false));
    if (sock_list.back().sock->save_data (ip, port) != 0) {
        std::cerr << "[Server::make_socket] fail create tcp socket\n";
        return 1;
    }

    sock_list.emplace_back (list_node_t(new UDP_socket, false));
    if (sock_list.back().sock->save_data (ip, port) != 0) {
        std::cerr << "[Server::make_socket] fail create udp socket\n";
        return 2;
    }
    
    return 0;
}

int Server::do_routine ()
{
    struct pollfd poll_item;
    
    for (auto &iter : sock_list) {
        if (iter.sock->run_as_server() != 0) {
            std::cerr << "[Server::do_routine] run_as_server failed. skip\n";
            continue;
        }

        poll_item.fd = iter.sock->get_fd();
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

        auto list_iter = sock_list.begin();
        auto remove_iter = sock_list.end();
        
        for (size_t i = 0; i < poll_list.size(); ++i, ++list_iter) {
            
            if (remove_iter != sock_list.end()) {
                remove_socket(remove_iter);
                remove_iter = sock_list.end();
            }
            
            if ((poll_list[i].events & POLLOUT) && (poll_list[i].revents & POLLOUT)) {
                switch (list_iter->sock->write_into_socket(nullptr)) {
                case ECHO_RET_CODE::OK:
                    poll_list[i].events = POLLIN;
                    break;
                    
                case ECHO_RET_CODE::IN_PROGRESS:
                    //nothing
                    break;
                    
                default:
                    remove_iter = list_iter;
                };
            }
            
            if ((poll_list[i].revents & POLLIN) != 0) {

                std::unique_ptr<Socket> sock_arg;
                std::shared_ptr<Data_Container> data;
                
                switch (list_iter->sock->handle_incoming_data (sock_arg, data)) {
                case ECHO_RET_CODE::NEW:
                    sock_list.emplace_back (list_node_t{sock_arg.release(), true});
                    break;

                case ECHO_RET_CODE::OK:
                    std::cout << "[Server::do_routine] ECHO_RET_CODE::OK\n";
                    work_with_numbers (data);
                    if (sock_arg != nullptr) {
                        sock_arg->write_into_socket(data);
                        data->reset();
                    }
                    else {
                        poll_list[i].events = POLLOUT;
                    }
                    break;
                    
                case ECHO_RET_CODE::IN_PROGRESS:
                    std::cout << "[Server::do_routine] in progress. keep waiting\n";
                    break;
                    
                case ECHO_RET_CODE::FAIL:
                    //oops, but do nothing
                    break;

                case ECHO_RET_CODE::CLOSE:
                    remove_iter = list_iter;
                    break;
                }
            }
        }
        
        if (remove_iter != sock_list.end()) {
            remove_socket(remove_iter);
            remove_iter = sock_list.end();
        }

        sync_two_lists ();
    }
    
    for (auto& iter : sock_list) {
        iter.sock->stop_socket();
    }
    
    return 0;
}
