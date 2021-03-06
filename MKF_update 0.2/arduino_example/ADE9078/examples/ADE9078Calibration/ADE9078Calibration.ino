 /* ADE9078 Calibration Firmware  -  Calibrates ADE9078 and loads calibration values into the EEPROM where they can be loaded by other user code

ADE9078 board and library created by: David Brady, Jeff Baez, and Jacky Wan 11/2018

Code major development by David Brady

PCB design done by Esteban Granizo and Peter Nguyen (12/2018)

University of California, Irvine - California Plug Load Research Center (CalPlug)

December 2018 - Library First Release (December 2018) - Library Latest Release for ADE9078

Copyright (C) The Regents of the University of California, 2019

  Note: Please refer to the Analog Devices ADE9078 datasheet - much of this library was based directly on the statements and methods provided in it as well as the ADE9000 and the template firmwares provided by this chipset!  Their authors get paid, trust them over us!

  Released into the public domain.
 
Note: this firmware is based directly on the sample provided ADE9000 Calibration firmware.  Please carefully review this example!  The authors of this ADE9078 version greatly appreciate the authors of this example for allowing us a great structure to use for this example:  

/*The calibration inputs are stored in the ADE9078CalibrationInputs.h file. The phase and parameter being calibrated is input through the serial console*/
/*Calibration constants are computed and stored in the EEPROM */
/*Calibration should be done with the end application settings. If any  parameters(GAIN,High pass corner,Integrator settings) are changed, the device should be re-calibrated*/
/*This application assumues the PGA_GAIN among all current channels is same. Also, the PGA_GAIN among all voltage channels should be same*/  
//Each PGA gain may have different calibration values associated when changed as a set for all inputs.

#include "Arduino.h" //this includes the arduino library header. It makes all the Arduino functions available in this tab.
//#include <EEPROM.h>  //Load and save EEPROM calibration values to Arduino Onboard EEPROM (should be declared elsewhere in user code!)
#include <SPI.h>
//#include "ADE9078registers.h"
#include <ADE9078CalibrationInputs.h>
#include <math.h>
#include <ADE9078.h>

//Architecture Control:
//Make sure you select in the ADE9078.h file the proper board architecture, either Arduino/AVR/ESP8266 or ESP32
//REMINDER: ONLY SELECT THE SINGLE OPTION FOR THE BOARD TYPE YOU ARE USING!

 
//****EEPROM Settings******
#define NUM_ELEMENTS 14 //Define total fields for EEPROM storage that are cleared and used
// temporary char buffers for individual EEPROM saved values, local copy for the RAM to use after EEPROM load complete
char data[250] = {}; //holder for the string pushed into the EEPROM; buffer for EEPROM string
char data2[250] = {}; //holder for the string pushed into the EEPROM; used for test read from the EEPROM

//Fields for calibration values from ADE9000 example, declare blank to begin with
char Valconfigured[5]; //Identification if previous values are OK (can be used to detect good values on boot-up, if not, load default values
char ValADDR_AIGAIN[20];
char ValADDR_BIGAIN[20];
char ValADDR_CIGAIN[20];
char ValADDR_NIGAIN[20];
char ValADDR_AVGAIN[20];
char ValADDR_BVGAIN[20];
char ValADDR_CVGAIN[20];
char ValADDR_APHCAL0[20];
char ValADDR_BPHCAL0[20];
char ValADDR_CPHCAL0[20];
char ValADDR_APGAIN[20];
char ValADDR_BPGAIN[20];
char ValADDR_CPGAIN[20];
//**********

//The following functions are used to assist with loading calibration and mode values onto the on-board EEPROM:

void save_data(char* data)  // Saves data in the passed array to the EEPROM
{
  Serial.println("Call to write data to EEPROM");
  #ifdef ESP32ARCH //Architecture defined in the ADE9078.h file!
  EEPROM.begin(512);
  #endif
  for (int i = 0; i < strlen(data); ++i)
  {
    EEPROM.write(i, (int)data[i]);
    delay(1);
  }
  #ifdef ESP32ARCH
  EEPROM.commit();
  #endif
  Serial.println("Write data complete");
  delay(100);
}

void load_data(char* data) //passes all read fields back by reference
{
  Serial.println("Read data from EEPROM");
  #ifdef ESP32ARCH
  EEPROM.begin(512);
  #endif
  int count = 0;
  int address = 0;
  while (count < NUM_ELEMENTS)
  {
    char read_char = (char)EEPROM.read(address);
    delay(1);
    strncat(data, &read_char, 1);
    if (read_char == '#')
      ++count;
    ++address;
	delay(100);
  }
  Serial.println("Read data complete with the following values read back: ");
  Serial.println(data);
}


