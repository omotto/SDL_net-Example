#include "SDL.h"
#include "SDL_net.h"

#include <iostream>
#include <sstream>

class UDPConnection {
	public:
		UDPConnection(const std::string &ip, int32_t remotePort, int32_t localPort);
		~UDPConnection();
		int getError(void);
		int createPacket(int packetSize);
		int sendData(const std::string &str);
		int checkForData(void);
		UDPpacket * getPacket(void);
	private:
		int error;
		UDPsocket ourSocket;
		IPaddress serverIP;
		UDPpacket *packet;
};

UDPConnection::UDPConnection(const std::string &ip, int32_t remotePort, int32_t localPort) {
	this->error = 0;
	std::cout << "Initializing SDL_net...\n";
	if (SDLNet_Init() == -1) {
		std::cout << "SDLNet_Init failed : " << SDLNet_GetError() << std::endl;
		this->error = 1; 
	} else {
		std::cout << "Success!" << std::endl;
		std::cout << "Opening local port " << localPort << "..." << std::endl;
		// Sets our sovket with our local port
		this->ourSocket = SDLNet_UDP_Open(localPort);
		if (ourSocket == NULL) {
			std::cout << "SDLNet_UDP_Open failed : " << SDLNet_GetError() << std::endl;
			this->error = 2;
		} else {
			std::cout << "Success!" << std::endl;
			std::cout << "Setting IP ( " << ip << " ) " << "and remote port ( " << remotePort << " )" << std::endl;
			// Set IP and remote port number with correct endianess
			if (SDLNet_ResolveHost(&this->serverIP, ip.c_str(), remotePort)  == -1) {
				std::cout << "SDLNet_ResolveHost failed : " << SDLNet_GetError() << std::endl;
				this->error = 3;
			} else {
				std::cout << "Success!" << std::endl;
			}
		}
	}
}
	
UDPConnection::~UDPConnection() {
	SDLNet_FreePacket(this->packet);
	SDLNet_Quit();
}

int UDPConnection::getError(void) {
	return this->error;
}

int UDPConnection::createPacket(int packetSize) {
	std::cout << "Creating packet with size " << packetSize << "..." << std::endl;
	// Allocate memory for the packet
	this->packet = SDLNet_AllocPacket(packetSize);
	if (packet == NULL) {
		std::cout << "SDLNet_AllocPacket failed : " << SDLNet_GetError() << std::endl;
		return -1; 
	}
	// Set the destination host and port -> We got these from calling SetIPAndPort()
	packet->address.host = this->serverIP.host; 
	packet->address.port = this->serverIP.port;
	std::cout << "Success!" << std::endl;
	return 0;
}

int UDPConnection::sendData(const std::string &str) {
	int numPackets;
	// UDPPacket::data is an Uint8, which is similar to char* -> This means we can't set it directly.
	memcpy(packet->data, str.c_str(), str.length() );
	packet->len = str.length();
	std::cout << "Sending : \'" << str << "\', Length : " << packet->len << std::endl;
	// SDLNet_UDP_Send returns number of packets sent. 0 means error
	if ((numPackets = SDLNet_UDP_Send(this->ourSocket, -1, this->packet)) == 0) {
		std::cout << "SDLNet_UDP_Send failed : " << SDLNet_GetError() << std::endl;
		return -1; 
	}
	std::cout << numPackets << " packets sent successfully!" << std::endl;
	return numPackets;
}

int UDPConnection::checkForData(void) {
	int numrecv;
	std::cout << "Check for data..." << std::endl;
	// Check t see if there is a packet wauting for us...
	numrecv = SDLNet_UDP_Recv(this->ourSocket, this->packet);
	switch (numrecv) {
		case 0:
			std::cout << "No data received!" << std::endl;
		break;
		case 1:
			std::cout << "Data received: " << this->packet->data << std::endl;
		break;
		case -1:
			std::cout << "SDLNet_UDP_Recv failed : " << SDLNet_GetError() << std::endl;
		break;
		default:
			std::cout << "Unexpected return value : " << numrecv << std:endl;
		break;
	}
	return numrecv;
}

UDPpacket * UDPConnection::getPacket(void) {
	return this->packet;
}

int main(int argc, char **argv) {
	std::string IP = "127.0.0.1";
	int32_t localPort = 1230;
	int32_t remotePort = 1230;
	
	UDPConnection udpConnection(IP, remotePort, localPort);

	udpConnection.createPacket(512);
	udpConnection.sendData("This is a test");
	udpConnection.checkForData();
		
	std::cout << "Received packet message : " << udpConnection.getPacket()->data << std::endl;
	std::cout << "Received packet length  : " << udpConnection.getPacket()->len << std::endl;
	
	return 0;
}

