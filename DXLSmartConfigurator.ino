/*
 * Origin : Josue Alejandro Gutierrez Alvarez
 * Modification:
 *  - bring to Arduino IDE instead of Eclipse
 *  - Use a Teensy 4.0 instead of the MK26F
 *  - Use a touch screen instead of the switch -> from the ST7789V to the ILI9341
 *  - Use the Dynamixel2Arduino library instead of custom one
 * To do:
 *  - check if register exist for this model
 *  - R/W diplay of each register getModelDependencyFuncInfo
 *  - min/max of each register getModelDependencyFuncInfo
 *  - keyboard
 *    - add float capability
 *  - dynamixel factory restore
 *  - add model number on the info page
 *  - eeprom the protocol and value
 *  - test
 */
// Screen
#include "SPI.h"
#include "ILI9341_t3.h"
#include <Adafruit_STMPE610.h>
// GRAPHICS:
#include "Splashscreen.h"
#include "Interface.h"
#include "Animations.h"
//Dynamixel
#include <Dynamixel2Arduino.h>
#include "RegistersDXL.h"

#define DXL_SERIAL    Serial1
const uint8_t DXL_DIR_PIN = 2;       
const uint8_t BROADCAST_ID = 254;    
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);
using namespace ControlTableItem;

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// TFT pins
const uint8_t TFT_DC = 9;
const uint8_t TFT_CS = 10;
const uint8_t MIC_PIN = 14;
const uint8_t BACKLIGHT_PIN = 23;
// Use hardware SPI (#13, #12, #11) and the above for CS/DC
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

#define YP	0
#define	XP	1
#define YM	2
#define XM	3

#define BACKGROUND		  ILI9341_BLACK
#define dataBarColor	  ILI9341_LIGHTGREY 
#define dataColor		    ILI9341_WHITE
#define dataColorError	ILI9341_RED

#define MINMAXVAL	156

const char Baudrates[14][11] = {"10,500,000","6,000,000","4,500,000","4,000,000","3,000,000","2,000,000","1,000,000","500,000  ","400,000  ","200,000  ","115,200  ","57,600   ","19,200   ","9,600    "};
const uint32_t configBaudrate[14] = {10500000,6000000,4500000,4000000,3000000,2000000,1000000,500000,400000,200000,115200,57600,19200,9600};

unsigned int currentBaudrate = 0;
float protocol = 1.0;

float motorRegisterDecimals = 0.0;
int currentRegister = 0;
int registerData = 0;
int motorRegister = 0;
int currentMotor = 0;
int motorIndex = 0;
int motorList[255];

char motorRegisterDecimalsStr[4];
char motorRegisterBuffer[30];
char motorRegisterStr[8];

char statusBuffer[30];
char currentMotorStr[4];
char motorIndexStr[4];
char motorIDStr[4];
char model[20];

uint8_t CurrentScreen = 0;
uint8_t keyboardEnabled = 0;
uint8_t lockStatus = 1;
uint8_t motorID = 0;

TS_Point p;

void displayMainScreen();
void displayScanScreen();
void displayScanMotors();
void displayRegisterScreen();
void displayRestoreScreen(unsigned char option);
void displayConfigScreen();
void displayMotorStatus();

elapsedMillis refreshMillis = 0;
unsigned long refreshInterval = 500; //refresh ping and status every 500ms

void setup() {
  // Init DXL
  dxl.begin(configBaudrate[currentBaudrate]);
  dxl.setPortProtocolVersion(protocol);	
  	
	// Initialization for TFT Display 
  pinMode( BACKLIGHT_PIN, OUTPUT );
  analogWrite( BACKLIGHT_PIN, 1023 );
  tft.begin();
  tft.setRotation( 3 );
  tft.fillScreen(ILI9341_BLACK);
  
	tft.writeRect(0,0,240,320,(uint16_t*)StartScreen);					// Display SplashScreen
	delay(3000);											// Wait for SplashScreen

	displayMainScreen();									// Display main menu screen

}

