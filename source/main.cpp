#include "packet.h"

#include <stdio.h>
#include <SFML/Window/Joystick.hpp>

#define _WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>

#define QC_ADDR "192.168.88.16" // arbitrarily decided quadcopter ip address
#define NETWORK_PORT "7501"
#define NETWORK_PORT_INT 7501

// link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib") // this is AWESOME!
#pragma comment (lib, "winmm.lib")

int main() {


	// Create socket
	SOCKET sock = INVALID_SOCKET;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	
	WSADATA wsa;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsa); // WSAStartup
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	iResult = getaddrinfo(NULL, NETWORK_PORT, &hints, &result); // getaddrinfo (NULL means local to caller)

	sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol); // socket

	if (sock == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}


	iResult = bind(sock, result->ai_addr, (int)result->ai_addrlen); // bind

	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(sock);
		WSACleanup();
		return 1;
	}


	// setup qc address
	sockaddr_in qc_addr;

	qc_addr.sin_family = AF_INET;
	qc_addr.sin_port = htons(NETWORK_PORT_INT);
	inet_pton(AF_INET, QC_ADDR, &(qc_addr.sin_addr));


	// Setup joystick from SFML
	sf::Joystick joystick;
	if (joystick.isConnected(0) == 0) { // why is this backwards? 
		printf("connected to joystick 0!\n");
	} else {
		printf("Failed to connect to Joystick 0...\n");
		freeaddrinfo(result);
		WSACleanup();
	}



	// main loop
	while (1) {

		// gather input from joystick
		joystick.update();

		double pitch = (double)joystick.getAxisPosition(0, sf::Joystick::Y);
		double roll = (double)joystick.getAxisPosition(0, sf::Joystick::X);
		double yaw = (double)joystick.getAxisPosition(0, sf::Joystick::R);
		double thrust = (double)joystick.getAxisPosition(0, sf::Joystick::Z);

		//printf("%1.6f %1.6f %1.6f %1.6f\n", pitch, roll, yaw, thrust); 


		control_packet control;

		control.pitch = pitch;
		control.roll = roll;
		control.yaw = yaw;
		control.thrust = thrust;
		

		iResult = sendto(sock, (char*)(&control), sizeof(control_packet), 0, (SOCKADDR*)(&qc_addr), sizeof(qc_addr));
		if (iResult == SOCKET_ERROR) {
			wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
			closesocket(sock);
			WSACleanup();
			return 1;
		}

	}







	freeaddrinfo(result);
	WSACleanup();

	return 0;
}