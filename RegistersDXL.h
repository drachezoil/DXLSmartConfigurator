/*
 * AX12.h
 *
 *  Created on: 19 ene. 2021
 *      Author: Josue
 */

#ifndef REGISTERSDXL_H_
#define REGISTERSDXL_H_

char registerDescription[90][30] = {
    "MODEL_NUMBER         ",
    "MODEL_INFORMATION    ",
    "FIRMWARE_VERSION     ",
    "PROTOCOL_VERSION     ",
    "ID                   ",
    "SECONDARY_ID         ",
    "BAUD_RATE            ",
    "DRIVE_MODE           ",
    "CONTROL_MODE         ",
    "OPERATING_MODE       ",
    "CW_ANGLE_LIMIT       ",
    "CCW_ANGLE_LIMIT      ",
    "TEMPERATURE_LIMIT    ",
    "MIN_VOLTAGE_LIMIT    ",
    "MAX_VOLTAGE_LIMIT    ",
    "PWM_LIMIT            ",
    "CURRENT_LIMIT        ",
    "VELOCITY_LIMIT       ",
    "MAX_POSITION_LIMIT   ",
    "MIN_POSITION_LIMIT   ",
    "ACCELERATION_LIMIT   ",
    "MAX_TORQUE           ",
    "HOMING_OFFSET        ",
    "MOVING_THRESHOLD     ",
    "MULTI_TURN_OFFSET    ",
    "RESOLUTION_DIVIDER   ",
    "EXTERNAL_PORT_MODE_1 ",
    "EXTERNAL_PORT_MODE_2 ",
    "EXTERNAL_PORT_MODE_3 ",
    "EXTERNAL_PORT_MODE_4 ",
    "STATUS_RETURN_LEVEL  ",
    "RETURN_DELAY_TIME    ",
    "ALARM_LED            ",
    "SHUTDOWN             ",

    "TORQUE_ENABLE        ",
    "LED                  ",
    "LED_RED              ",
    "LED_GREEN            ",
    "LED_BLUE             ",
    "REGISTERED_INSTRUCTION",
    "HARDWARE_ERROR_STATUS",
    "VELOCITY_P_GAIN      ",
    "VELOCITY_I_GAIN      ",
    "POSITION_P_GAIN      ",
    "POSITION_I_GAIN      ",
    "POSITION_D_GAIN      ",
    "FEEDFORWARD_1ST_GAIN ",
    "FEEDFORWARD_2ND_GAIN ",
    "P_GAIN               ",
    "I_GAIN               ",
    "D_GAIN               ",
    "CW_COMPLIANCE_MARGIN ",
    "CCW_COMPLIANCE_MARGIN",
    "CW_COMPLIANCE_SLOPE  ",
    "CCW_COMPLIANCE_SLOPE ",
    "GOAL_PWM             ",
    "GOAL_TORQUE          ",
    "GOAL_CURRENT         ",
    "GOAL_POSITION        ",
    "GOAL_VELOCITY        ",
    "GOAL_ACCELERATION    ",
    "MOVING_SPEED         ",
    "PRESENT_PWM          ",
    "PRESENT_LOAD         ",
    "PRESENT_SPEED        ",
    "PRESENT_CURRENT      ",
    "PRESENT_POSITION     ",
    "PRESENT_VELOCITY     ",
    "PRESENT_VOLTAGE      ",
    "PRESENT_TEMPERATURE  ",
    "TORQUE_LIMIT         ",
    "REGISTERED           ",
    "MOVING               ",
    "LOCK                 ",
    "PUNCH                ",
    "CURRENT              ",
    "SENSED_CURRENT       ",
    "REALTIME_TICK        ",
    "TORQUE_CTRL_MODE_ENABLE",
    "BUS_WATCHDOG         ",
    "PROFILE_ACCELERATION ",
    "PROFILE_VELOCITY     ",
    "MOVING_STATUS        ",
    "VELOCITY_TRAJECTORY  ",
    "POSITION_TRAJECTORY  ",
    "PRESENT_INPUT_VOLTAGE",
    "EXTERNAL_PORT_DATA_1 ",
    "EXTERNAL_PORT_DATA_2 ",
    "EXTERNAL_PORT_DATA_3 ",
    "EXTERNAL_PORT_DATA_4 "
};

