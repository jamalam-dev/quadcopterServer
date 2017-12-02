#pragma once


typedef struct control_packet {

	double pitch; // rad/s
	double roll; // rad/s
	double yaw; // rad/s
	double thrust; // m/s^2 (acceleration), though we might change this to the bias value as the design progresses

				   // these are ENGINEERNG units (i.e., they have already been processed by the host server from joystick units to physical units)

} control_packet;
