#define USBSerial Serial
//#define N2KMESSAGETX // used if we want to send n2k messages

// This files contains all the n2k stuff for this project
#include "matouch-pins.h"
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include <N2kMessages.h>
#include <N2KDeviceList.h>

extern bool startUpDelayDone;
extern u_int32_t chipId;

#ifdef N2KMESSAGETX
// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM={127505L,0};

// Define schedulers for messages. Define schedulers here disabled. Schedulers will be enabled
// on OnN2kOpen so they will be synchronized with system.
// We use own scheduler for each message so that each can have different offset and period.
// Setup periods according PGN definition (see comments on IsDefaultSingleFrameMessage and
// IsDefaultFastPacketMessage) and message first start offsets. Use a bit different offset for
// each message so they will not be sent at same time.
tN2kSyncScheduler FuelTankLevelScheduler(false,500,500);
tN2kSyncScheduler PortFreshTankLevelScheduler(false,1000,450);
tN2kSyncScheduler StarbrdFreshTankLevelScheduler(false,1500,550);
tN2kSyncScheduler WasteTankLevelScheduler(false,2000,400);

// forward decalration
void SendN2kTankLevel(void);

#else 
// need a dummy buffer here
const unsigned long TransmitMessages[] PROGMEM={0};
#endif

// forward declaration
void setupN2K(void);
tN2kDeviceList *pN2kDeviceList;
uint8_t n2kDevicesConnected = 0;

typedef struct {
  unsigned long PGN;
  void (*Handler)(const tN2kMsg &N2kMsg); 
} tNMEA2000Handler;

// globals for n2k values
double locEngRPM = 0;
double locEngOilPres = 0, locEngOilTemp=0, locEngCoolTemp=0, locEngAltVolt=0, locEngFuelRate=0, locEngHours=0, locEngCoolPres=0, locEngFuelPres=0;
double locCOG=0, locSOG=0;
tN2kHeadingReference locRef;
double locWindSpeed, locWindAngle;
tN2kWindReference locWindReference;

// forward declaration
// callbacks for items to be displayed
void engineRapidUpdate(const tN2kMsg &);
void engineDynamicUpdate(const tN2kMsg &);
void fluidLevel(const tN2kMsg &);
void batteryStatus(const tN2kMsg &);
void temperatureExtended(const tN2kMsg &);
void cogsogRapid(const tN2kMsg &);
void windData(const tN2kMsg &);

tNMEA2000Handler NMEA2000Handlers[]={
  {127488L,&engineRapidUpdate},
  {127489L,&engineDynamicUpdate},
  {127505L,&fluidLevel},
  {127508L,&batteryStatus},
  {129026, &cogsogRapid},
  {130306, &windData},
  {130316L,&temperatureExtended},
  {0,0}
};

// *****************************************************************************
// Call back for NMEA2000 open. This will be called, when library starts bus communication.
// See NMEA2000.SetOnOpen(OnN2kOpen); on setup()
void OnN2kOpen() {
  #ifdef N2KMESSAGETX
  // Start schedulers now.
  FuelTankLevelScheduler.UpdateNextTime();
  PortFreshTankLevelScheduler.UpdateNextTime();
  StarbrdFreshTankLevelScheduler.UpdateNextTime();
  WasteTankLevelScheduler.UpdateNextTime();
  #endif
} // end onn2kopen


//NMEA 2000 message handler
void HandleNMEA2000Msg(const tN2kMsg &N2kMsg) {
  int iHandler;
  // Find handler
  for (iHandler=0; NMEA2000Handlers[iHandler].PGN!=0 && !(N2kMsg.PGN==NMEA2000Handlers[iHandler].PGN); iHandler++);
  if (NMEA2000Handlers[iHandler].PGN!=0) {
    NMEA2000Handlers[iHandler].Handler(N2kMsg); 
  }
} // end HandleNMEA2000Msg