void wipe_data()
{
  Serial.println("Wipe EEPROM Values called");
  const char* sep = "#";
  #ifdef ESP32ARCH
  EEPROM.begin(512);
  #endif
  for (int i = 0; i < NUM_ELEMENTS; ++i)
  {
    EEPROM.write(i, (int)sep);
    delay(1);
    EEPROM.write(i, 50); //end character
    delay(1);
  }
  #ifdef ESP32ARCH
  EEPROM.commit();
  #endif
  Serial.println("Wipe data complete");
}

void EEPROMInit() { //function called once on a virgin micro-controller to set the EEPROM with used fields
  Serial.println(" Begin EEPROM Initialization Function...");
  //Default values
  strcpy(Valconfigured, "0"); //indication that the EEPROM is configured with values, use 0, the un-configured, default state
  //Set field values with default calibration values
  strcpy(ValADDR_AIGAIN, "1");
  strcpy(ValADDR_BIGAIN, "1");
  strcpy(ValADDR_CIGAIN, "1");
  strcpy(ValADDR_NIGAIN, "1");
  strcpy(ValADDR_AVGAIN, "1");
  strcpy(ValADDR_BVGAIN, "1");
  strcpy(ValADDR_CVGAIN, "1");
  strcpy(ValADDR_APHCAL0, "1");
  strcpy(ValADDR_BPHCAL0, "1");
  strcpy(ValADDR_CPHCAL0, "1");
  strcpy(ValADDR_APGAIN, "1");
  strcpy(ValADDR_BPGAIN, "1");
  strcpy(ValADDR_CPGAIN, "1");
  wipe_data(); //wipe current EEPROM values, without this the configured value is almost certainly not pre-set properly.
  const char* sep = "#";
  strcat(data, Valconfigured);
  strcat(data, sep);
  strcat(data, ValADDR_AIGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_BIGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_CIGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_NIGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_AVGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_BVGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_CVGAIN);
  strcat(data, sep); 
  strcat(data, ValADDR_APHCAL0);
  strcat(data, sep);
  strcat(data, ValADDR_BPHCAL0);
  strcat(data, sep);
  strcat(data, ValADDR_CPHCAL0);
  strcat(data, sep);
  strcat(data, ValADDR_APGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_BPGAIN);
  strcat(data, sep);
  strcat(data, ValADDR_CPGAIN);
  strcat(data, sep);

  Serial.println("This is the final string to load into EEPROM:");
  Serial.println(data);
  Serial.println("The length is: ");
  Serial.println((unsigned)strlen(data));
  Serial.println();
  save_data(data);
  Serial.println("Data saved to EEPROM");
  Serial.println("Test read of saved data from EEPROM:");
  load_data(data2);
  Serial.println(data2);

  Serial.println("EEPROM Initialization Complete");
}

void load_data_allfields() //load the data field by field into the RAM holders - MUST MATCH EEPROM formatting!
{
  Serial.println("Call to read data from EEPROM");
  #ifdef ESP32ARCH
  EEPROM.begin(512);
  #endif
  int count = 0;
  int address = 0;
  while (count < NUM_ELEMENTS)
  {
    char read_char = (char)EEPROM.read(address);
    delay(1);
    if (read_char == '#')
    {
      Serial.println(data);
      switch (count)
      {
	    //load each value field by field and parse into the RAM holders for each value, the readout must match the loaded data fields!

	    case 0: strcpy(Valconfigured, data); break;
        case 1: strcpy(ValADDR_AIGAIN, data); break;
        case 2: strcpy(ValADDR_BIGAIN, data); break;
        case 3: strcpy(ValADDR_CIGAIN, data); break;
        case 4: strcpy(ValADDR_NIGAIN, data); break;
        case 5: strcpy(ValADDR_AVGAIN, data); break;
        case 6: strcpy(ValADDR_BVGAIN, data); break;
        case 7: strcpy(ValADDR_CVGAIN, data); break;
        case 8: strcpy(ValADDR_APHCAL0, data); break;
        case 9: strcpy(ValADDR_BPHCAL0, data); break;
        case 10: strcpy(ValADDR_CPHCAL0, data); break;
        case 11: strcpy(ValADDR_APGAIN, data); break;
        case 12: strcpy(ValADDR_BPGAIN, data); break;
        case 13: strcpy(ValADDR_CPGAIN, data); break;
      }
      count++;
      strcpy(data,"");
    }
    else
    {
      strncat(data, &read_char, 1);  //total of 1 character appended
    }
    ++address;
  }
  //when using the recalled values (outside of this function, the following approaches can be used to make the arrays that hold values as characters into numerical values for use:
  //atoi() can be used to generate integers needed from char arrays for loading, e.g. integerFromPC = atoi(strtokIndx);     // convert this part to an integer strtokIndx = strtok(NULL, ",");floatFromPC = atof(strtokIndx);     // convert this part to a float (see: http://forum.arduino.cc/index.php?topic=396450)
    //floatFromPC = atof(strtokIndx);     // convert this part to a float
	//typically the calibration values should be saved as reciprovcals so they are large integers rather than large floats to avoid round-off errors!!
  Serial.println("<--Read data complete, this was read");
}//*****************************************************



