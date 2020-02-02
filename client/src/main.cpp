#include <Client.h>
#include <signal.h>
#include <limits>

struct args_t {
    ECHO_PROTO proto;
    in_port_t port;
};

void check_args (int argc, char *argv[], args_t &arg)
{
    if (argc != 4) {
        std::cerr << "[check_args] run client <ip> <port> <proto = <tcp|udp>\n";
        exit (1);
    }

    unsigned long dummy_port = strtoul (argv[2], nullptr, 10);

    if (dummy_port == 0 || dummy_port > std::numeric_limits<in_port_t>().max()) {
        std::cerr << "[check_args] wrong port\n";
        exit (2);
    }

    arg.port = static_cast <in_port_t>(dummy_port);

    if (strcmp (argv[3], "tcp") == 0) {
        arg.proto = ECHO_PROTO::TCP;
    }
    else if (strcmp (argv[3], "udp") == 0) {
        arg.proto = ECHO_PROTO::UDP;
    }
    else {
        std::cerr << "[check_args] wrong proto\n";
        std::cerr << "[check_args] run client <ip> <port> <proto = <tcp|udp>\n";
        exit  (3);
    }
}

int main (int argc, char *argv[])
{
    Client clnt;
    args_t parsed_args;

    check_args (argc, argv, parsed_args);
    signal (SIGINT, Client::signal_handle);

    if (clnt.make_socket(argv[1], parsed_args.port, parsed_args.proto) != 0) {
        std::cerr << "[main] make_socket failed\n";
        return 1;
    }

    if (clnt.do_routine() != 0) {
        std::cerr << "[main] do_routine failed\n";
        return 2;
    }

    std::cout << "[main] all done\n";
    return 0;
}
