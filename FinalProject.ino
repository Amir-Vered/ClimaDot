//import libraries
#include <LiquidCrystal.h>
#include <dht11.h>
#include <IRremote.hpp>

//define lcd variables
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//define dht sensor variables
const int dp = 7;
dht11 dht;

//define ir reader variables
const int irp = 13;
IRrecv irrecv(irp);

//define button and contrast potentiometer variables
const int bp = 2;
const int cp = A0;

//set up an enum to store the page number
enum Page {
  one,
  two,
  three,
  def
};

//initialize state variables
Page page = one;
bool fer = false;
bool power = true;

void setup() {
  //Set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  //Start the receiver
  irrecv.enableIRIn();

  //set up the button as an interupt to control power
  attachInterrupt(digitalPinToInterrupt(bp), powerHandler, RISING);

  //run the initial humidity reading, so the while loop in loop() can saver memory
  int check = dht.read(dp);
  char sBuf[17];
  char pBuf[17]; 
  snprintf(sBuf, sizeof(sBuf), "Humidity: %d%%   ", dht.humidity);
  pageNumber(pBuf, 1);
  lcd.setCursor(0, 0);
  lcd.print(sBuf);
  lcd.setCursor(0, 1);
  lcd.print(pBuf);
}

void loop() {
  while (!irrecv.decode()) { //if signal is not recieved
    delay(10); //.01s 
  }

  //handle readings
  translateIR();
  irrecv.resume(); // receive the next value

  int check = dht.read(dp); //read dht

  char sBuf[17]; // first line
  char pBuf[17]; // second line

  //page state switch statement
  switch (page) {
    case one:
      snprintf(sBuf, sizeof(sBuf), "Humidity: %d%%   ", dht.humidity); //Humidity: ##%
      pageNumber(pBuf, 1);
      break;
    case two: {
      float temp = dht.temperature; //take temperature reading
      char tempStr[10]; //initialize char buffer
      if (fer) { //f or c switch
        dtostrf(ctof(temp), 4, 1, tempStr);
        snprintf(sBuf, sizeof(sBuf), "Temp: %sF   ", tempStr); //Temp: ##.#F
      } else {
        dtostrf(temp, 4, 1, tempStr);
        snprintf(sBuf, sizeof(sBuf), "Temp: %sC   ", tempStr); //Temp: ##.#C
      }
      pageNumber(pBuf, 2);
      break;
    }
    case three:
      //use map to convert count to percentage 
      snprintf(sBuf, sizeof(sBuf), "Contrast: %d%%   ", map(analogRead(cp), 0, 1023, 100, 0)); //Contrast: ##% 
      pageNumber(pBuf, 3);
      break;
    default:
      //default case should never occur
      snprintf(sBuf, sizeof(sBuf), "Invalid Page");
      snprintf(pBuf, sizeof(pBuf), "???");
      break;
  }

  //print to lcd screen
  lcd.setCursor(0, 0);
  lcd.print(sBuf);
  lcd.setCursor(0, 1);
  lcd.print(pBuf);
}

void translateIR()
{
  //map the IR code to the remote key
  switch (irrecv.decodedIRData.decodedRawData) {
    case 0xBA45FF00: //POWER
      powerHandler();
      break;
    case 0xB847FF00: //FUNC
      fer = !fer;
      break;
    case 0xF30CFF00: //1
      page = one;   
      break;
    case 0xE718FF00: //2
      page = two; 
      break;
    case 0xA15EFF00: //3
      page = three;
      break;
    default: break; //do nothing
  }
  
  delay(500); //0.5s
}

//converts celcius to fahrenheit
float ctof(float c) {
  return (1.8 * c) + 32;
}

//either enables or disables the lcd screen
void powerHandler() {
  if (power) {
    lcd.noDisplay();
  } else {
    lcd.display();
  }
  power = !power;
}

//creates a char buffer with the inputed number at the end
void pageNumber(char* buf, int n) {
    for (int i = 0; i < 15; i++) buf[i] = ' '; // Fill first 15 characters
    buf[15] = '0' + n; //number
    buf[16] = '\0'; // null terminator
}
