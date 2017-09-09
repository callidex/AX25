
#define F_CPU 16000000
#define ARDUINO 100
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <TinyGPS.h>
#include <stdarg.h>
#include <digitalWriteFast.h>

#include <MicroView.h>

#include "Modem.h"
#include "SimpleFIFO.h"
#include "Packet.h"
#include "Radio.h"
#include "AFSKDecode.h"
#include "APRS.h"


//SerialCommand sCmd;

// RX on 4, TX on 5
// green on 4
// yellow on 5



// Create our AFSK modem
Modem modem;
// Enable the radio PTT on pin 7

#define PTTPORT 5
#define AUDIOOUTPORT 3

Radio radio(PTTPORT);


// GPS routines
//GPS gps;


unsigned char unprotoDigis[][7] = {
	{"WIDE1 "},
	{0x0}
};
//
//static unsigned char lastEEPROMRevision = 0;
//
//#include <avr/eeprom.h>
//#define eeprom_read_to(dst_p, eeprom_field, dst_size) eeprom_read_block(dst_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(dst_size, sizeof((__eeprom_data*)0)->eeprom_field)); eeprom_busy_wait();
//#define eeprom_read(dst, eeprom_field) eeprom_read_to(&dst, eeprom_field, sizeof(dst))
//#define eeprom_write_from(src_p, eeprom_field, src_size) eeprom_write_block(src_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(src_size, sizeof((__eeprom_data*)0)->eeprom_field)); eeprom_busy_wait();
//#define eeprom_write(src, eeprom_field) { typeof(src) x = src; eeprom_write_from(&x, eeprom_field, sizeof(x)); }
//#define MIN(x,y) (y<x?y:x)
//
//#define EEPROM_VERSION 8
struct __eeprom_data {
  byte ver; // Version
  byte revision; // Revision of EEPROM data
  char srcCallsign[6];
  unsigned char srcSSID;
  char dstCallsign[6];
  unsigned char dstSSID;
  unsigned int txInterval;
  long serialBaud;
  long kissBaud;
};

// EEPROM defaults
//const uint8_t def_ver PROGMEM = EEPROM_VERSION;
const uint8_t def_revision PROGMEM = 0;
const char  def_srcCallsign[6] PROGMEM = {'V','K','4','P','L','Y'};
const uint8_t def_srcSSID PROGMEM = 0;
const char  def_dstCallsign[6] PROGMEM = {'A','P','Z','1','0','1'};
const uint8_t def_dstSSID PROGMEM = 0;
const uint16_t def_txInterval PROGMEM = (uint16_t)30000;
const uint32_t def_serialBaud PROGMEM = (uint32_t)57600;
const uint32_t def_kissBaud PROGMEM = (uint32_t)4800;

// Store the current device configuration
static struct __eeprom_data configData;

// Compare versions of EEPROM config data and compiled variant,
// and reprogram as necessary.
void loadConfigData() {
  //eeprom_read(configData.ver, ver);
  //if(configData.ver == EEPROM_VERSION) {
    //eeprom_read(configData.revision, revision);
    //eeprom_read_to(configData.srcCallsign, srcCallsign, 6);
    //eeprom_read(configData.srcSSID, srcSSID);
    //eeprom_read_to(configData.dstCallsign, dstCallsign, 6);
    //eeprom_read(configData.dstSSID, dstSSID);
    //eeprom_read(configData.txInterval, txInterval);
    //eeprom_read(configData.serialBaud, serialBaud);
    //eeprom_read(configData.kissBaud, kissBaud);
    //if(configData.txInterval < 1000) {
      //configData.txInterval = 1000;
      //eeprom_write(configData.txInterval, txInterval);
    //}
    //lastEEPROMRevision = configData.revision;
  //} else {
    //configData.ver = pgm_read_byte_near(&def_ver);
    configData.revision = pgm_read_byte_near(&def_revision);
    configData.srcSSID = pgm_read_byte_near(&def_srcSSID);
    configData.dstSSID = pgm_read_byte_near(&def_dstSSID);
    configData.txInterval = pgm_read_word_near(&def_txInterval);
    configData.serialBaud = pgm_read_dword_near(&def_serialBaud);
    configData.kissBaud = pgm_read_dword_near(&def_kissBaud);
    for(byte i = 0; i < 6; ++i) {
      configData.srcCallsign[i] = pgm_read_byte_near(def_srcCallsign+i);
      configData.dstCallsign[i] = pgm_read_byte_near(def_dstCallsign+i);
    }
    //eeprom_write(configData.ver, ver);
    //eeprom_write(configData.revision, revision);
    //eeprom_write_from(configData.srcCallsign, srcCallsign, 6);
    //eeprom_write(configData.srcSSID, srcSSID);
    //eeprom_write_from(configData.dstCallsign, dstCallsign, 6);
    //eeprom_write(configData.dstSSID, dstSSID);
    //eeprom_write(configData.txInterval, txInterval);
    //eeprom_write(configData.serialBaud, serialBaud);
    //eeprom_write(configData.kissBaud, kissBaud);
  //}
}

