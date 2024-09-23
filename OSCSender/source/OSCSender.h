#pragma once
#include <oscpp/client.hpp>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <string>
#include "OSCSendingData.h"

namespace OSCforWin
{
    /**
     * @class OscSender
     * @brief Singleton class for sending OSC messages over UDP.
     */
    class OscSender {
    public:
        /**
         * @brief Gets the singleton instance of OscSender.
         * @return Reference to the singleton instance of OscSender.
         */
        static OscSender& getInstance() {
            static OscSender instance;
            return instance;
        }

        /**
         * @brief Sets the address and port for the OSC server.
         * @param address The IP address of the OSC server.
         * @param port The port number of the OSC server.
         * @throws std::runtime_error if the address is invalid or not supported.
         */
        void setAddressAndPort(const std::string& address, int port) {
            serveraddr.sin_port = htons(port);
            if (inet_pton(AF_INET, address.c_str(), &serveraddr.sin_addr) <= 0) {
                throw std::runtime_error("Invalid address or address not supported. Error: " + std::to_string(WSAGetLastError()));
            }
            addressSet = true;
        }

        /**
         * @brief Sends the OSC message using the provided OSCSendingData object.
         * @param sendingData The OSCSendingData object containing the OSC message to be sent.
         * @throws std::runtime_error if the address and port are not set or if sending fails.
         */
        void send(const OSCSendingData& sendingData) {
            if (!addressSet) {
                throw std::runtime_error("Address and port not set.");
            }

            int packetSize = static_cast<int>(sendingData.packetSize());


            int result = sendto(sockfd, sendingData.data(), packetSize, 0,
                reinterpret_cast<const struct sockaddr*>(&serveraddr), sizeof(serveraddr));

            if (result == SOCKET_ERROR || result != packetSize) {
                throw std::runtime_error("Failed to send message. Error code: " + std::to_string(WSAGetLastError()));
            }
        }

        OscSender(const OscSender&) = delete;
        OscSender& operator=(const OscSender&) = delete;

    private:
        OscSender()
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                throw std::runtime_error("WSAStartup failed.");
            }

            sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sockfd == INVALID_SOCKET) {
                WSACleanup();
                throw std::runtime_error("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
            }

            serveraddr.sin_family = AF_INET;
        }

        ~OscSender() {
            if (sockfd != INVALID_SOCKET) {
                closesocket(sockfd);
            }
            WSACleanup();
        }

        SOCKET sockfd;
        sockaddr_in serveraddr{};
        bool addressSet = false;
    };
}