/*Function declerations*/
void ADE9078_calibrate();
void ADE9078_iGain_calibrate(int32_t *,int32_t *,int32_t *,int32_t *, int);     //Current gain calibration function
void ADE9078_vGain_calibrate(int32_t *,int32_t *,int32_t *,int32_t *, int);     //Voltage gain calibration function
void ADE9078_pGain_calibrate(int32_t *,int32_t *,int32_t *,int32_t *, float);   //Power gain calibration function
void ADE9078_phase_calibrate(int32_t *,int32_t *,int32_t *,int32_t *, int);     //Phase calibration function
void calibrationEnergyRegisterSetup();                                          //Setup of Energy registers used in calibration. Donot Edit
void getPGA_gain();
int8_t isRegisterPositive(int32_t);
void storeCalConstToEEPROM();


#define IGAIN_CAL_REG_SIZE 4  
int32_t xIgain_registers[IGAIN_CAL_REG_SIZE];   //order [AIGAIN, BIGAIN, CIGAIN, NIGAIN]
int32_t xIgain_register_address[IGAIN_CAL_REG_SIZE]=
         {AIGAIN_32, BIGAIN_32, CIGAIN_32, NIGAIN_32};   //order [AIGAIN, BIGAIN, CIGAIN, NIGAIN]
int32_t xIrms_registers[IGAIN_CAL_REG_SIZE];
int32_t xIrms_registers_address[IGAIN_CAL_REG_SIZE]= {AIRMS_32, BIRMS_32, CIRMS_32, NIRMS_32};

#define VGAIN_CAL_REG_SIZE 3
int32_t xVgain_registers[VGAIN_CAL_REG_SIZE];   //order [AVGAIN, BVGAIN, CVGAIN]
int32_t xVgain_register_address[VGAIN_CAL_REG_SIZE]={AVGAIN_32, BVGAIN_32, CVGAIN_32};   //order [AVGAIN, BVGAIN, CVGAIN]
int32_t xVrms_registers[VGAIN_CAL_REG_SIZE];
int32_t xVrms_registers_address[VGAIN_CAL_REG_SIZE]= {AVRMS_32, BVRMS_32, CVRMS_32};

#define PHCAL_CAL_REG_SIZE 3
int32_t xPhcal_registers[PHCAL_CAL_REG_SIZE];   //order [APHCAL, BPHCAL, CPHCAL]
int32_t xPhcal_register_address[PHCAL_CAL_REG_SIZE]={APHCAL0_32, BPHCAL0_32, CPHCAL0_32};   //order [APHCAL, BPHCAL, CPHCAL]
int32_t xWATTHRHI_registers[PHCAL_CAL_REG_SIZE];  //Active energy registers
int32_t xWATTHRHI_registers_address[PHCAL_CAL_REG_SIZE]= {AWATTHR_HI_32, BWATTHR_HI_32, CWATTHR_HI_32};
int32_t xVARHRHI_registers[PHCAL_CAL_REG_SIZE];
int32_t xVARHRHI_registers_address[PHCAL_CAL_REG_SIZE]= {AVARHR_HI_32, BVARHR_HI_32, CVARHR_HI_32};

#define PGAIN_CAL_REG_SIZE 3
int32_t xPgain_registers[PGAIN_CAL_REG_SIZE];   //order [APGAIN, BPGAIN, CPGAIN]
int32_t xPgain_register_address[PGAIN_CAL_REG_SIZE]={APGAIN_32, BPGAIN_32, CPGAIN_32};   //order [AVGAIN, BVGAIN, CVGAIN, NVGAIN]
//The Power gain calibration reads active energy registers. The content and address arrays are defined in the PHCAL section above

//Global variables
#define EGY_REG_SIZE 3
int8_t calCurrentPGA_gain=0;
int8_t calVoltagePGA_gain=0;
int32_t accumulatedActiveEnergy_registers[EGY_REG_SIZE];
int32_t accumulatedReactiveEnergy_registers[EGY_REG_SIZE];
uint32_t calibrationDataToEEPROM[CALIBRATION_CONSTANTS_ARRAY_SIZE];