void loop(){

  if(refreshMillis>refreshInterval){
    refreshMillis = 0;
    switch(CurrentScreen){
      case 1:               // Scan/Select Motor Screen
        if(motorIndex > 0){
          displayMotorInfo(motorIndex);
        }
      case 2:               // Register Screen
      case 3:               // Factory restore Screen
        if(motorIndex > 0){		
          motorPing(motorID);          
        }
        break;
      case 4:                //baudrate && protocol && calibration
      	displayPutStr(Baudrates[currentBaudrate],112,8,2,dataColor,dataBarColor);
      	if( protocol == 1){
      		displayPutStr("1.0",188,8,2,dataColor,dataBarColor);
      	}else if(protocol == 2){
      		displayPutStr("2.0",188,8,2,dataColor,dataBarColor);
      	}
       break;
    }
  }
  
  if (ts.touched()) {
    // Retrieve a point  
    p = ts.getPoint();
    // Scale from ~0->4000 to tft.width using the calibration #"s
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    switch(CurrentScreen){
      case 0: /* Main Screen - Menu Screen */
        if(p.y > scanY1 && p.y < scanY2 && p.x > scanX1 && p.x < scanX2){
          CurrentScreen = 1;          // Main Menu to Scan Screen
          displayScanScreen();        // Display Scan screen
          displayScanMotors();        // Start motor search
        }
        if(p.y > regY1 && p.y < regY2 && p.x > regX1 && p.x < regX2){
          CurrentScreen = 2;          // Main Menu to Register Screen
          displayRegisterScreen();    // Display Register Screen
          displayMotorStatus();       // Display motor status
        }
        if(p.y > resetY1 && p.y < resetY2 && p.x > resetX1 && p.x < resetX2){
          CurrentScreen = 3;          // Main Menu to Reset Screen
          displayRestoreScreen(1);    // Display Reset Screen
          displayMotorStatus();       // Display motor status
        }
        if(p.y > configY1 && p.y < configY2 && p.x > configX1 && p.x < configX2){
          CurrentScreen = 4;            // Main Menu to Config Screen
          displayConfigScreen();        // Display Configuration Screen
        }
        break;
      case 1: /* Scan Screen */
        if(p.y > bot1Y1 && p.y < bot1Y2 && p.x > bot1X1 && p.x < bot1X2){
          displayScanMotors();        // Start motor search
        }
        if(p.y > bot2Y1 && p.y < bot2Y2 && p.x > bot2X1 && p.x < bot2X2){
          displayMotorStatus();        // Display motor status
          if(currentMotor > 0)
            currentMotor--;           // Go to previous motor
          motorID = motorList[currentMotor];  // Update selected motor
          displayMotorStatus();       // Display motor status
        }
        if(p.y > bot3Y1 && p.y < bot3Y2 && p.x > bot3X1 && p.x < bot3X2){
          displayMotorStatus();        // Display motor status
          if(currentMotor < (motorIndex - 1))
            currentMotor++;           // Go to next motor
          motorID = motorList[currentMotor];  // Update selected motor
          displayMotorStatus();       // Display motor status
        }
        if(p.y > bot4Y1 && p.y < bot4Y2 && p.x > bot4X1 && p.x < bot4X2){
          CurrentScreen = 0;
          displayMainScreen();        // Display Main menu
          dxl.ledOff(BROADCAST_ID);   // Turn off all motors LED
        }
        break;
      case 2: /* Register Screen */
        if(p.y > bot1Y1 && p.y < bot1Y2 && p.x > bot1X1 && p.x < bot1X2){
          dxl.writeControlTableItem(currentRegister, motorID, registerData);
          displayRegisterScreen();    // Display Register Screen
          displayMotorStatus();       // Display motor status
        }
        if(p.y > bot2Y1 && p.y < bot2Y2 && p.x > bot2X1 && p.x < bot2X2){
          displayMotorStatus();        // Display motor status
          prevReg();
        }
        if(p.y > bot3Y1 && p.y < bot3Y2 && p.x > bot3X1 && p.x < bot3X2){
          displayMotorStatus();        // Display motor status
          nextReg();
        }
        if(p.y > bot4Y1 && p.y < bot4Y2 && p.x > bot4X1 && p.x < bot4X2){
          CurrentScreen = 0;
          displayMainScreen();        // Display Main menu
          dxl.ledOff(BROADCAST_ID);   // Turn off all motors LED
        }
        int keyValue = keyboardTouch();
        //display value typed
        if(keyValue != -2){
          if(keyValue == -1){ // backspace
            registerData = registerData/10;
          }
          else{
            registerData = registerData*10+keyValue;
          }
          itoa(registerData,motorRegisterStr,10);
          strcpy(motorRegisterBuffer,motorRegisterStr);
          displayPutStr(motorRegisterBuffer,100,4,2,dataColor,dataBarColor); 
        }
        break;
      case 3: /* Reset Screen */
        if(p.y > bot1Y1 && p.y < bot1Y2 && p.x > bot1X1 && p.x < bot1X2){
          if(motorIndex > 0){
            Dynamixelreset(motorID); /*  TO CHANGE  */
            displayPutStr("Scan to update motor list.",44,12,0,ILI9341_YELLOW ,BACKGROUND);
            displayRestoreScreen(2);
          }else{
            displayPutStr("Scan and select motor.    ",44,12,0,ILI9341_YELLOW ,BACKGROUND);
          }
        }
        if(p.y > bot2Y1 && p.y < bot2Y2 && p.x > bot2X1 && p.x < bot2X2){
          displayMotorStatus();        // Display motor status
          if(currentMotor > 0)
            currentMotor--;           // Go to previous motor
          motorID = motorList[currentMotor];  // Update selected motor
          displayMotorStatus();       // Display motor status
        }
        if(p.y > bot3Y1 && p.y < bot3Y2 && p.x > bot3X1 && p.x < bot3X2){
          displayMotorStatus();        // Display motor status
          if(currentMotor < (motorIndex - 1))
            currentMotor++;           // Go to next motor
          motorID = motorList[currentMotor];  // Update selected motor
          displayMotorStatus();       // Display motor status
        }
        if(p.y > bot4Y1 && p.y < bot4Y2 && p.x > bot4X1 && p.x < bot4X2){
          CurrentScreen = 0;
          displayMainScreen();        // Display Main menu
          dxl.ledOff(BROADCAST_ID);   // Turn off all motors LED
        }
        break;
      case 4: /* Config Screen */
        if(p.y > bot1Y1 && p.y < bot1Y2 && p.x > bot1X1 && p.x < bot1X2){
          currentBaudrate++;          // Roll-out Baud Rate
          if(currentBaudrate > 7)
            currentBaudrate = 0;
          dxl.begin(configBaudrate[currentBaudrate]); // DynamixelInit different Baud rate
        }
        if(p.y > bot2Y1 && p.y < bot2Y2 && p.x > bot2X1 && p.x < bot2X2){
          protocol++;
          if(protocol > 2)
            protocol = 1;
          dxl.setPortProtocolVersion(protocol);  
        }
        if(p.y > bot3Y1 && p.y < bot3Y2 && p.x > bot3X1 && p.x < bot3X2){
          // DO TOUCH CALIBRATION
        }
        if(p.y > bot4Y1 && p.y < bot4Y2 && p.x > bot4X1 && p.x < bot4X2){
          CurrentScreen = 0;
          displayMainScreen();        // Display Main menu
          dxl.ledOff(BROADCAST_ID);   // Turn off all motors LED
        }
        break;
    }
  }
}