// Simply output a string from program memory to the serial port
void cmdSerialOut(PGM_P p) {
	register uint8_t c;
	while((c = pgm_read_byte(p++)) != '\0') {
	//	Serial.write((uint8_t)c);
	}
	
}

// Clear bit
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
// Set bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
//
//// Set this device's own callsign
//void cmdSetMyCall() {
	//char *arg;
	//arg = sCmd.next();
	//if(arg == NULL) {
		//cmdSerialOut(PSTR("MYCALL <callsign>"));
	//} else {
		//if(strlen(arg) >= 6) {
			//memcpy(configData.srcCallsign, arg, 6);
		//} else {
			//memcpy(configData.srcCallsign, arg, strlen(arg));
			//for(unsigned char i = strlen(arg); i < 6; ++i) {
				//configData.srcCallsign[i] = ' ';
			//}
		//}		
	    //eeprom_write(++configData.revision, revision);
		//eeprom_write_from(configData.srcCallsign, srcCallsign, 6);
		//cmdSerialOut(PSTR("Set MYCALL to "));
		//Serial.println(arg);
	//}
//}
//
//// Set the default beacon interval
//void cmdSetInterval() {
    //int interval = atoi(sCmd.next());
    //if(interval < 1000)
		//interval = 1000;
    //eeprom_write(++configData.revision, revision);
	//configData.txInterval = interval;
    //eeprom_write(interval, txInterval);	
	//cmdSerialOut(PSTR("Transmit beacon interval set to "));
	//Serial.println(interval);
//}
//
//// Set the destination callsign
//void cmdSetDstCall() {
	//
//}

// Prototype for the packet header creation
static Packet *ax25MakePacketHeader(const char dst[6], const unsigned char dstSSID,
									const char src[6], const unsigned char srcSSID,
									const unsigned int len,
									const unsigned char digis[][7]);

// We'll store our serial output packet here received via TX
Packet *serTxPacket = NULL;

TinyGPS gps;
  
 
 void getgps (TinyGPS &gps)    // Define the getgps function
 // We pass in a pointer to the gps data and for extraction of the longitude and latitude etc.
 // Don’t worry if you don’t understand pointers for now… just accept the ‘&’ needs to be there!
 {
	 //  Now to define all the variables we will need in the function.

	 float flat, flon;                        //  We are going to use decimal places (floating point variables).  To save memory you can work
	 //  just in whole numbers, but this makes for results you can understand more easily.  See TinyGPS
	 //  on-line for more information.

	 int year;                                 //  Of course the year will be a whole number (integer)
	 byte month, day, hour, minute, second, hundredths;  //  Byte variables simply save memory when the result is 0-255.

	 unsigned long fix_age;      //  This tells you how old the data is from the gps unit, to see if it is ‘stale’.

	 // Now let’s go get some data!  TinyGPS does all the hard work for you with a great selection of library functions.
	 // We’ll use a variety to retrieve data, but in practice you are unlikely to want all these.
	 // Delete lines not wanted to save memory.

	 gps.f_get_position(&flat, &flon);   // returns +/- latitude/longitude in degrees.  I have no idea why the long and lat values are returned as pointers…
	 //Just roll with it for now!
	 float falt = gps.f_altitude();               // +/- altitude in meters into falt
	 float fc = gps.f_course();                   // course in degrees into fc
	 float fk = gps.f_speed_knots();       // speed in knots into fk
	 float fmph = gps.f_speed_mph();     // speed in miles/hr into fmph
	 float fmps = gps.f_speed_mps();     // speed in m/sec into fmps – this will be the one we use.
	 float fkmph = gps.f_speed_kmph();   // speed in km/hr into fkmph – unless we are in France of course!

	 //  And finally the date/time data.  The function returns pointers to all these.  Again, just refer to them without the ‘&’ prefix in code.

	 gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);

	 // Print out on MicroView screen:
	 uView.clear(PAGE);          // Clear the screen.

	 // One line of data on each ‘line’, as defined by the cursor positions below.
	 // Microview leaves the cursor where it left off unless told otherwise.  So the print commands below for date and time
	 // simply print each element in turn across the page.

	 uView.setCursor(0,0);
	 uView.print("HackTracker");

	 uView.setCursor(0,10);          // set cursor at 0,0 at print the day (so it will be a number between 1 and 31)
	 uView.print(day);                  // Print the day at position 0,0
	 uView.print("/");                   // followed by a traditional / date delimiter
	 uView.print(month);            // then the month…
	 uView.print("/");
	 uView.print(year);

	 uView.setCursor(0,20);        // set cursor at 0,10 – so we are startin a new line 10 pixels down.
	 uView.print(hour);
	 uView.print(":");
	 uView.print(minute);
	 uView.print(":");
	 uView.print(second);

	 // Now for the latitude, longitude and altitude data.
	 // The position of the data along each line is being explicitely set so they all start at the same place.

	 uView.setCursor(0,30);        // set cursor at 0,20
	 uView.print("Lat:");              // Print the label..
	 uView.setCursor(25,30);       // Set the cursor to the right of the label
	 uView.print(flat);                  // and print the latitude value

	 uView.setCursor(0,40);
	 uView.print("Lng:");
	 uView.setCursor(25,40);
	 uView.print(flon);



	 uView.setCursor(0,40);
	 uView.print("Alt:");
	 uView.setCursor(25,40);
	 uView.print(falt);
	 uView.display();
 }