char AXRegisterRange[35][30] = {
		"Range: N/A              RD   ",		// 0 - Motor Model          2 Byte RD
		"Range: N/A              RD   ",		// 2 - Firmware Version 	1 Byte RD
		"Range: 0 - 253          RD/WR",		// 3 - Motor ID         	1 Byte RD/WR
		"Range: 0 - 254          RD/WR",		// 4 - Baud Rate 			1 Byte RD/WR
		"Range: 0 - 254          RD/WR",		// 5 - Return Delay Time	1 Byte RD/WR
		"Range: 0 - 1023         RD/WR",		// 6 - CW Angle Limit 		2 Byte RD/WR
		"Range: 0 - 1023         RD/WR",		// 8 - CCW Angle Limit		2 Byte RD/WR
		"Range: 0 - 150          RD/WR",		// 11 - Temperature Limit	1 Byte RD/WR
		"Range: 50 - 250         RD/WR",		// 12 - Minimum Voltage     1 Byte RD/WR
		"Range: 50 - 250         RD/WR",		// 13 - Maximum Voltage 	1 Byte RD/WR
		"Range: 0 - 1023         RD/WR",		// 14 - Torque Limit		2 Byte RD/WR
		"Range: 0 - 2            RD/WR",		// 16 - Status Return Level	1 Byte RD/WR
		"Range: 0 - 127          RD/WR",		// 17 - Alarm LED			1 Byte RD/WR
		"Range: 0 - 127          RD/WR",		// 18 - Alarm Shutdown 		1 Byte RD/WR
		"Range: 0 - 1            RD/WR",		// 19 - Operating Mode		1 Byte RD/WR
		"Range: Unknown          RD   ",		// 20 - Down Calibration	2 Byte RD
		"Range: Unknown          RD   ",		// 22 - Up Calibration 		2 Byte RD
		"Range: 0 - 1            RD/WR",		// 24 - Torque Enable 		1 Byte RD/WR
		"Range: 0 - 1            RD/WR",		// 25 - LED					1 Byte RD/WR
		"Range: 0 - 255          RD/WR",		// 26 - CW Compliance M.	1 Byte RD/WR
		"Range: 0 - 255          RD/WR",		// 27 - CCW Compliance M.	1 Byte RD/WR
		"Range: 0 - 254          RD/WR",		// 28 - CW Compliance S. 	1 Byte RD/WR
		"Range: 0 - 254          RD/WR",		// 29 - CCW Compliance S. 	1 Byte RD/WR
		"Range: 0 - 1023         RD/WR",		// 30 - Goal Position  		2 Byte RD/WR
		"Range: 0 - 1023         RD/WR",		// 32 - Moving Speed  		2 Byte RD
		"Range: 0 - 1023         RD/WR",		// 34 - Torque Limit   		2 Byte RD/WR
		"Range: 0 - 1023         RD   ",		// 36 - Present Position 	2 Byte RD
		"Range: 0 - 1023         RD   ",		// 38 - Present Speed 		2 Byte RD
		"Range: 0 - 1023         RD   ",		// 40 - Present Load 		2 Byte RD
		"Range: 50 - 250         RD   ",		// 42 - Present Voltage		1 Byte RD
		"Range: 0 - 150          RD   ",		// 43 - Present Temperature	1 Byte RD
		"Range: 0 - 1            RD/WR",		// 44 - Registered Inst.	1 Byte RD/WR
		"Range: 0 - 1            RD   ",		// 46 - Moving				1 Byte RD
		"Range: 0 - 1            RD/WR",		// 47 - Lock  				1 Byte RD/WR
		"Range: 32 - 1023        RD/WR" 		// 48 - Punch 				2 Byte RD/WR
};

char AXRegisterBytes[35] = {
		2, // Address 0 Motor Model
		1, // Address 2 Motor Firmware Version
		1, // Address 3 Motor ID
		1, // Address 4 UART Baud Rate
		1, // Address 5 Return Delay Time
		2, // Address 6 CW Angle Limit
		2, // Address 8 CCW Angle Limit
		1, // Address 11 Temperature Limit
		1, // Address 12 Min Voltage Limit
		1, // Address 13 Max Voltage Limit
		2, // Address 14 Max Torque
		1, // Address 16 Status Return Level
		1, // Address 17 Alarm LED
		1, // Address 18 Alarm Shutdown
		1, // Address 19 Operating Mode
		2, // Address 20 Down Calibration
		2, // Address 22 Up Calibration
		1, // Address 24 Torque Enable
		1, // Address 25 LED
		1, // Address 26 CW Compliance Margin
		1, // Address 27 CCW Compliance Margin
		1, // Address 28 CW Compliance Slope
		1, // Address 29 CCW Compliance Slope
		2, // Address 30 Goal Position
		2, // Address 32 Moving Speed
		2, // Address 34 Torque Limit
		2, // Address 36 Present Position
		2, // Address 38 Present Speed
		2, // Address 40 Present Load
		1, // Address 42 Present Voltage
		1, // Address 43 Present Temperature
		1, // Address 44 Registered Instruction
		1, // Address 46 Moving
		1, // Address 47 Lock
		2, // Address 48 Punch
};

char AXRegisterAddress[35] = {
		0,2,3,4,5,6,8,11,12,13,14,16,17,18,19,20,22,24,25,26,27,28,29,30,32,34,36,38,40,42,43,44,46,47,48
};

char AXRegisterLock[35] = {
		1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,1,0,0
};

#endif /* REGISTERSDXL_H_ */
