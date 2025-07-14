#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Mock E2AP/FlexRIC headers for demonstration
// In a real scenario, you would include the actual FlexRIC headers
namespace E2AP {
    struct E2AP_PDU {};
    struct RIC_subscription_request {};
    struct RIC_subscription_response {};
    struct RIC_indication {};
}

// Flag to control the main loop
volatile sig_atomic_t running = 1;

// Signal handler to gracefully shut down the xApp
void signal_handler(int signum) {
    std::cout << "Caught signal " << signum << ", shutting down..." << std::endl;
    running = 0;
}

// Real function to connect to FlexRIC using TCP/SCTP (TCP for demo)
bool connect_to_flexric(const char* address, int port) {
    std::cout << "Connecting to FlexRIC at " << address << ":" << port << "..." << std::endl;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Use SOCK_STREAM for TCP, SOCK_SEQPACKET for SCTP
    if (sockfd < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return false;
    }
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address." << std::endl;
        close(sockfd);
        return false;
    }
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection to FlexRIC failed." << std::endl;
        close(sockfd);
        return false;
    }
    std::cout << "Connection established." << std::endl;
    close(sockfd); // For demo, close immediately. In real code, keep open.
    return true;
}

// Real function to send a subscription request (dummy payload)
bool subscribe_to_kpm() {
    std::cout << "Sending E2SM-KPM subscription request..." << std::endl;
    // In real code, you would encode and send a RIC_subscription_request PDU over the socket.
    // Here, just simulate a delay and success.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Subscription successful." << std::endl;
    return true;
}

// Real function to listen for indications (simulate with socket accept)
void listen_for_indications() {
    int listen_port = 36422; // Dummy port for indication simulation
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(listen_port);
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 1);

    std::cout << "Listening for RIC Indications on port " << listen_port << "..." << std::endl;
    while (running) {
        std::cout << "Waiting for RIC Indication..." << std::endl;
        // Simulate a blocking accept (in real code, use SCTP and parse E2AP_PDU)
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(listen_fd, &fds);
        timeval tv = {5, 0}; // 5 seconds timeout
        int rv = select(listen_fd + 1, &fds, NULL, NULL, &tv);
        if (rv > 0 && FD_ISSET(listen_fd, &fds)) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd >= 0) {
                std::cout << "Received RIC Indication (simulated)" << std::endl;
                // In real xApp, decode E2AP_PDU and process KPM report here
                close(client_fd);
            }
        } else {
            std::cout << "No indication received (timeout)." << std::endl;
        }
    }
    close(listen_fd);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    const char* flexric_address = std::getenv("FLEXRIC_IP");
    if (!flexric_address) {
        flexric_address = "127.0.0.1"; // Default address
    }

    std::cout << "Starting hello-xapp..." << std::endl;

    if (!connect_to_flexric(flexric_address, 36422)) { // Standard SCTP port for E2
        std::cerr << "Failed to connect to FlexRIC." << std::endl;
        return 1;
    }

    if (!subscribe_to_kpm()) {
        std::cerr << "Failed to subscribe to E2SM-KPM service." << std::endl;
        return 1;
    }

    listen_for_indications();

    std::cout << "xApp has been shut down." << std::endl;

    return 0;
}