void convertToDMS(float newlat,char out[])
{
	int deglat = newlat ;          // deglat = 27
	newlat -= deglat; // remove the degrees from the calculation              // should be    0.558
	newlat *= 60; // convert to minutes
	int mins =newlat;
	int minlat = newlat;
	newlat -= minlat; // remove the minuts from the calculation
	newlat *= 60; // convert to seconds
	int secs = (newlat*100)/60;
	sprintf(out,"%d%2d.%2d",deglat,mins,secs);
}

 
void setup()  {
 
	__malloc_margin = 64;
	uView.begin();
	uView.clear(PAGE);			// clear page
	
	loadConfigData();
	uView.println("Waiting");
    uView.println(" for GPS");	// display HelloWorld
	uView.println(" signal");	// display HelloWorld
	uView.display();
	Serial.begin(115200);
	//Serial.println(PMTK_SET_NMEA_UPDATE_1HZ);

  //
	// Use pin 3 for the audio output
	pinModeFast(AUDIOOUTPORT, OUTPUT);

	// Radio PTT
	pinModeFast(PTTPORT, OUTPUT);
	digitalWriteFast(PTTPORT, LOW);

	// HDLC received notice
	//pinModeFast(2, OUTPUT);
	//pinModeFast(6, OUTPUT);
	//digitalWriteFast(2, LOW);
	//digitalWriteFast(6, LOW);
  
	// Source timer2 from clkIO (datasheet p.164)
	ASSR &= ~(_BV(EXCLK) | _BV(AS2));

	// Do non-inverting PWM on pin OC2B (arduino pin 3) (p.159).
	// OC2A (arduino pin 11) stays in normal port operation:
	// COM2B1=1, COM2B0=0, COM2A1=0, COM2A0=0
	TCCR2A = (TCCR2A | _BV(COM2B1)) & ~(_BV(COM2B0) | _BV(COM2A1) | _BV(COM2A0));
  
  sbi (TCCR2A, WGM20);  // Mode 1  / Phase Correct PWM
  cbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);
  
  // No prescaler (p.162)
  TCCR2B = (TCCR2B & ~(_BV(CS22) | _BV(CS21))) | _BV(CS20);



  unprotoDigis[0][6] = 1;
  modem.start();
} 

SimpleFIFO<char,8> serPacket;

// 2 bytes HDLC, 14 bytes addressing, 2 bytes control, data, 2 bytes FCS, 2 bytes HDLC
#define PACKET_OVERHEAD 22