// function to setup n2k
void setupN2K(void){
  
  Serial.printf ( "\nStart N2K Setup\n");
  // Set Product information
  // Tweaked to display properly on Raymarine...
  NMEA2000.SetProductInformation("V1", // Manufacturer's Model serial code
                                 103, // Manufacturer's product code
                                 "SN00000001",  // Manufacturer's Model ID
                                 "1.0.0.01 (2024-01-15)",  // Manufacturer's Software version code
                                 "MaTouch N2K Display" // Manufacturer's Model version
                                 );
  // Set device information
  NMEA2000.SetDeviceInformation(chipId, // Unique number. Use e.g. Serial number.
                                130, // Device function=Analog -> NMEA2000. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                120, // Device class=Sensor Communication Interface. See codes on https://web.archive.org/web/20190531120557/https://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                2041 // Just choosen free from code list on https://web.archive.org/web/20190529161431/http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );

  // Uncomment 2 rows below to see, what device will send to bus. Use e.g. OpenSkipper or Actisense NMEA Reader                           
  NMEA2000.SetForwardStream(&Serial);
  // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment next line
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,42);
  // attach a listener function
  NMEA2000.SetMsgHandler(HandleNMEA2000Msg);

  //NMEA2000.SetDebugMode(tNMEA2000::dm_Actisense); // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
  NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  // Here we tell library, which PGNs we transmit
  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  // Define OnOpen call back. This will be called, when CAN is open and system starts address claiming.
  NMEA2000.SetOnOpen(OnN2kOpen);

  // used for determining the number of devices on the bus
  pN2kDeviceList = new tN2kDeviceList(&NMEA2000);

  // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
  NMEA2000 . SetN2kCANMsgBufSize ( 8 );
  NMEA2000 . SetN2kCANReceiveFrameBufSize ( 250 );
  NMEA2000 . SetN2kCANSendFrameBufSize ( 250 );

  // open the interface
  NMEA2000.Open();
} // end setupn2k

// do n2k processing
void doN2Kprocessing(){
  // send any messages here
  #ifdef N2KMESSAGETX
  SendN2kTankLevel();
  #endif
  
  // now parse any rx'd messages here
  NMEA2000.ParseMessages();

  // grab the number of devices on the bus
  if (startUpDelayDone == true) {
    // update if the list changes
    if (pN2kDeviceList->ReadResetIsListUpdated() == true) {
      // get number of devices on bus
      n2kDevicesConnected = pN2kDeviceList->Count();
      //#ifdef SERIALDEBUG
      Serial.println("**********************************************************************");
      Serial.print("Number of devices updated: ");
      Serial.println(pN2kDeviceList->Count());
      Serial.println("**********************************************************************");
      //#endif
    } // end if
  } // end if
} // end doN2Kprocessing

#ifdef N2KMESSAGETX
// *****************************************************************************
void SendN2kTankLevel() {
  tN2kMsg N2kMsg;
  double level;

  // was used to force a n2k level message
  #if 0
  // force level for calibration
  if (!digitalRead(14)){
    level = 100.0;
  } else {
    level = 0.0;
  }// end if
  #endif

  if ( FuelTankLevelScheduler.IsTime() ) {
    // fuel level
    FuelTankLevelScheduler.UpdateNextTime();
    SetN2kFluidLevel(N2kMsg, 0, N2kft_Fuel, level, 75.0);
    NMEA2000.SendMsg(N2kMsg);
  }
  if ( PortFreshTankLevelScheduler.IsTime() ) {
    PortFreshTankLevelScheduler.UpdateNextTime();
    SetN2kFluidLevel(N2kMsg, 0, N2kft_BlackWater, level, 75.0);
    NMEA2000.SendMsg(N2kMsg);
  }
  if ( StarbrdFreshTankLevelScheduler.IsTime() ) {
    StarbrdFreshTankLevelScheduler.UpdateNextTime();
    SetN2kFluidLevel(N2kMsg, 0, N2kft_Water, level, 110.0);
    NMEA2000.SendMsg(N2kMsg);
  }
  if ( WasteTankLevelScheduler.IsTime() ) {
    WasteTankLevelScheduler.UpdateNextTime();
    SetN2kFluidLevel(N2kMsg, 1, N2kft_Water, level, 110.0);
    NMEA2000.SendMsg(N2kMsg);    
  }
} // end send2ktanklevel
#endif

void engineRapidUpdate(const tN2kMsg &N2kMsg) {
    unsigned char SID;
    double locEngBoost;
    int8_t locEngTilt;

    if (ParseN2kEngineParamRapid(N2kMsg,SID,locEngRPM,locEngBoost,locEngTilt) ) {
      #ifdef SERIALDEBUG
        Serial.print("Engine RPM ");
        Serial.println(locEngRPM);
      #endif
    }
} // end rapidUpdate

void engineDynamicUpdate(const tN2kMsg &N2kMsg){
    unsigned char SID;
    double locEngCoolPres, locEngFuelPres;
    int8_t locEngLoad, locEngTorque;

    // from nmea2k def'ns
    /* inline bool ParseN2kEngineDynamicParam(const tN2kMsg &N2kMsg, unsigned char &EngineInstance, double &EngineOilPress,
                      double &EngineOilTemp, double &EngineCoolantTemp, double &AltenatorVoltage,
                      double &FuelRate, double &EngineHours, double &EngineCoolantPress, double &EngineFuelPress,
                      int8_t &EngineLoad, int8_t &EngineTorque) */

    if (ParseN2kEngineDynamicParam(N2kMsg, SID, locEngOilPres, locEngOilTemp, locEngCoolTemp, locEngAltVolt, locEngFuelRate, locEngHours, locEngCoolPres, locEngFuelPres, locEngLoad, locEngTorque)) {
      #ifdef SERIALDEBUG
        Serial.print("Engine Oil Pres ");
        Serial.print(locEngOilPres);
        Serial.print(" Kpa  Engine Cool Temp ");
        Serial.print(locEngCoolTemp);
        Serial.print(" Kelvin  Engine Alt Volts ");
        Serial.print(locEngAltVolt);
        Serial.println(" volts");
      #endif
    } // end if
} // end enfineDynamicUpdate

