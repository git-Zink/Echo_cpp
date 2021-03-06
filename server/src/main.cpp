#include <Server.h>
#include <iostream>
#include <climits>

struct args_t {
    in_port_t port;
};

void check_args (int argc, char *argv[], args_t &arg)
{
    if (argc != 3) {
        std::cerr << "[check_args] run client <ip> <port> <proto = <tcp|udp>\n";
        exit (1);
    }

    unsigned long dummy_port = strtoul (argv[2], nullptr, 10);

    if (dummy_port == 0 || dummy_port > std::numeric_limits<in_port_t>().max()) {
        std::cerr << "[check_args] wrong port\n";
        exit (2);
    }

    arg.port = static_cast <in_port_t>(dummy_port);
}

int main (int argc, char *argv[])
{
    args_t parsed_args;
    Server srv;

    signal (SIGINT, Server::signal_handle);
    check_args (argc, argv, parsed_args);

    if (srv.make_socket (argv[1], parsed_args.port) != 0) {
        std::cerr << "[main] make_socket failed\n";
        return 3;
    }

    if (srv.do_routine() != 0) {
        std::cerr << "[main] do_routine failed\n";
        return 4;
    }
    
    std::cout << "[main] all done\n";
    return 0;
}