static Packet *ax25MakePacketHeader(const char dst[6], const unsigned char dstSSID,
									const char src[6], const unsigned char srcSSID,
									const unsigned int len,
									const unsigned char digis[][7]) {
	Packet *packet = PacketBuffer::makePacket(PACKET_OVERHEAD + len);
	unsigned char i;

	if(packet == NULL) // Check that we had memory
		return NULL;
    
	packet->start();
	// Destination address
	for(i = 0; i < 6; ++i) 
	{
		packet->appendFCS(dst[i] << 1);
	}
	packet->appendFCS(0b11100000 | (dstSSID & 0xf) << 1);
  //
  // Source address
  for(i = 0; i < 6; ++i) {
    packet->appendFCS(src[i] << 1);
  }
  if(digis == NULL) {
	  packet->appendFCS(0b01100001 | (srcSSID & 0xf) << 1); // "Last address"
  } else {
	  unsigned char d = 0;
	  packet->appendFCS(0b01100000 | (dstSSID & 0xf) << 1); // Still more for digipeaters
	  while(digis[d]) { // For each digi
		  for(i = 0; i < 6; ++i) {
			  packet->appendFCS(digis[d][i] << 1);
		  }
		  unsigned char final = 0b01100000 | (digis[d][i]&0xf)<<1;
		  if(!digis[++d]) // No more digis
			  packet->appendFCS(final | 1); // No more, final flag
		  else
			  packet->appendFCS(final);
	  }		  
  }  
  // Frame control parameters
  packet->appendFCS(0x03);
  packet->appendFCS(0xf0);
  return packet;
}

Packet *ax25MakePacket(const char dst[6], const unsigned char dstSSID, const char src[6], const unsigned char srcSSID, const char *data) {
  const unsigned char strLen = strlen(data);
  Packet *packet = ax25MakePacketHeader(
      dst, dstSSID,
      src, srcSSID,
      strLen, 0x0);
      
  if(packet == NULL)
    return NULL; // Check that we had memory
  
  // Append the actual data string
  for(unsigned char i = 0; i < strLen; ++i) {
    packet->appendFCS(data[i]);
  }
  
  packet->finish();
  
  // Return the packet
  return packet;
}

ISR(ADC_vect) 
{
	modem.timer();
}

void loop() 
{
	if(modem.txReady())
	{
		if(modem.txStart())
		{
			radio.ptt(true);
		}
	}
	else if(modem.isDone())
	{
		if(radio.ptt())
		{
			radio.ptt(false);
		}
		if(millis() - lastTx >= configData.txInterval)
		{
					byte a;
			while (Serial.available() > 0 )  // If data is available at the serial port
			{
			
				
				a = Serial.read();            // and if so read it into variable ‘a’
				if (gps.encode(a))            //If the data is valid NMEA data
				{	uView.println("Got reading");
					uView.display();
					getgps(gps);
					Packet *packet = ax25MakePacketHeader(configData.dstCallsign, configData.dstSSID,configData.srcCallsign, configData.srcSSID,128, 0x0);
					if(packet != NULL)
					{ 
							// Make sure we had memory
						float flat, flon;  
						long alt;
						gps.f_get_position(&flat, &flon);   // returns +/- latitude/longitude in degrees.  I have no idea why the long and lat values are returned as pointers…
						unsigned long fix_age, course, speed, hdop;
						unsigned short sats;
//							if(fix_age < 5000) 
						{
							alt = gps.altitude()*0.0328084;
							course = gps.course()/100;
							speed = gps.speed();
							sats = gps.satellites();
							hdop = gps.hdop()/100;
						//	packet->write('!');
							packet->write('=');
			
							char  slat[15];
							char  slon[15];
							// convert

							convertToDMS(abs(flat),slat);
							convertToDMS(abs(flon),slon);


							packet->print(slat);
	 
							if(flat>0) 
							{
								packet->write('N');
							}
							else
							{
								packet->write('S');
							}
							packet->write('//');
							packet->print(slon);
							if(flon>0)
							{
								packet->write('E');
							}
							else
							{
								packet->write('W');
								uView.println('W');			
							}
							packet->write('[');
							if(speed > 0)
							{
								//unsigned char s;
								//packet->write((uint8_t)((course/4)+33)); // Compressed course
								//for(s = 0; s < 85; ++s)
								//{
									//unsigned short int speedRef = pgm_read_word_near(speedTable + s);
									//if(speedRef >= s) break;
								//}
								//packet->write(s + 33);
								//packet->write('G'); // Current, other sentence, other tracker
							}
							else
							{
							//	packet->print(" sT");
							}
							if(alt)
							{
								packet->print("/A=");
								packet->print((unsigned short)alt);
							}
							packet->print(" HackerTracker -VK4PLY v0.1");
							packet->finish();
							if(!modem.putTXPacket(packet))
							{
								PacketBuffer::freePacket(packet);
							}
						}
					}
				}
			
			}
		}
	}
}