//ADE9078 ade9078;
#define local_SPI_freq 1000000
#define local_SS 10
#define IRQ0_INTERRUPT_PIN 2
#define INT_MODE FALLING
#define ACCUMULATION_TIME 5                 //accumulation time in seconds when EGY_TIME=7999, accumulation mode= sample based
#define EGY_INTERRUPT_MASK0 0x00000001      //Enable EGYRDY interrupt


enum CAL_STATE
{
  CAL_START,
  CAL_VI_CALIBRATE,
  CAL_PHASE_CALIBRATE,
  CAL_PGAIN_CALIBRATE,
  CAL_STORE,
  CAL_STOP,
  CAL_RESTART,
  CAL_COMPLETE
};


CAL_STATE CUR_STATE = CAL_START;   //current state is start

struct InitializationSettings* is = new InitializationSettings; //define structure for initialized values

ADE9078 myADE9078(local_SS, local_SPI_freq, is); // Call the ADE9078 Object with hardware parameters specified, local variables are copied to private variables inside the class when object is created.

void setup() {

  Serial.begin(115200);
  delay(200);
  myADE9078.initialize();       /*Setup ADE9078. Refer to ADE9078.cpp file for detail. 
                                GAIN should not be changed after calibration. Recalibrate if any configuration affecting the digital datapath changes.*/
  calibrationEnergyRegisterSetup();
  getPGA_gain();  
  //Wire.begin(); 
  //myADE9078.writeByteToEeprom(ADDR_EEPROM_WRITTEN_BYTE,~(EEPROM_WRITTEN)); //clear calibration done status 
}

void loop()
{
  delay(1000);
  ADE9078_calibrate();
}

