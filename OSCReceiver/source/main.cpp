#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <oscpp/print.hpp> // required for printing OSC messages
#include "OSCReceiver.h"

#pragma comment(lib, "Ws2_32.lib")  // required for Winsock2

constexpr int PORT = 7000;

std::atomic<bool> running(true);

void signalHandler(int signum) {
    running = false;
}

/*
* Handle incoming OSC messages.
 * @param packet The OSC packet received.
 * @param clientAddr The address of the client that sent the packet.
*/
void HandleMessage(const OSCPP::Server::Packet&& packet, const sockaddr_in& clientAddr) {
    OSCPP::detail::printPacket(std::cout, packet, OSCPP::detail::Indent(1));
    std::cout << std::endl;
}


int main() {
    try {
        // Set up signal handler for SIGINT (Ctrl+C)
        std::signal(SIGINT, [](int signum) { signalHandler(signum); });

        OSCforWin::OSCReceiver& receiver = OSCforWin::OSCReceiver::getInstance();
        receiver.setPort(PORT);
        receiver.Start(HandleMessage);

        std::cout << "Receive OSC messages on port " << PORT << std::endl;
        std::cout << "Press Ctrl+C to exit." << std::endl;

        // dummy loop running until Ctrl+C is pressed
        while (running) {
            // Sleep for 1 second
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        receiver.Stop();

        std::cout << "Program exited successfully." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
