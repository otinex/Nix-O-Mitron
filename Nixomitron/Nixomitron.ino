#include <DS3231.h>
#include <CapacitiveSensor.h>
//#include <DHT.h>
#define DHTPIN 5
#define DHTTYPE DHT22

CapacitiveSensor cs = CapacitiveSensor(4, 2);        // 10 megohm resistor between pins 4 & 2, pin 2 is sensor pin, add wire, foil
DS3231 rtc(SDA, SCL);
//DHT dht(DHTPIN,DHTTYPE,1);

Time t;
const int latchPin = 8;
const int clockPin = 12;
const int dataPin = 11;
char secondByte = 0, minuteByte = 0, hourByte = 0;
int mode = 10;
bool currentread = false, lastread = false, changeState;
unsigned long previousMillis = 0;
const long interval = 100;
bool reading, oldreading;
int temp;


void setup()
{
	pinMode(latchPin, OUTPUT);
	pinMode(dataPin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	cs.set_CS_AutocaL_Millis(0xFFFFFFFF); //turn off autocalibrate
	//dht.begin();

										  // Setup Serial connection
	Serial.begin(9600);

	// Uncomment the next line if you are using an Arduino Leonardo
	//while (!Serial) {}

	// Initialize the rtc object

	rtc.begin();

////	 The following lines can be uncommented to set the date and time
	 //rtc.setDOW(SATURDAY);     // Set Day-of-Week to SUNDAY
	 //rtc.setTime(15, 44, 30);     // Set the time to 12:00:00 (24hr format)
	 //rtc.setDate(25, 03, 2017);   // Set the date to January 1st, 2014
}

void loop()
{

	//Send Day-of-Week
	Serial.print(rtc.getDOWStr());
	Serial.print(" ");

	// Send date
	Serial.print(rtc.getDateStr());
	Serial.print(" -- ");

	// Send time
	Serial.println(rtc.getTimeStr());
	Serial.println(t.sec);



	//temp = dht.readTemperature();
	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= interval) {
		
		previousMillis = currentMillis;
		currentread = getSensorState(100);

		Serial.println(currentread);
		Serial.println(mode);

		if (currentread != lastread) {
			if (lastread == true) {
				changeState = true;
			}
		}
		else {
			changeState = false;
		}

		switch (mode) {
			case 10: {
				showTime();
			}
					 break;
			case 20: {
				cycle();
			}
					 break;
			case 30: {
				blankOut();
				break;
			}
			case 40: {
				showTemp();
				break;
			}
			default:
			{

			}
		}	
	}

	delay(50);
	lastread = currentread;
	changeState = false;

}


byte decToBcd(int val)
{
	return((val / 10 * 16) + (val % 10));
}

byte swap(byte val)
{
	return(val << 4) | (val >> 4);
}

void cycle(void)
{
	changeState = false;
	for (int i = 0; i <= 9; i++) {
		digitalWrite(latchPin, LOW);
		shiftOut(dataPin, clockPin, MSBFIRST, swap(decToBcd(i * 10 + i)));
		shiftOut(dataPin, clockPin, MSBFIRST, swap(decToBcd(i * 10 + i)));
		shiftOut(dataPin, clockPin, MSBFIRST, swap(decToBcd(i * 10 + i)));
		digitalWrite(latchPin, HIGH);
		delay(100);
	}
	mode = 10;
}

void blankOut(void) {
	
	delay(100);

	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, 255);
	shiftOut(dataPin, clockPin, MSBFIRST, 255);
	shiftOut(dataPin, clockPin, MSBFIRST, 255);
	digitalWrite(latchPin, HIGH);

	if (changeState == 1) { mode = 10;}
	changeState = false;
}

bool getSensorState(int threshold) {
	long start = millis();
	long total = cs.capacitiveSensor(30);

	if (total > threshold) {
		reading = true;
	}
	else {
		reading = false;
	}

	if (reading != oldreading) {
		if (reading == true) {
			return true;
		}
		else return false;
	}
	oldreading = reading;
}

void showTime(void) {
	
	t = rtc.getTime();

	secondByte = swap(decToBcd(t.sec));
	minuteByte = swap(decToBcd(t.min));
	hourByte = swap(decToBcd(t.hour));

	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, secondByte);
	shiftOut(dataPin, clockPin, MSBFIRST, minuteByte);
	shiftOut(dataPin, clockPin, MSBFIRST, hourByte);
	digitalWrite(latchPin, HIGH);
	if (t.sec == 0) { mode = 20; }
	else if (changeState == 1) { mode = 30; }
	changeState = false;

}

void showTemp(void) {
	
	//int temp = dht.readTemperature();

	digitalWrite(latchPin, LOW);
	shiftOut(dataPin, clockPin, MSBFIRST, 255);
	shiftOut(dataPin, clockPin, MSBFIRST, swap(decToBcd(temp)));
	shiftOut(dataPin, clockPin, MSBFIRST, 255);
	digitalWrite(latchPin, HIGH);

	if (changeState == 1) { mode = 30;}
	

}