void ADE9078_calibrate()
{
  float calPf ;  
  int16_t temp;
  char serialReadData;
  static int8_t calChannel = 0; //the channel being calibrated
  static int8_t channelCalLength = 1; //the length

  switch(CUR_STATE)
  {
      case CAL_START:       //Start
      Serial.println("Starting calibration process. Select one of the following {Start (Y/y) OR Abort(Q/q) OR Restart (R/r)}:");
      while (!Serial.available());  //wait for serial data to be available
      serialReadData = Serial.read();
      if(serialReadData == 'Y' || serialReadData == 'y')
        { 
           Serial.println("Enter the Phase being calibrated:{Phase A(A/a) OR Phase B(B/b) OR Phase C(C/c) OR Neutral(N/n) OR All Phases(D/d)}:");
           while(Serial.read()>=0);     //Flush any extra characters
           while (!Serial.available());  //wait for serial data to be available
           serialReadData = Serial.read(); 
           if(serialReadData == 'A' || serialReadData == 'a') 
              {
                Serial.println("Calibrating Phase A:");
                calChannel=0; 
                channelCalLength=1;
              }
           else if(serialReadData == 'B' || serialReadData == 'b') 
              {
                Serial.println("Calibrating Phase B:");
                calChannel=1;
                channelCalLength=1;
              } 
           else if(serialReadData == 'C' || serialReadData == 'c') 
              {
                Serial.println("Calibrating Phase C:");
                calChannel=2;
                channelCalLength=1;
              }
           else if(serialReadData == 'N' || serialReadData == 'n') 
              {
               // calChannel=0;
              //  channelCalLength=1;                    
              }
           else if(serialReadData == 'D' || serialReadData == 'd') 
              {
                Serial.println("Calibrating all phases:");
                calChannel=0; //Start from channel A
                channelCalLength=3;                    
              }
           else 
              {
                Serial.println("Wrong input");
                while(Serial.read()>=0)
                 {
                   temp = Serial.read();  //Read till the buffer is empty
                 }
                serialReadData=' ';
                break;                                             
              }
           
           CUR_STATE=CAL_VI_CALIBRATE;
           Serial.print("Starting calibration with ");
           Serial.print(NOMINAL_INPUT_VOLTAGE);
           Serial.print(" Vrms and ");
           Serial.print(NOMINAL_INPUT_CURRENT);
           Serial.println(" Arms");
           while(Serial.read()>=0)
             {
                temp = Serial.read();  //Read till the buffer is empty
             }
           serialReadData=' ';
           break;
        }
      else
        {
           if(serialReadData == 'Q' || serialReadData == 'q')
              {
                CUR_STATE=CAL_STOP;
                Serial.println("Aborting calibration");
                while(Serial.read()>=0)
                  {
                    temp = Serial.read();  //Read till the buffer is empty
                  }                
               serialReadData=' ';
               break;
              }
            else if(serialReadData == 'R' || serialReadData == 'r')
              {
                 CUR_STATE=CAL_RESTART;
                 while(Serial.read()>=0)
                  {
                    temp = Serial.read();  //Read till the buffer is empty
                  }
                  serialReadData=' ';
                  break;
              }
           else 
             {
                Serial.println("Wrong input");
                while(Serial.read()>=0)
                 {
                    temp = Serial.read();  //Read till the buffer is empty
                 }
                serialReadData=' ';
                break;
             }
        }

      break;
    
      case CAL_VI_CALIBRATE:   //Calibrate
      ADE9078_iGain_calibrate(&xIgain_registers[calChannel],&xIgain_register_address[calChannel], &xIrms_registers[calChannel], &xIrms_registers_address[calChannel], channelCalLength);       //Calculate xIGAIN
      ADE9078_vGain_calibrate(&xVgain_registers[calChannel], &xVgain_register_address[calChannel], &xVrms_registers[calChannel], &xVrms_registers_address[calChannel], channelCalLength);       //Calculate xVGAIN
      Serial.println("Current gain calibration completed");
      Serial.println("Voltage gain calibration completed");
      CUR_STATE=CAL_PHASE_CALIBRATE;
      break;
      
      case CAL_PHASE_CALIBRATE:
      Serial.println("Perform Phase calibration: Yes(Y/y) OR No (N/n): ");
      while(Serial.read()>=0);     //Flush any extra characters
      while (!Serial.available());  //wait for serial data to be available
      serialReadData = Serial.read();
      if(serialReadData == 'Y' || serialReadData == 'y')
        {
            Serial.println("Ensure Power factor is 0.5 lagging such that Active and Reactive energies are positive: Continue: Yes(Y/y) OR Restart (R/r): ");
            while(Serial.read()>=0);     //Flush any extra characters
            while(!Serial.available());  //Wait for new input
            serialReadData =Serial.read();
            if(serialReadData == 'Y' || serialReadData == 'y')
              {
               while(Serial.read()>=0); //Flush any extra characters            
                ADE9078_phase_calibrate(&xPhcal_registers[calChannel],&xPhcal_register_address[calChannel], &accumulatedActiveEnergy_registers[calChannel], &accumulatedReactiveEnergy_registers[calChannel], channelCalLength);     //Calculate xPHCAL
               Serial.println("Phase calibration completed");
              }
            else 
              {
                if(serialReadData == 'R' || serialReadData == 'r')
                  {
                    CUR_STATE=CAL_RESTART;
                    while(Serial.read()>=0);     //Flush any extra characters               
                    break;
                  }
                  else 
                  {
                    while(Serial.read()>=0);     //Flush any extra characters                               
                    Serial.println("Wrong Input");
                    break;
                  }
              }           
          }
      else
        {
          if(serialReadData == 'N' || serialReadData == 'n')
            {
                while(Serial.read()>=0);     //Flush any extra characters                                               
                Serial.println("Skipping phase calibration ");
            }
          else 
            {                   

              while(Serial.read()>=0);     //Flush any extra characters                                               
              Serial.println("Wrong input");
              break;
             }
        }
      CUR_STATE = CAL_PGAIN_CALIBRATE;      
      break;       
      
      case CAL_PGAIN_CALIBRATE:   
      Serial.println("Starting Power Gain calibration");  
      Serial.println("Enter the Power Factor of inputs for xPGAIN calculation: 1(1) OR CalibratingAnglePF(0): ");
      while(Serial.read()>=0);
      while (!Serial.available());  //wait for serial data to be available
      serialReadData = Serial.read();
      if(serialReadData == '1')
        {
           while(Serial.read()>=0);
           calPf=1; 
        }
      else
        {
          if(serialReadData == '0')
            {
              while(Serial.read()>=0);
              calPf=CAL_ANGLE_RADIANS(CALIBRATION_ANGLE_DEGREES);
            }
          else
          {
             while(Serial.read()>=0);        
             Serial.println("Wrong input");
             break;
          }
  
        }      
      ADE9078_pGain_calibrate(&xPgain_registers[calChannel],&xPgain_register_address[calChannel],&accumulatedActiveEnergy_registers[calChannel],channelCalLength, calPf);
      Serial.println("Power gain calibration completed ");
      Serial.println("Calibration completed. Storing calibration constants to EEPROM ");
      CUR_STATE = CAL_STORE;      
      break; 
    
      case CAL_STORE:     //Store Constants to EEPROM
      //storeCalConstToEEPROM();
      Serial.println("Calibration constants successfully stored in EEPROM. Exit Application");
      CUR_STATE = CAL_COMPLETE; 
      break;
    
      case CAL_STOP:    //Stop calibration
      Serial.println("Calibration stopped. Restart Arduino to recalibrate");
      CUR_STATE = CAL_COMPLETE; 
      break;
    
      case CAL_RESTART:    //restart
      Serial.println("Restarting calibration");
      CUR_STATE = CAL_START; 
      break;

      case CAL_COMPLETE:
      break;
      
      default:
      break;
      
  }
  
}


