// Basic Test Demonstration for ADE9078 to read and report values (ADE9078_TEST)
//California Plug Load Research Center - 2019


#include <ADE9078.h>
#include <SPI.h>
#include <EEPROM.h>


//Architecture Control:
//Make sure you select in the ADE9078.h file the proper board architecture, either Arduino/AVR/ESP8266 or ESP32
//REMINDER: ONLY SELECT THE SINGLE OPTION FOR THE BOARD TYPE YOU ARE USING!


//Define ADE9078 object with hardware parameters specified
#define local_SPI_freq 1000000  //Set SPI_Freq at 1MHz  - used for Arduino/AVR configuration
#define local_SS 10  //Set the SS pin for SPI communication as pin 10, typical on Arduino Uno and similar boards

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
  char* sep = "#";
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
  char* sep = "#";
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
  //when using the recalled values (ouside of this function, the following approaches can be used to make the arrays that hold values as characters into numerical values for use:
  //atoi() can be used to generate integers needed from char arrays for loading, e.g. integerFromPC = atoi(strtokIndx);     // convert this part to an integer strtokIndx = strtok(NULL, ",");floatFromPC = atof(strtokIndx);     // convert this part to a float (see: http://forum.arduino.cc/index.php?topic=396450)
    //floatFromPC = atof(strtokIndx);     // convert this part to a float
  Serial.println("<--Read data complete, this was read");
}//*****************************************************

struct InitializationSettings* is = new InitializationSettings; //define structure for initialized values

ADE9078 myADE9078(local_SS, local_SPI_freq, is); // Call the ADE9078 Object with hardware parameters specified, local variables are copied to private variables inside the class when object is created.




void setup() {

	//Need to add:
	//1)Read in EEPROM values, check the "Configured bit", if 1, display, calibration values available, load these calibration values to the ADE9078's RAM
	//2)If 0, then display that no calibration values are present and load default values (Tell user to run calibration firmware)
	//3) Proceed with load and operation of this demo
  Serial.begin(115200);
  delay(200);
  is->vAGain=1;
  is->vBGain=1;
  is->vCGain=1;

  is->iAGain=1;
  is->iBGain=1;
  is->iCGain=1;
  is->iNGain=1;

  is->powerAGain=1;
  is->powerBGain=1;
  is->powerCGain=1;

  //Use these settings to configure wiring configuration at stertup
  //FYI: B010=2 in DEC
  //FYI: B100 = 4 in DEC

	//4 Wire Wye configuration - non-Blondel compliant:
	is->vConsel=0;
	is->iConsel=0;

	//Delta, Blondel compliant:
	//is->vConsel=4; //byte value of 100,
	//is->iConsel=0;


	 //Please continue for all cases

  SPI.begin();
  delay(200);
    myADE9078.initialize(); //Call initialization of the ADE9078 withe default configuration plus options specified
	//EEPROMInit()  //call only once on a virgin chip to "partition" EEPROM for the input type expected moving forward
	//load_data_allfields();  //load EEPROM values

}


