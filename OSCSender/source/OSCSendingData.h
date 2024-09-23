#pragma once
#include <oscpp/client.hpp>
#include <vector>
#include <stdexcept>

namespace OSCforWin {
	/**
	 * @class OSCSendingData
	 * @brief Manages data (packet and buffer) for sending OSC messages.
	 */
	class OSCSendingData {
	public:
		/**
		 * @brief Constructor that initializes the buffer and packet with a given buffer size.
		 * @param bufferSize The size of the buffer to be allocated (default is 1024).
		 */
		OSCSendingData(size_t bufferSize = 1024) : buffer(bufferSize), packet(buffer.data(), buffer.size()) {}

		/**
		 * @brief Returns a reference to the OSCPP::Client::Packet object.
		 * @return Reference to the OSCPP::Client::Packet object.
		 */
		OSCPP::Client::Packet& getPacket() { return packet; }

		/**
		 * @brief Returns a pointer to the data stored in the buffer.
		 * @return Pointer to the data stored in the buffer.
		 */
		const char* data() const { return buffer.data(); }

		/**
		 * @brief Returns the size of the buffer.
		 * @return Size of the buffer.
		 */
		size_t bufferSize() const { return buffer.size(); }

		/**
		 * @brief Returns the size of the packet.
		 * @return Size of the packet.
		 */
		size_t packetSize() const { return packet.size(); }

		/**
		 * @brief Resets the packet to its initial state.
		 */
		void reset() { packet.reset(); }

	private:
		std::vector<char> buffer;
		OSCPP::Client::Packet packet;
	};
}