void ADE9078_iGain_calibrate(int32_t *igainReg, int32_t *igainRegAddress, int32_t *iRmsReg, int32_t *iRmsRegAddress, int arraySize)
{
  float temp;
  int32_t actualCodes;
  int32_t expectedCodes;
  int32_t registerReading;
  int32_t aigain;
  int i;

  temp=ADE9078_RMS_FULL_SCALE_CODES*CURRENT_TRANSFER_FUNCTION*calCurrentPGA_gain*NOMINAL_INPUT_CURRENT *sqrt(2); 
  expectedCodes= (int32_t) temp;  //Round off 
  Serial.print("Expected IRMS Code: "); 
  Serial.println(expectedCodes,HEX);
  for (i=0; i < arraySize ;i++)
    {
      actualCodes = myADE9078.spiRead32(iRmsRegAddress[i]);
      temp= (((float)expectedCodes/(float)actualCodes)-1)* 134217728;  //calculate the gain.
      igainReg[i] = (int32_t) temp; //Round off
      Serial.print("Channel ");
      Serial.print(i+1);
      Serial.print(" actual IRMS Code: "); 
      Serial.println(actualCodes,HEX);      
      Serial.print("Current Gain Register: "); 
      Serial.println(igainReg[i],HEX);
    }
}

void ADE9078_vGain_calibrate(int32_t *vgainReg, int32_t *vgainRegAddress, int32_t *vRmsReg, int32_t *vRmsRegAddress, int arraySize)
{
  float temp;
  int32_t actualCodes;
  int32_t expectedCodes;
  int32_t registerReading;
  int i;
  
  temp=ADE9078_RMS_FULL_SCALE_CODES*VOLTAGE_TRANSFER_FUNCTION*calVoltagePGA_gain*NOMINAL_INPUT_VOLTAGE*sqrt(2); 
  expectedCodes= (int32_t) temp;  //Round off
  Serial.print("Expected VRMS Code: "); 
  Serial.println(expectedCodes,HEX);  
  for (i=0; i < arraySize ;i++)
    {
      actualCodes = myADE9078.spiRead32(vRmsRegAddress[i]);
      temp= (((float)expectedCodes/(float)actualCodes)-1)* 134217728;  //calculate the gain.
      vgainReg[i] = (int32_t) temp; //Round off  
      Serial.print("Channel ");
      Serial.print(i+1);
      Serial.print(" actual VRMS Code: "); 
      Serial.println(actualCodes,HEX);       
      Serial.print("Voltage Gain Register: "); 
      Serial.println(vgainReg[i],HEX);
    }
}

void ADE9078_phase_calibrate(int32_t *phcalReg,int32_t *phcalRegAddress,int32_t *accActiveEgyReg,int32_t *accReactiveEgyReg, int arraySize)
{
  Serial.println("Computing phase calibration registers..."); 
  delay((ACCUMULATION_TIME+1)*1000); //delay to ensure the energy registers are accumulated for defined interval
  float errorAngle;
  float errorAngleDeg;
  float omega;
  double temp;
  double temp1;
  double temp2;
  int32_t actualActiveEnergyCode;
  int32_t actualReactiveEnergyCode; 
  int32_t phcalREG;
  int i;
  omega = (float)2 *(float)3.14159*(float) INPUT_FREQUENCY /(float)ADE90xx_FDSP;
  

  for (i=0; i < arraySize ;i++)
    {
        actualActiveEnergyCode = accActiveEgyReg[i];
        actualReactiveEnergyCode = accReactiveEgyReg[i];
        errorAngle = (double)-1 * atan( ((double)actualActiveEnergyCode*(double)sin(CAL_ANGLE_RADIANS(CALIBRATION_ANGLE_DEGREES))-(double)actualReactiveEnergyCode*(double)cos(CAL_ANGLE_RADIANS(CALIBRATION_ANGLE_DEGREES)))/((double)actualActiveEnergyCode*(double)cos(CAL_ANGLE_RADIANS(CALIBRATION_ANGLE_DEGREES))+(double)actualReactiveEnergyCode*(double)sin(CAL_ANGLE_RADIANS(CALIBRATION_ANGLE_DEGREES))));
        temp = (((double)sin((double)errorAngle-(double)omega)+(double)sin((double)omega))/((double)sin(2*(double)omega-(double)errorAngle)))*134217728;
        phcalReg[i]= (int32_t)temp;
        errorAngleDeg = (float)errorAngle*180/3.14159;
        Serial.print("Channel ");
        Serial.print(i+1);        
        Serial.print(" actual Active Energy Register: "); 
        Serial.println(actualActiveEnergyCode,HEX);
        Serial.print("Channel ");
        Serial.print(i+1);                 
        Serial.print(" actual Reactive Energy Register: "); 
        Serial.println(actualReactiveEnergyCode,HEX);              
        Serial.print("Phase Correction (degrees): "); 
        Serial.println(errorAngleDeg,5);       
        Serial.print("Phase Register: "); 
        Serial.println(phcalReg[i],HEX);        
    }



}