void prevReg(){
  if(currentRegister > 0){
    currentRegister--;        // Go to previous register
    // Check if register exist for this model
    dxl.ControlTableItemInfo_t item_info;
    item_info = dxl.getControlTableItemInfo(dxl.getModelNumber(motorID), currentRegister);
    if(item_info.addr_length == 0){
      prevReg();
    }
  }
  dislpayRegister();
}

void nextReg(){
  if(currentRegister < 90){
    currentRegister++;        // Go to next register
    // Check if register exist for this model
    dxl.ControlTableItemInfo_t item_info;
    item_info = dxl.getControlTableItemInfo(dxl.getModelNumber(motorID), currentRegister);
    if(item_info.addr_length == 0){
      nextReg();
    }
  }
  dislpayRegister();
}

void dislpayRegister(){
  displayPutStr(registerDescription[currentRegister],72,4,0,dataColor,dataBarColor);                  // Display register description
  motorRegister = dxl.readControlTableItem(currentRegister, motorID);               // Read motor register
  itoa(motorRegister,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,"     ");
  displayPutStr(motorRegisterBuffer,90,4,2,dataColor,dataBarColor);                                   // Display Register Value
  //ItemAndRangeInfo_t item_info;
  //item_info = dxl.getModelDependencyFuncInfo(dxl.getModelNumber(motorID), currentRegister);
  //item_info.min_value
  //item_info.max_value
  //displayPutStr(item_info.,136,4,0,dataColor,dataBarColor);                   // Display register range
}