void loop() {

  double AvRMS, BvRMS, CvRMS;
  double IrmsA, IrmsB, IrmsC;
  double wattHoursA;
  double apparentPowerA, apparentPowerB, apparentPowerC;
  double reactivePowerA, reactivePowerB, reactivePowerC;
  double activePowerA, activePowerB, activePowerC;


  //Serial.print("Version_16: ");
  //Serial.println(myADE9078.getVersion()); // PHNOLOAD_32
  //delay(20);

  //Serial.print("PHNOLOAD_32: ");
  //Serial.println(myADE9078.getPHNOLOAD()); // PHNOLOAD_32
  //delay(20);

  int32_t VrmsStorageA[64] = {};
  int32_t VrmsStorageB[64] = {};
  int32_t VrmsStorageC[64] = {};

  for (int i=0; i < 64; ++i)
  {
      myADE9078.readVrms();
      VrmsStorageA[i] = myADE9078.lastReads.vrms.a;
      VrmsStorageB[i] = myADE9078.lastReads.vrms.b;
      VrmsStorageC[i] = myADE9078.lastReads.vrms.c;
  }

  Serial.print("Av:");
  for (int i=0; i<64; ++i)
  {
      Serial.print(VrmsStorageA[i]);
      Serial.print(",");
  }
  Serial.println("$");

  Serial.print("Bv:");
  for (int i=0; i<64; ++i)
  {
      Serial.print(VrmsStorageB[i]);
      Serial.print(",");
  }
  Serial.println("$");

  // Serial.print("Cv:");
  // for (int i=0; i<64; ++i)
  // {
  //     Serial.print(VrmsStorageC[i]);
  //     Serial.print(",");
  // }
  //  Serial.println("$");

  // myADE9078.readVrms();
  // Serial.print("A, B, C rms (V): ");
  // Serial.print(myADE9078.lastReads.vrms.a);
  // Serial.print(" ");
  // Serial.print(myADE9078.lastReads.vrms.b);
  // Serial.print(" ");
  // Serial.print(myADE9078.lastReads.vrms.c);
  // Serial.print(" ");


  // AvRMS = myADE9078.getAVrms();
  // Serial.print("AVrms (V): ");
  // Serial.println(AvRMS);
  // delay(10);
  //
  // BvRMS = myADE9078.getBVrms();
  // Serial.print("BVrms (V): ");
  // Serial.println(BvRMS);
  // delay(10);
  //
  // CvRMS = myADE9078.getCVrms();
  // Serial.print("BVrms (V): ");
  // Serial.println(BvRMS);
  // delay(10);

  //myADE9078.readIrms();
  //Serial.print("A, B, C rms (I): ");
  //Serial.print(myADE9078.lastReads.irms.a);
  //Serial.print(" ");
  //Serial.print(myADE9078.lastReads.irms.b);
  //Serial.print(" ");
  //Serial.print(myADE9078.lastReads.irms.c);
  //Serial.print(" ");

  // IrmsA = myADE9078.getIrmsA();
  // Serial.print("IrmsA (mA): ");
  // Serial.println(IrmsA);
  // delay(10);
  //
  // IrmsB = myADE9078.getIrmsB();
  // Serial.print("IrmsB (mA): ");
  // Serial.println(IrmsB);
  // delay(10);
  //
  // IrmsC = myADE9078.getIrmsC();
  // Serial.print("IrmsA (mA): ");
  // Serial.println(IrmsC);
  // delay(10);

  // wattHoursA = myADE9078.readWattHoursA();
  // Serial.print("WattHoursA: ");
  // Serial.println(wattHoursA);
  // delay(10);
  //
  // apparentPowerA = myADE9078.getInstApparentPowerA();
  // Serial.print("Apparent Power A (mW): ");
  // Serial.println(apparentPowerA);
  // delay(10);
  //
  // apparentPowerB = myADE9078.getInstApparentPowerB();
  // Serial.print("Apparent Power B (mW): ");
  // Serial.println(apparentPowerB);
  // delay(10);
  //
  // apparentPowerC = myADE9078.getInstApparentPowerC();
  // Serial.print("Apparent Power C (mW): ");
  // Serial.println(apparentPowerC);
  // delay(10);
  //
  // activePowerA = myADE9078.getInstActivePowerA();
  // Serial.print("Active Power A (mW): ");
  // Serial.println(activePowerA);
  // delay(10);
  //
  // activePowerB = myADE9078.getInstActivePowerB();
  // Serial.print("Active Power B (mW): ");
  // Serial.println(activePowerB);
  // delay(10);
  //
  // activePowerC = myADE9078.getInstActivePowerC();
  // Serial.print("Active Power C (mW): ");
  // Serial.println(activePowerC);
  // delay(10);

  //reactivePowerA = myADE9078.getInstReactivePowerA();
  //Serial.print("Reactive Power A (mW): ");
  //Serial.println(reactivePowerA);
  //delay(10);

  //reactivePowerB = myADE9078.getInstReactivePowerB();
  //Serial.print("Reactive Power B (mW): ");
  //Serial.println(reactivePowerA);
  //delay(10);

  //reactivePowerC = myADE9078.getInstReactivePowerC();
  //Serial.print("Reactive Power C (mW): ");
  //Serial.println(reactivePowerA);
  //delay(10);

  //delay(1000); //Delay before repeating the loop
}