void ADE9078_pGain_calibrate(int32_t *pgainReg, int32_t *pgainRegAddress, int32_t *accActiveEgyReg, int arraySize, float pGaincalPF)
{
  Serial.println("Computing power gain calibration registers...");
  delay((ACCUMULATION_TIME+1)*1000); //delay to ensure the energy registers are accumulated for defined interval
  int32_t expectedActiveEnergyCode;
  int32_t actualActiveEnergyCode;
  int i;
  float temp;
  temp = ((float)ADE90xx_FDSP * (float)NOMINAL_INPUT_VOLTAGE * (float)NOMINAL_INPUT_CURRENT * (float)CALIBRATION_ACC_TIME * (float)CURRENT_TRANSFER_FUNCTION *(float)calCurrentPGA_gain* (float)VOLTAGE_TRANSFER_FUNCTION *(float)calVoltagePGA_gain* (float)ADE9078_WATT_FULL_SCALE_CODES * 2 * (float)(pGaincalPF))/(float)(8192);
  expectedActiveEnergyCode = (int32_t)temp;
  Serial.print("Expected Active Energy Code: "); 
  Serial.println(expectedActiveEnergyCode,HEX);    
  
  for (i=0; i < arraySize ;i++)
    {
      actualActiveEnergyCode = accActiveEgyReg[i];

      temp= (((float)expectedActiveEnergyCode/(float)actualActiveEnergyCode)-1)* 134217728;  //calculate the gain.
      pgainReg[i] = (int32_t) temp; //Round off  
      Serial.print("Channel ");
      Serial.print(i+1); 
      Serial.print("Actual Active Energy Code: "); 
      Serial.println(actualActiveEnergyCode,HEX);       
      Serial.print("Power Gain Register: "); 
      Serial.println(pgainReg[i],HEX);      
    }
}

void calibrationEnergyRegisterSetup()
{
  uint16_t epcfgRegister;
  myADE9078.spiWrite32(MASK0_32,EGY_INTERRUPT_MASK0);   //Enable EGYRDY interrupt
  myADE9078.spiWrite16(EGY_TIME_16,EGYACCTIME);   //accumulate EGY_TIME+1 samples (8000 = 1sec)
  epcfgRegister =  myADE9078.spiRead16(EP_CFG_16);   //Read EP_CFG register
  epcfgRegister |= CALIBRATION_EGY_CFG;                //Write the settings and enable accumulation
  myADE9078.spiWrite16(EP_CFG_16,epcfgRegister);
  delay(2000); 
  myADE9078.spiWrite32(STATUS0_32,0xFFFFFFFF);
  attachInterrupt(digitalPinToInterrupt(IRQ0_INTERRUPT_PIN),updateEnergyRegisterFromInterrupt,INT_MODE);   
}


void getPGA_gain()
{
  int16_t pgaGainRegister;
  int16_t temp;  
  pgaGainRegister = myADE9078.spiRead16(PGA_GAIN_16);  //Ensure PGA_GAIN is set correctly in SetupADE9000 function.
  Serial.print("PGA Gain Register is: ");
  Serial.println(pgaGainRegister,HEX);
  temp =    pgaGainRegister & (0x0003);  //extract gain of current channel
  if (temp == 0)  // 00-->Gain 1: 01-->Gain 2: 10/11-->Gain 4
      {
        calCurrentPGA_gain =1; 
      }
  else
      {
        if(temp==1)
        {
         calCurrentPGA_gain =2;  
        }
        else
        {
         calCurrentPGA_gain =4;  
        }
      }
  temp =    (pgaGainRegister>>8) & (0x0003); //extract gain of voltage channel
  if (temp == 0)
      {
        calVoltagePGA_gain =1; 
      }
  else
      {
        if(temp==1)
        {
         calVoltagePGA_gain =2;  
        }
        else
        {
         calVoltagePGA_gain =4;  
        }
      }  
}