void displayMainScreen(){
	tft.fillScreen(BACKGROUND);			// Clear screen and set background
	tft.writeRect(0,0,240,32,(uint16_t*)mainMenuTittle);				// Display Main menu title
	tft.writeRect(0,312,240,8,(uint16_t*)mainButtons);					// Display Main menu buttons

  tft.writeRect(10,64,100,100,(uint16_t*)mainButtons);
  tft.writeRect(130,64,100,100,(uint16_t*)registersButton);
  tft.writeRect(10,184,100,100,(uint16_t*)resetButton);
  tft.writeRect(130,184,100,100,(uint16_t*)configButton);
}

void displayScanScreen(){
  
  // ADD MODEL NUMBER
  
	tft.fillScreen(BACKGROUND);		// Clear screen
	tft.writeRect(0,0,240,32,(uint16_t*)scanSelect);				// Screen title Scan/Select Motors

	tft.fillRect(64,0,240,4,BACKGROUND);		// Clear Progress Bar

	tft.fillRect(68,0,240,64+6,dataBarColor);
  displayPutStr("Position",72,4,0,dataColor,dataBarColor);
	displayPutStr("---*",90,10,2,dataColor,dataBarColor);
	displayPutStr("MIN ---*",88,MINMAXVAL,0,dataColor,dataBarColor);
	displayPutStr("MAX ---*",108,MINMAXVAL,0,dataColor,dataBarColor);

	tft.fillRect(140,0,240,64+6,dataBarColor);
	displayPutStr("Voltage",144,4,0,dataColor,dataBarColor);
	displayPutStr("-.-v  ",162,10,2,dataColor,dataBarColor);
	displayPutStr("MIN -.-v",160,MINMAXVAL,0,dataColor,dataBarColor);
	displayPutStr("MAX -.-v",180,MINMAXVAL,0,dataColor,dataBarColor);

	tft.fillRect(212,0,240,64+6,dataBarColor);
	displayPutStr("Temperature",216,4,0,dataColor,dataBarColor);
	displayPutStr("--*C",234,10,2,dataColor,dataBarColor);
	displayPutStr("MIN --*C",232,MINMAXVAL,0,dataColor,dataBarColor);
	displayPutStr("MAX --*C",252,MINMAXVAL,0,dataColor,dataBarColor);

	tft.writeRect(0,288,240,32,(uint16_t*)mainMenu);			// scan/select Menu
}

void displayRegisterScreen(){

	tft.fillScreen(BACKGROUND);		// Clear screen
	tft.writeRect(0,0,240,32,(uint16_t*)configRegisters);			// Screen title Configure Register

	lockStatus = dxl.getTorqueEnableStat(motorID);
	if(lockStatus == 0){
		tft.writeRect(216,38,22,22,(uint16_t*)unlocked);					// unlocked position
	}else{
		tft.writeRect(216,38,22,22,(uint16_t*)locked);					// locked position
	}

	tft.fillRect(68,0,240,86,dataBarColor);	// Register data and description bar
	displayPutStr("Add: ---, Motor register     ",72,4,0,dataColor,dataBarColor);
	displayPutStr("---   ",90,4,2,dataColor,dataBarColor);
	displayPutStr("Range: --- to ---.           ",136,4,0,dataColor,dataBarColor);

	tft.writeRect(0,158,240,126,(uint16_t*)keyboard);					// Display keyboard

	tft.writeRect(0,288,240,32,(uint16_t*)regMenu);					// configure register menu
}

