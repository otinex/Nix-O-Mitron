/*
    Name:       InternetClockGateway.ino
    Created:	20.08.2018 20:02:10
    Author:     Melchi\R
*/

#include <WiFiManager.h>			//https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266WebServer.h>		//Local WebServer used to serve the configuration portal
#include <ESP8266WiFi.h>			//ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>				//Local DNS Server used for redirecting all requests to the configuration portal
#include <WiFiUdp.h>				// we need UDP to communicate with the NTP Server


unsigned int localPort = 2390;		//local UDP port to listen for packets
const char *ntpServerName = "at.pool.ntp.org";	//NTP Server for my location (Austria)
IPAddress timeServerIP;				//time server IP address
const int NTP_PACKET_SIZE = 48;		//length of the NTP packet that contains the timestamp
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold the bytes from the NTP packet
WiFiUDP udp;						//instance of the UDP object

void setup()
{	
	Serial.begin(115200);
	WiFiManager wifiManager;
	//wifiManager.resetSettings();
	wifiManager.autoConnect("Nix-O-Mitron 3000", "123456789");
	Serial.println("Connected");
	Serial.println("starting UDP...");
	udp.begin(localPort);
	Serial.println("Local UDP Port:");
	Serial.println(udp.localPort());
	WiFi.begin();
}

void loop()
{
	while (!Serial.available())
	{
		//wait for the serial connection to be established
	}
	if (Serial.read() == 'a')
	{
		WiFi.hostByName(ntpServerName, timeServerIP);
		sendNTPpacket(timeServerIP); // send an NTP packet to a time server
		delay(1000); // wait to see if a reply is available

		int cb = udp.parsePacket();
		if (!cb) {
			Serial.println("no packet yet");
		}
		else {
			Serial.print("packet received, length=");
			Serial.println(cb);
			// We've received a packet, read the data from it
			udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

			//the timestamp starts at byte 40 of the received packet and is four bytes,
			// or two words, long. First, esxtract the two words:

			unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
			unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
			// combine the four bytes (two words) into a long integer
			// this is NTP time (seconds since Jan 1 1900):
			unsigned long secsSince1900 = highWord << 16 | lowWord;
			Serial.print("Seconds since Jan 1 1900 = ");
			Serial.println(secsSince1900);

			// now convert NTP time into everyday time:
			Serial.print("Unix time = ");
			// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
			const unsigned long seventyYears = 2208988800UL;
			// subtract seventy years:
			unsigned long epoch = secsSince1900 - seventyYears;
			// print Unix time:
			Serial.println(epoch);


			// print the hour, minute and second:
			Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
			Serial.print((epoch % 86400L) / 3600); // print the hour (86400 equals secs per day)
			Serial.print(':');
			if (((epoch % 3600) / 60) < 10) {
				// In the first 10 minutes of each hour, we'll want a leading '0'
				Serial.print('0');
			}
			Serial.print((epoch % 3600) / 60); // print the minute (3600 equals secs per minute)
			Serial.print(':');
			if ((epoch % 60) < 10) {
				// In the first 10 seconds of each minute, we'll want a leading '0'
				Serial.print('0');
			}
			Serial.println(epoch % 60); // print the second
		}
		// wait ten seconds before asking for the time again
	}
	else
	{
		Serial.println("Command not found");
	}

	//delay(10000);

}
// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
	Serial.println("sending NTP packet...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); //NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}