void storeCalConstToEEPROM()
{
  //Arrange the data as formatted in 'ADE9000_Eeprom_CalibrationRegAddress' array.
  int8_t i;
  uint32_t temp;
  uint32_t checksum=0; //  adds all the gain and phase calibration registers. The truncated 32 bit data is stored as checksum in EEPROM.
  
  for(i=0;i<IGAIN_CAL_REG_SIZE;i++) //arrange current gain calibration registers
     {
       calibrationDataToEEPROM[i]=xIgain_registers[i];

     }
  for(i=0;i<VGAIN_CAL_REG_SIZE;i++) //arrange voltage gain calibration registers
     {
       calibrationDataToEEPROM[i+IGAIN_CAL_REG_SIZE]=xVgain_registers[i];

     }
  for(i=0;i<PHCAL_CAL_REG_SIZE;i++) //arrange phase calibration registers
     {
       calibrationDataToEEPROM[i+IGAIN_CAL_REG_SIZE+VGAIN_CAL_REG_SIZE]=xPhcal_registers[i];
     }     
  for(i=0;i<PGAIN_CAL_REG_SIZE;i++) //arrange phase calibration registers
     {
       calibrationDataToEEPROM[i+IGAIN_CAL_REG_SIZE+VGAIN_CAL_REG_SIZE+PHCAL_CAL_REG_SIZE]=xPgain_registers[i];
     }

  for(i=0;i<CALIBRATION_CONSTANTS_ARRAY_SIZE;i++)    
     {
       checksum +=calibrationDataToEEPROM[i];  
     }
     


     
  for(i=0;i<CALIBRATION_CONSTANTS_ARRAY_SIZE;i++)
     {
      // myADE9078.writeWordToEeprom(ADE9000_Eeprom_CalibrationRegAddress[i],calibrationDataToEEPROM[i]);
       delay(10);         
     }
  for(i=0;i<CALIBRATION_CONSTANTS_ARRAY_SIZE;i++)
     {
//       temp= myADE9078.readWordFromEeprom(ADE9000_Eeprom_CalibrationRegAddress[i]);
       delay(10);
       Serial.println(temp,HEX);         
     }      
  //myADE9078.writeWordToEeprom(ADDR_CHECKSUM_EEPROM,checksum);           //Save checksum to EEPROM
//  myADE9078.writeByteToEeprom(ADDR_EEPROM_WRITTEN_BYTE,EEPROM_WRITTEN); //Save calibration status in EEPROM      

             
}

int8_t isRegisterPositive(int32_t registerValue)
{
  if ((int32_t)registerValue <0) 
  return 1;
  else return 0;
}

void updateEnergyRegisterFromInterrupt()
{
  int8_t i;
  static int8_t count=0;
  static int32_t intermediateActiveEgy_Reg[EGY_REG_SIZE]={0};
  static int32_t intermediateReactiveEgy_Reg[EGY_REG_SIZE]={0};
  uint32_t temp;
  temp = myADE9078.spiRead32(STATUS0_32);
  temp&=EGY_INTERRUPT_MASK0;
  if (temp==EGY_INTERRUPT_MASK0)
  {
      myADE9078.spiWrite32(STATUS0_32,0xFFFFFFFF);
      for(i=0;i<EGY_REG_SIZE;i++)
      {
        intermediateActiveEgy_Reg[i]+=myADE9078.spiRead32(xWATTHRHI_registers_address[i]);  //accumulate the registers
        intermediateReactiveEgy_Reg[i]+=myADE9078.spiRead32(xVARHRHI_registers_address[i]);   //accumulate the registers
      }
    
      if (count == (ACCUMULATION_TIME-1))  //if the accumulation time is reached, update the final values to registers
        {
          for(i=0;i<EGY_REG_SIZE;i++)
              {
                accumulatedActiveEnergy_registers[i] = intermediateActiveEgy_Reg[i];
                accumulatedReactiveEnergy_registers[i] = intermediateReactiveEgy_Reg[i];
                intermediateActiveEgy_Reg[i]=0;  // Reset the intermediate registers
                intermediateReactiveEgy_Reg[i]=0;   //Reset the intermediate registers
              }
          count=0;   //Reset counter
          return;   //exit function
        }
     count++;
     return;
  }


}