void displayRestoreScreen(unsigned char option){
	if(option == 1){
		tft.fillScreen(BACKGROUND);		// Clear screen
		tft.writeRect(0,0,240,32,(uint16_t*)factoryReset);				// Screen title Restore default

		tft.fillRect(68,0,240,212,dataBarColor);
		displayPutStr("Restore motor configurations",72,4,0,dataColor,dataBarColor);
		displayPutStr("to ",72+14,4,0,dataColor,dataBarColor);
		displayPutStr("Factory Default",72+14,4+24,0,ILI9341_ORANGE ,dataBarColor);
		displayPutStr("?",72+14,4+(8*18),0,dataColor,dataBarColor);

		displayPutStr("/// Warning /////////////////",116,4,0,ILI9341_WHITE ,ILI9341_RED );
		displayPutStr("This will set the ID to 1    ",136,4,0,dataColor,dataBarColor);
		displayPutStr("and Baud Rate to 1Mbps.      ",150,4,0,dataColor,dataBarColor);

		displayPutStr("Please avoid using identical ",176,4,0,dataColor,dataBarColor);
		displayPutStr("ID for multiple Motors.      ",192,4,0,dataColor,dataBarColor);
		displayPutStr("You may face communication   ",206,4,0,dataColor,dataBarColor);
		displayPutStr("failure or may not be able to",220,4,0,dataColor,dataBarColor);
		displayPutStr("detect Motors with same ID.  ",234,4,0,dataColor,dataBarColor);
		displayPutStr("/////////////////////////////",256,4,0,ILI9341_WHITE ,ILI9341_RED );

		tft.writeRect(0,288,240,32,(uint16_t*)resetMenu);
	}else if (option == 2){
		tft.fillRect(68,0,240,212,dataBarColor);
		displayPutStr("Restore motor configurations",72,4,0,dataColor,dataBarColor);
		displayPutStr("to ",72+14,4,0,dataColor,dataBarColor);
		displayPutStr("Factory Default",72+14,4+24,0,ILI9341_ORANGE,dataBarColor);
		displayPutStr("?",72+14,4+(8*18),0,dataColor,dataBarColor);
		tft.writeRect(58,120,124,124,(uint16_t*)resetOK);
		displayPutStr("          Reset OK!         ",252,4,0,dataColor,dataBarColor);
	}
}

void displayConfigScreen(){
	tft.fillScreen(BACKGROUND);
	tft.writeRect(0,0,240,32,(uint16_t*)configuration);

	tft.writeRect(0,32,240,60,(uint16_t*)smartMotorLogo);

	tft.fillRect(94,0,240,70,dataBarColor);
	tft.fillRect(94,0,2,70,ILI9341_BLUE );
	displayPutStr("Baud Rate (bps)",98,8,0,dataColor,dataBarColor);

	tft.fillRect(170,0,240,70,dataBarColor);
	tft.fillRect(170,0,2,70,ILI9341_YELLOW);
	displayPutStr("Protocol",174,8,0,dataColor,dataBarColor);

	tft.fillRect(248,0,240,32,dataBarColor);
	tft.fillRect(248,0,2,32,ILI9341_ORANGE);
	displayPutStr("TouchScreen Calibration",256,8,0,dataColor,dataBarColor);

	tft.writeRect(0,288,240,32,(uint16_t*)configMenu);
}

void displayMotorStatus(){
	if(motorIndex > 0){
		tft.fillRect(48,2,6,6,ILI9341_GREEN);				// Online Motor
		itoa((currentMotor+1),currentMotorStr,10);
		itoa(motorIndex,motorIndexStr,10);
		itoa(motorID,motorIDStr,10);

		strcpy(statusBuffer,"Motor ");strcat(statusBuffer,currentMotorStr);
		strcat(statusBuffer," of ");strcat(statusBuffer,motorIndexStr);
		strcat(statusBuffer,", ID: ");strcat(statusBuffer,motorIDStr);
		displayPutStr(statusBuffer,44,12,0,ILI9341_WHITE,BACKGROUND);
	}else{
		tft.fillRect(48,2,6,6,ILI9341_RED);				// Online Motor
		displayPutStr("No motors detected! ",44,12,0,ILI9341_RED,BACKGROUND);
	}
}