void fluidLevel(const tN2kMsg &N2kMsg){
    unsigned char SID;
    double locLevel, locCapacity;
    tN2kFluidType locFluidType;

    if (ParseN2kFluidLevel(N2kMsg, SID, locFluidType, locLevel, locCapacity) ) {
      if (locFluidType == N2kft_Fuel) {
      #ifdef SERIALDEBUG
        Serial.print("Engine Fuel Level ");
        Serial.print(locLevel);
        Serial.println(" percent");
      #endif
      } // end if
    } // end if
} // end fluidLevel

void batteryStatus(const tN2kMsg &N2kMsg){
    unsigned char SID;
    unsigned char locBattInst;
    double locBattVolt, locBattCurrent, locBatteryTemp;

    /*inline bool ParseN2kDCBatStatus(const tN2kMsg &N2kMsg, unsigned char &BatteryInstance, double &BatteryVoltage, double &BatteryCurrent,
                     double &BatteryTemperature, unsigned char &SID)*/

    if (ParseN2kDCBatStatus(N2kMsg, locBattInst, locBattVolt, locBattCurrent, locBatteryTemp, SID) ) {
      if (locBattInst == 0) {
      #ifdef SERIALDEBUG        
        Serial.print("House Batt Voltage ");
        Serial.print(locBattVolt);
        Serial.println(" volts");
      #endif
      } // end if
    } // end if
} // end batteryStatus

void temperatureExtended(const tN2kMsg &N2kMsg){
    unsigned char SID, locTempInstance;
    double locTemp, locTempSet;
    tN2kTempSource locTempSource;

    /*bool ParseN2kPGN130316(const tN2kMsg &N2kMsg, unsigned char &SID, unsigned char &TempInstance, tN2kTempSource &TempSource,
                     double &ActualTemperature, double &SetTemperature) */

    if (ParseN2kPGN130316(N2kMsg, SID, locTempInstance, locTempSource, locTemp, locTempSet) ) {
      if (locTempSource == N2kts_EngineRoomTemperature) {
      #ifdef SERIALDEBUG
        Serial.print("Engine Room Temp ");
        Serial.print(locTemp);
        Serial.println(" Kelvin");
      #endif
      } // end if
      if (locTempSource == N2kts_ExhaustGasTemperature) {
      #ifdef SERIALDEBUG
        Serial.print("Engine Exhaust Gas Temp ");
        Serial.print(locTemp);
        Serial.println(" Kelvin");
      #endif
      } // end if
    } // end if
} // end temperatureExtended

void cogsogRapid(const tN2kMsg &N2kMsg){
    unsigned char SID, locTempInstance;

    /*inline bool ParseN2kCOGSOGRapid(const tN2kMsg &N2kMsg, unsigned char &SID, tN2kHeadingReference &ref, double &COG, double &SOG) {
        return ParseN2kPGN129026(N2kMsg,SID,ref,COG,SOG); */

    if (ParseN2kCOGSOGRapid(N2kMsg, SID, locRef, locCOG, locSOG) ) {
      #ifdef SERIALDEBUG
        Serial.print("COG ");
        Serial.print(locCOG);
        Serial.print("   SOG ");
        Serial.print(locSOG);
        Serial.println(" m/s");
      #endif          
    } // end if
} // end temperatureExtended

void windData(const tN2kMsg &N2kMsg){
    unsigned char SID, locTempInstance;

    /* inline bool ParseN2kWindSpeed(const tN2kMsg &N2kMsg, unsigned char &SID, double &WindSpeed, double &WindAngle, tN2kWindReference &WindReference) {
        return ParseN2kPGN130306(N2kMsg,SID,WindSpeed,WindAngle,WindReference); */

    if (ParseN2kWindSpeed(N2kMsg, SID, locWindSpeed, locWindAngle, locWindReference) ) {
      #ifdef SERIALDEBUG
        Serial.print("Wind Speed ");
        Serial.print(locWindSpeed);
        Serial.print("m/s   Wind Angle ");
        Serial.print(locWindAngle);
        Serial.print("rad   Wind Reference ");
        Serial.print(locWindReference);
        Serial.println(" ");
      #endif          
    } // end if
} // end temperatureExtended
