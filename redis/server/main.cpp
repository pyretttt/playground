#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>

constexpr size_t k_max_msg = 4096;

static void process(int fd) {
    char buff[64] = {0};
    ssize_t n = read(fd, buff, sizeof(buff) - 1);
    if (n < 0) {
        std::cout << "Nothing to read" << std::endl;
        return;
    }

    std::cout << "Client send: " << buff << std::endl;

    write(fd, buff, n);
}

static int32_t read_full(int fd, char *buff, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buff, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buff += rv;
    }
    return 0;
}

static int32_t write_all(int fd, const char *buff, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buff, n);
        if (rv <= 0) {
            return -1;
        }
        assert((size_t) rv <= n);
        n -= (size_t) rv;
        buff += rv;
    }
    return 0;
}

int one_request(int fd) {
    
}

int main() {
    using namespace std;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int val{1};
    int rv = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    if (rv) {
        cerr << "Failed to configure socket" << endl;
    }

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(8000);
    addr.sin_addr.s_addr = ntohl(0);

    socklen_t sock_len = sizeof(addr);
    rv = bind(fd, reinterpret_cast<const sockaddr *>(&addr), sock_len);
    if (rv) {
        cerr << "Failed to bind socket" << endl;
    }
    
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        cerr << "Failed to listen socket" << endl;
    }

    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, reinterpret_cast<sockaddr *>(&client_addr), &socklen);

        if (connfd < 0) {
            continue;
        }

        while (true) {
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    }
}