void displayScanMotors(){
	tft.fillRect(38,0,200,20,BACKGROUND);					// Clear status bar
	tft.fillRect(64,0,240,4,BACKGROUND);					// Clear search bar
	tft.fillRect(48,2,6,6,ILI9341_BLUE);							// Search Motor
	displayPutStr("Scanning motors...    ",44,12,0,ILI9341_WHITE,BACKGROUND);
	motorIndex = 0;
	currentMotor = 0;
	for(motorID = 0; motorID < 254; motorID++){
		motorList[motorID] = 0;
	}
	for(motorID = 0; motorID < 254; motorID++){
		if(dxl.ping(motorID)){
			motorList[motorIndex++] = motorID;
		}
		tft.fillRect(64,0,((motorID*240)/253),4,ILI9341_GREEN);		// Progress Bar
	}
	tft.fillRect(38,0,200,20,BACKGROUND);					// Clear status bar
	tft.fillRect(64,0,240,4,BACKGROUND);					// Clear search bar
	if(motorIndex > 0){
		motorID = motorList[currentMotor];
		tft.fillRect(48,2,6,6,ILI9341_GREEN);				// Online Motor
		itoa((currentMotor+1),currentMotorStr,10);
		itoa(motorIndex,motorIndexStr,10);
		itoa(motorID,motorIDStr,10);
		strcpy(statusBuffer,"Motor ");strcat(statusBuffer,currentMotorStr);
		strcat(statusBuffer," of ");strcat(statusBuffer,motorIndexStr);
		strcat(statusBuffer,", ID: ");strcat(statusBuffer,motorIDStr);
		displayPutStr(statusBuffer,44,12,0,ILI9341_WHITE,BACKGROUND);

    dxl.ledOff(BROADCAST_ID);   // Turn off all motors LED
		dxl.ledOn(motorID);         // Turn on the current motor LED
		dxl.torqueOff(motorID);			// Unlock motor
   
	}else{
		tft.fillRect(48,2,6,6,ILI9341_RED);				// Online Motor
		displayPutStr("No motors detected! ",44,12,0,ILI9341_RED,BACKGROUND);
	}

}

void motorPing(int motorIndex){
  if(dxl.ping(motorID)){            // Update motor status
    tft.fillRect(48,2,6,6,ILI9341_GREEN);       // Online Motor
  }else{
    tft.fillRect(48,2,6,6,ILI9341_RED);         // Off-line Motor
  }
}

