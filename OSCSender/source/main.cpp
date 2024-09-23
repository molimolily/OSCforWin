#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include "OSCSender.h"

#pragma comment(lib, "Ws2_32.lib")  // required for Winsock2

constexpr auto ADDRESS = "127.0.0.1";
constexpr int PORT = 7000;

std::atomic<bool> running(true);

void signalHandler(int signum) {
    running = false;
}

int getCurrentSeconds() {
    auto now = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return static_cast<int>(seconds % 60);
}

int main() {
    try {
        // Set up signal handler for SIGINT (Ctrl+C)
        std::signal(SIGINT, [](int signum) { signalHandler(signum); });

        OSCforWin::OscSender& sender = OSCforWin::OscSender::getInstance();
        sender.setAddressAndPort(ADDRESS, PORT);

        OSCforWin::OSCSendingData sendingData;

        std::cout << "Sending OSC packets to " << ADDRESS << ":" << PORT << std::endl;
        std::cout << "Press Ctrl+C to exit." << std::endl;

        while (running) {

            int sec = getCurrentSeconds();

            sendingData.getPacket()
                .openMessage("/seconds", 1)
                .int32(sec)
                .closeMessage();
            sender.send(sendingData);
            sendingData.reset();

            // Sleep for 1 second
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Program exited successfully." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
