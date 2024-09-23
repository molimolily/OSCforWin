#pragma once
#include <oscpp/server.hpp>
#include <array>
#include <vector>
#include <WinSock2.h>
#include <stdexcept>
#include <thread>
#include <functional>
#include <atomic>
#include <chrono>

namespace OSCforWin {
	constexpr size_t MAX_PACKET_SIZE = 8192;

	/**
	 * @class OSCReceiver
	 * @brief Singleton class for receiving OSC messages over UDP.
	 */
	class OSCReceiver
	{
	public:
		using MessageCallback = std::function<void(const OSCPP::Server::Packet&& packet, const sockaddr_in& clientAddr)>;

		/**
		 * @brief Gets the singleton instance of OSCReceiver.
		 * @return Reference to the singleton instance of OSCReceiver.
		 */
		static OSCReceiver& getInstance()
		{
			static OSCReceiver instance;
			return instance;
		}

		/**
		 * @brief Sets the port for the OSC server and binds the socket.
		 * @param port The port number to bind the socket to.
		 * @throws std::runtime_error if binding the socket fails.
		 */
		void setPort(int port)
		{
			serverAddr.sin_port = htons(port);

			if (bind(sockfd, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
			{
				throw std::runtime_error("Failed to bind socket!");
			}

			portSet = true;
		}

		/**
		 * @brief Starts the receiver thread to listen for incoming OSC messages.
		 * @param callback The callback function to handle received messages.
		 * @throws std::runtime_error if the port is not set.
		 */
		void Start(MessageCallback callback)
		{
			if (!portSet)
			{
				throw std::runtime_error("Port is not set!");
			}

			running.store(true);
			receiverThread = std::thread(&OSCReceiver::receiveLoop, this, callback);
		}

		/**
		 * @brief Stops the receiver thread.
		 */
		void Stop()
		{
			running.store(false);
			if (receiverThread.joinable())
			{
				receiverThread.join();
			}
		}

		OSCReceiver(const OSCReceiver&) = delete;
		OSCReceiver& operator=(const OSCReceiver&) = delete;
	private:
		OSCReceiver()
		{
			WSADATA wsaData;
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			{
				throw std::runtime_error("Failed to initialize WinSock!");
			}

			sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (sockfd == INVALID_SOCKET)
			{
				WSACleanup();
				throw std::runtime_error("Failed to create socket!");
			}

			// Set socket to non-blocking mode
			u_long mode = 1;
			ioctlsocket(sockfd, FIONBIO, &mode);

			serverAddr.sin_family = AF_INET;
			serverAddr.sin_addr.s_addr = INADDR_ANY;
		}

		~OSCReceiver()
		{
			Stop();
			if (sockfd != INVALID_SOCKET)
			{
				closesocket(sockfd);
			}
			WSACleanup();
		}

		/**
		 * @brief The main loop for receiving OSC messages.
		 * @param callback The callback function to handle received messages.
		 */
		void receiveLoop(MessageCallback callback)
		{
			sockaddr_in clientAddr{};
			int clientAddrSize = sizeof(clientAddr);
			std::array<char, MAX_PACKET_SIZE> buffer;

			while (running)
			{
				int bytesReceived = recvfrom(sockfd, buffer.data(), MAX_PACKET_SIZE, 0, (SOCKADDR*)&clientAddr, &clientAddrSize);
				if (bytesReceived > 0)
				{
					std::vector<char> packetData(buffer.data(), buffer.data() + bytesReceived);
					callback(OSCPP::Server::Packet(packetData.data(), bytesReceived), clientAddr);
				}
				else if (bytesReceived == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
					}
					else
					{
						throw std::runtime_error("recvfrom failed!");
					}
				}
			}
		}

		SOCKET sockfd;
		sockaddr_in serverAddr{};
		bool portSet = false;
		std::thread receiverThread;
		std::atomic<bool> running{ false };
	};
}