void displayMotorInfo(int motorIndex){
  
  // ADD MODEL NUMBER
  /*checkModel(motorID);
  strcat(model,"*  ");
  displayPutStr(model,98-8,10,2,dataColor,dataBarColor);
  */
  motorRegister = dxl.getPresentPosition(motorID, UNIT_DEGREE);       // Read motor position
  itoa(motorRegister,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,"*  ");
  displayPutStr(motorRegisterBuffer,98-8,10,2,dataColor,dataBarColor);
  
  motorRegister = dxl.readControlTableItem(MIN_POSITION_LIMIT, motorID);      // Read motor CW Angle Limit
  itoa(motorRegister,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,"MIN ");strcat(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,"*   ");
  displayPutStr(motorRegisterBuffer,88,MINMAXVAL,0,dataColor,dataBarColor);
  
  motorRegister = dxl.readControlTableItem(MAX_POSITION_LIMIT, motorID);      // Read motor CCW Angle Limit
  itoa(motorRegister,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,"MAX ");strcat(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,"*   ");
  displayPutStr(motorRegisterBuffer,108,MINMAXVAL,0,dataColor,dataBarColor);
  
  motorRegister = dxl.readControlTableItem(PRESENT_VOLTAGE, motorID);      // Read motor voltage
  motorRegisterDecimals = motorRegister - ((motorRegister/10)*10);
  itoa(motorRegisterDecimals,motorRegisterDecimalsStr,10);
  itoa((motorRegister/10),motorRegisterStr,10);
  strcpy(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,".");
  strcat(motorRegisterBuffer,motorRegisterDecimalsStr);strcat(motorRegisterBuffer,"v ");
  displayPutStr(motorRegisterBuffer,170-8,10,2,dataColor,dataBarColor);
  
  motorRegister = dxl.readControlTableItem(MIN_VOLTAGE_LIMIT, motorID);      // Read motor minimum voltage limit
  motorRegisterDecimals = motorRegister - ((motorRegister/10)*10);
  itoa(motorRegisterDecimals,motorRegisterDecimalsStr,10);
  itoa(motorRegister/10,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,"MIN ");strcat(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,".");
  strcat(motorRegisterBuffer,motorRegisterDecimalsStr);strcat(motorRegisterBuffer,"v ");
  displayPutStr(motorRegisterBuffer,160,MINMAXVAL,0,dataColor,dataBarColor);
  
  motorRegister = dxl.readControlTableItem(MAX_VOLTAGE_LIMIT, motorID);      // Read motor maximum voltage limit
  motorRegisterDecimals = motorRegister - ((motorRegister/10)*10);
  itoa(motorRegisterDecimals,motorRegisterDecimalsStr,10);
  itoa(motorRegister/10,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,"MAX ");strcat(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,".");
  strcat(motorRegisterBuffer,motorRegisterDecimalsStr);strcat(motorRegisterBuffer,"v ");
  displayPutStr(motorRegisterBuffer,180,MINMAXVAL,0,dataColor,dataBarColor);
  
  motorRegister = dxl.readControlTableItem(PRESENT_TEMPERATURE, motorID);      // Read motor temperature
  itoa(motorRegister,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,"*C  ");
  displayPutStr(motorRegisterBuffer,242-8,10,2,dataColor,dataBarColor); //242
  
  motorRegister = dxl.readControlTableItem(TEMPERATURE_LIMIT, motorID);      // Read motor MAX Temperature Limit
  itoa(motorRegister,motorRegisterStr,10);
  strcpy(motorRegisterBuffer,"MAX ");strcat(motorRegisterBuffer,motorRegisterStr);strcat(motorRegisterBuffer,"*C  ");
  displayPutStr("MIN --*C ",232,MINMAXVAL,0,dataColor,dataBarColor);
  displayPutStr(motorRegisterBuffer,252,MINMAXVAL,0,dataColor,dataBarColor);
}

void displayPutStr(const char *pString, uint16_t x, uint16_t y, uint8_t Size, uint16_t fColor, uint16_t bColor) {
  tft.setCursor(x , y);
  tft.setTextColor(fColor,bColor);
  tft.setTextSize(Size);
  tft.println(pString);
}

int keyboardTouch(){
  if(p.y > 255 && p.y < 280 && p.x > 80 && p.x < 160){
    return 0;
  }
  if(p.y > 160 && p.y < 190 && p.x > 0 && p.x < 80){
    return 1;
  }
  if(p.y > 160 && p.y < 190 && p.x > 80 && p.x < 160){
    return 2;
  }
  if(p.y > 160 && p.y < 190 && p.x > 160 && p.x < 240){
    return 3;
  }
  if(p.y > 190 && p.y < 220 && p.x > 0 && p.x < 80){
    return 4;
  }
  if(p.y > 190 && p.y < 220 && p.x > 80 && p.x < 160){
    return 5;
  }
  if(p.y > 190 && p.y < 220 && p.x > 160 && p.x < 240){
    return 6;
  }
  if(p.y > 220 && p.y < 255 && p.x > 0 && p.x < 80){
    return 7;
  }
  if(p.y > 220 && p.y < 255 && p.x > 80 && p.x < 160){
    return 8;
  }
  if(p.y > 220 && p.y < 255 && p.x > 160 && p.x < 240){
    return 9;
  }
  if(p.y > 255 && p.y < 280 && p.x > 160 && p.x < 240){
    return -1; // backspace
  }
  return -2; //nothing
}

void checkModel(int motorID){
  switch(dxl.getModelNumber(motorID)){
    case 12: strcpy(model,"AX12A"); break;
    case 300: strcpy(model,"AX12W"); break;
    case 18: strcpy(model,"AX18A"); break;
    case 10: strcpy(model,"RX10"); break;
    case 24: strcpy(model,"RX24F"); break;
    case 28: strcpy(model,"RX28"); break;
    case 64: strcpy(model,"RX64"); break;
    case 113: strcpy(model,"DX113"); break;
    case 116: strcpy(model,"DX116"); break;
    case 117: strcpy(model,"DX117"); break;
    case 107: strcpy(model,"EX106"); break;
    case 360: strcpy(model,"MX12W"); break;
    case 29: strcpy(model,"MX28"); break;
    case 310: strcpy(model,"MX64"); break;
    case 320: strcpy(model,"MX106"); break;
    case 30: strcpy(model,"MX28_2"); break;
    case 311: strcpy(model,"MX64_2"); break;
    case 321: strcpy(model,"MX106_2"); break;
    case 350: strcpy(model,"XL320"); break;
    case 1190: strcpy(model,"XL330_M077"); break;
    case 1200: strcpy(model,"XL330_M288"); break;
    case 1070: strcpy(model,"XC430_W150"); break;
    case 1080: strcpy(model,"XC430_W240"); break;
    case 1160: strcpy(model,"XXC430_W250"); break;
    case 1060: strcpy(model,"XL430_W250"); break;
    case 1090: strcpy(model,"XXL430_W250"); break;
    case 1030: strcpy(model,"XM430_W210"); break;
    case 1020: strcpy(model,"XM430_W350"); break;
    case 1130: strcpy(model,"XM540_W150"); break;
    case 1120: strcpy(model,"XM540_W270"); break;
    case 1050: strcpy(model,"XH430_V210"); break;
    case 1040: strcpy(model,"XH430_V350"); break;
    case 1010: strcpy(model,"XH430_W210"); break;
    case 1000: strcpy(model,"XH430_W350"); break;
    case 1110: strcpy(model,"XH540_W150"); break;
    case 1100: strcpy(model,"XH540_W270"); break;
    case 1150: strcpy(model,"XH540_V150"); break;
    case 1140: strcpy(model,"XH540_V270"); break;
    case 1180: strcpy(model,"XW540_T140"); break;
    case 1170: strcpy(model,"XW540_T260"); break;
    case 35072: strcpy(model,"PRO_L42_10_S300_R"); break;
    case 37928: strcpy(model,"PRO_L54_30_S400_R"); break;
    case 37896: strcpy(model,"PRO_L54_30_S500_R"); break;
    case 38176: strcpy(model,"PRO_L54_50_S290_R"); break;
    case 38152: strcpy(model,"PRO_L54_50_S500_R"); break;
    case 43288: strcpy(model,"PRO_M42_10_S260_R"); break;
    case 46096: strcpy(model,"PRO_M54_40_S250_R"); break;
    case 46352: strcpy(model,"PRO_M54_60_S250_R"); break;
    case 51200: strcpy(model,"PRO_H42_20_S300_R"); break;
    case 53768: strcpy(model,"PRO_H54_100_S500_R"); break;
    case 54024: strcpy(model,"PRO_H54_200_S500_R"); break;
    case 43289: strcpy(model,"PRO_M42_10_S260_RA"); break;
    case 46097: strcpy(model,"PRO_M54_40_S250_RA"); break;
    case 46353: strcpy(model,"PRO_M54_60_S250_RA"); break;
    case 51201: strcpy(model,"PRO_H42_20_S300_RA"); break;
    case 53761: strcpy(model,"PRO_H54_100_S500_RA"); break;
    case 54025: strcpy(model,"PRO_H54_200_S500_RA"); break;
    case 2000: strcpy(model,"PRO_H42P_020_S300_R"); break;
    case 2010: strcpy(model,"PRO_H54P_100_S500_R"); break;
    case 2020: strcpy(model,"PRO_H54P_200_S500_R"); break;
    case 2100: strcpy(model,"PRO_M42P_010_S260_R"); break;
    case 2110: strcpy(model,"PRO_M54P_040_S250_R"); break;
    case 2120: strcpy(model,"PRO_M54P_060_S250_R"); break;
    default: strcpy(model,"UNKNOWN");break;
  }
}
