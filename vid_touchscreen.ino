


#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>
#include <Fonts/FreeMono9pt7b.h>
#include <EEPROM.h>

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
#define JJCOLOR         0x1CB6
//______________________________________________________________________________
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0 
// Color definitions - in 5:6:5
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF
#define TEST            0x1BF5
#define JJCOLOR         0x1CB6
#define JJORNG          0xFD03
//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, 0);
int i = 0;
int page = 0;
int blv;
int sleep = 0;
int pulsev = 0;
int redflag = 0;
int greenflag = 0;
int redled = 2;
int greenled = A4;
int backlight = 5;
int skip4 = 0;
int skip3 = 0;

int out1 = 31;
int out2 = 32;
int out3 = 33;
int out4 = 34;

int battfill;
unsigned long sleeptime;
unsigned long battcheck = 10000; // the amount of time between voltage check and battery icon refresh
unsigned long prevbatt;
int battv;
int battold;
int battpercent;
int barv;
int prevpage;
int sleepnever;
int esleep;
int backlightbox;
int antpos = 278;
unsigned long awakeend;
unsigned long currenttime;
unsigned long ssitime;
char voltage[10];
char battpercenttxt [10];
//------------------------------------------------------
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
//------------------------------------------------------

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;





//-----------------------------------------------------------------------------------------------------------
void setup(void) {
  
  while (!Serial);     // used for leonardo debugging
 
  Serial.begin(115200);
  Serial.println(F("Video Controller Started"));
  
  tft.begin();

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  Serial.println("Capacitive touchscreen started");
  
  tft.fillScreen(ILI9341_BLACK);
    //tft.setFont(&FreeMono9pt7b);
  tft.setRotation(3);
  /*
  // make the color selection boxes
  tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
  tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_YELLOW);
  tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, ILI9341_GREEN);
  tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN);
  tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, ILI9341_BLUE);
  tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, ILI9341_MAGENTA);
  // select the current color 'red'
  tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  currentcolor = ILI9341_RED;

 //unsigned long testText() {
  //tft.fillScreen(ILI9341_BLACK);
  
  */
  
  esleep = EEPROM.read(1);
  blv = EEPROM.read(2);
//esleep = 3; // uncomment this and run once if you have not used the EEPROM before on your Arduino! Comment and reload after that.
//blv = 255; // uncomment this and run once if you have not used the EEPROM before on your Arduino! Comment and reload after that.c
  if (esleep == 1) {
    sleeptime = 10000;
  }
  if (esleep == 2) {
    sleeptime = 20000;
  }
  if (esleep == 3) {
    sleeptime = 30000;
  }
  if (esleep == 4) {
    sleeptime = 60000;
  }
  if (esleep == 5) {
    sleeptime = 120000;
  }
  if (esleep == 6) {
    sleeptime = 300000;
  }
  if (esleep == 7) {
    sleeptime = 600000;
  }
  if (esleep == 8) {
    sleeptime = 1200000;
  }
  if (esleep == 9) {
    sleeptime = 1800000;
  }
  if (esleep == 10) {
    sleeptime = 3600000;
  }
  if (esleep == 11) {
    sleeptime = 14400000;
  }
  if (esleep == 12) {
    sleepnever = 1;
  }
  awakeend = sleeptime + 1000; // set the current sleep time based on what the saved settings in EEPROM were
  pinMode(backlight, OUTPUT);
  
  pinMode(out1, OUTPUT);
  pinMode(out2, OUTPUT);
  pinMode(out3, OUTPUT);
  pinMode(out4, OUTPUT);
  
  
  
  
  analogWrite(backlight, 255);
  
tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  drawhomeicon(); // draw the home icon
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.print("           Home ");
  tft.drawRect(297, 1, 20, 8, ILI9341_WHITE); //battery body
  tft.fillRect(317, 3, 2, 4, ILI9341_WHITE); // battery tip
  tft.fillRect(298, 2, 18, 6, ILI9341_BLACK); // clear the center of the battery
  //drawbatt();
  ant(); // draw the bas "antenna" line without the "signal waves"
  signal(); // draw the "signal waves" around the "antenna"
  homescr(); // draw the homescreen
  tft.drawRect(0, 200, 245, 40, ILI9341_WHITE); // message box
  

  
  
}

void loop() {
  
  /*
  Serial.print("esleep ="); Serial.println(esleep);
  Serial.print("sleep ="); Serial.println(sleep);
  Serial.print("awakeend ="); Serial.println(awakeend);
  Serial.print("sleepnever ="); Serial.println(sleepnever);
  Serial.print("backlight ="); Serial.println(backlight);
  Serial.print("blv ="); Serial.println(blv);
  Serial.print("page ="); Serial.println(page);
  */
  
  // Wait for a touch
  if (! ctp.touched()) {
    return;
  }

  // Retrieve a point  
  TS_Point p = ctp.getPoint();
  
  // Print out raw data from screen touch controller
  Serial.print("X = "); Serial.print(p.x);
  Serial.print("\tY = "); Serial.print(p.y);
  Serial.print(" -> ");
  
 

  // flip it around to match the screen. map(value, fromLow, fromHigh, toLow, toHigh)
  //p.x = map(p.x, 0, 240, 240, 0);
  //p.y = map(p.y, 0, 320, 320, 0);
  int newx = p.y ;         //change the touchscreen to match the landscape graphics
  int newy = (240-p.x);   //change the touchscreen to match the landscape graphics
  p.x = newx;
  p.y = newy;

  // Print out the remapped (rotated) coordinates   
  Serial.print("("); Serial.print(p.x);
  Serial.print(", "); Serial.print(p.y);
  Serial.println(")");
  Serial.print(page);
  //___________________________________________________________________________________________
  
  
  
  
 /*
  
  //                                                     ---CHECKING TO SEE IF SCREEN HAS BEEN TOUCHED - DISPLAY SLEEP----
  currenttime = millis();
  unsigned long currentawake = millis();
  if((currentawake > awakeend) && (sleepnever == 0)) {
    if (sleep == 0) {
      for(i = blv ; i >= 0; i-=1) {
        analogWrite(backlight, i);
        delay(4);
      }
      sleep = 1;
    }
  
 */
 
    
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (ctp.touched()) {
    
    
    awakeend = currenttime + sleeptime; //set the sleep time when screen is pressed
    if (sleep == 1) { // if asleep, then fade the backlight up
      for(i = 0 ; i <= blv; i+=1) { 
        analogWrite(backlight, i);
        delay(1);
      }
      sleep = 0; // change the sleep mode to "awake"
      return;
    }
    


    // area 1
    if (p.y > 0 && p.y < 70 && p.x > 0 && p.x < 150) { // if this area is pressed
      if (page == 5) { // and if page 5 is drawn on the screen
        /*
        m5b1action(); // do whatever this button is
        tft.setTextColor(RED);
        tft.setTextSize(2);
        tft.setCursor(12, 213);
        tft.print("Menu 5 B1"); // display the command in the "message box"
        yled(550); // flash the LED yellow for a bit - change the 550 value to change LED time on
        clearmessage(); // after the LED goes out, clear the message
        */
      }
      if (page == 4) {
        m4b1action();
        tft.setCursor(12, 213);
        tft.print("Front of Truck");
        yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b1action();
        tft.setCursor(12, 213);
        tft.print("Trailer Driver ");
        yled(550);
        clearmessage();
      }
      if (page == 2) {
        
      }
      if (page == 1) {
        m1b1action();
        tft.setCursor(12, 213);
        tft.print("Front Camera");
        yled(550);
        clearmessage();
      }
      if (page == 0) { // if you are on the "home" page (0)
        page = 1; // then you just went to the first page
        
        redraw(); // redraw the screen with the page value 1, giving you the page 1 menu
      }
    }
    
    
    // area 2
    if (p.y > 0 && p.y < 70 && p.x > 170 && p.x < 320) {
      if (page == 5) {
        page = 4;
      
        redraw();
        skip4 = 1;
        
      }
      if (page == 4 && skip4 == 0) {
        m4b2action();
        tft.setCursor(12, 213);
        tft.print("Menu 4 B2");
        yled(550);
        clearmessage();
        skip4=0;
      }
      if (page == 3) {
        m3b2action();
        tft.setCursor(12, 213);
        tft.print("Trailer Passenger");
        yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b2action();
        tft.setCursor(12, 213);
        tft.print("Truck Mirror");
        yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b2action();
        tft.setCursor(12, 213);
        tft.print("Back and Down");
        yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 2;
        redraw();
      }
    }
    // area 3
    if (p.y > 80 && p.y < 130 && p.x > 0 && p.x < 150) {
      if (page == 5) {
       /*
        m5b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 5 B3");
        yled(550);
        clearmessage();
        */
      }
      if (page == 4) {
        m4b3action();
        tft.setCursor(12, 213);
        tft.print("Forward Driver");
        yled(550);
        clearmessage();
      }
      if (page == 3) {
        /*
        m3b3action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B3");
        yled(550);
        clearmessage();
        */
      }
      if (page == 2) {
        
      }
      if (page == 1) {
        m1b3action();
        tft.setCursor(12, 213);
        tft.print("Truck Driver Side");
        yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 3;
        redraw();
      }
    }
    // area 4
    if (p.y > 80 && p.y < 130 && p.x > 170 && p.x < 320) {
      if (page == 5) {
       page = 3;
       skip3 = 1;
        clearmessage();
        redraw();;
      }
      if (page == 4) {
        m4b4action();
        tft.setCursor(12, 213);
        tft.print("Forward Passenger");
        yled(550);
        clearmessage();
      }
      if (page == 3 && skip3 == 0) {
        m3b4action();
        tft.setCursor(12, 213);
        tft.print("Menu 3 B4");
        yled(550);
        clearmessage();
        skip3 = 0;
      }
      if (page == 2) {
        m2b4action();
        tft.setCursor(12, 213);
        tft.print("Trailer Passenger");
        yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b4action();
        tft.setCursor(12, 213);
        tft.print("Truck Passenger");
        yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 4;
        redraw();
      }
    }
    // area 5
    if (p.y > 140 && p.y < 190 && p.x > 0 && p.x < 150) {
      if (page == 5) {
       
      }
      if (page == 4) {
        m4b5action();
        tft.setCursor(12, 213);
        tft.print("Backwards Driver");
        yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b5action();
        tft.setCursor(12, 213);
        tft.print("End of Trailer");
        yled(550);
        clearmessage();
      }
      if (page == 2) {
        
      }
      if (page == 1) {
        m1b5action();
        tft.setCursor(12, 213);
        tft.print("Trailer Driver");
        yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 5;
        redraw();
      }
    }
    // area 6
    if (p.y > 140 && p.y < 190 && p.x > 170 && p.x < 320) {
      if (page == 5) {
        
      }
      if (page == 4) {
        m4b6action();
        tft.setCursor(12, 213);
        tft.print("Backwards Passenger");
        yled(550);
        clearmessage();
      }
      if (page == 3) {
        m3b6action();
        tft.setCursor(12, 213);
        tft.print("Back and Down");
        yled(550);
        clearmessage();
      }
      if (page == 2) {
        m2b6action();
        tft.setCursor(12, 213);
        tft.print("End of Trailer");
        yled(550);
        clearmessage();
      }
      if (page == 1) {
        m1b6action();
        tft.setCursor(12, 213);
        tft.print("Trailer Passenger");
        yled(550);
        clearmessage();
      }
      if (page == 0) {
        page = 6;
        redraw();
      }
    }
    // home
    if (p.y > 200 && p.y < 240 && p.x > 280 && p.x < 320) { // if the home icon is pressed
      if (page == 6) { // if you are leaving the settings page
        clearmessage(); // clear the battery voltage out of the message box
        tft.setTextSize(2);
        tft.setTextColor(YELLOW);
        tft.setCursor(12, 213);
        tft.print("Settings Saved"); // display settings saved in message box
        EEPROM.write(1, esleep); // write the sleep value to EEPROM, so it will not lose settings without power
        EEPROM.write(2, blv); // write the backlight value to EEPROM, so it will not lose settings without power
        clearsettings(); // erase all the drawings on the settings page
        
        
        
      }
      if (page == 0) { // if you are already on the home page
        drawhomeiconred(); // draw the home icon red
        delay(250); // wait a bit
        drawhomeicon(); // draw the home icon back to white
        return; // if you were on the home page, stop.
      }
      else { // if you are not on the settings, home, or keyboard page
        page = prevpage; // a value to keep track of what WAS on the screen to redraw/erase only what needs to be
        page = 0; // make the current page home
        
        redraw(); // redraw the page
      }
      
      
      
    }
    // message area
    if (p.y > 200 && p.y < 240 && p.x > 0 && p.x < 245) {
      clearmessage(); // erase the message
    }
    // backlight buttons
    if (p.y > 0 && p.y < 70 && p.x > 0 && p.x < 60) {
      if (page == 6) {
        blightdown();
      }
    }
    if (p.y > 0 && p.y < 70 && p.x > 260 && p.x < 320) {
      if (page == 6) {
        blightup();
      }
    }
    // sleep buttons
    
    if (p.y > 80 && p.y < 130 && p.x > 0 && p.x < 60) {
      if (page == 6) {
        sleepdec();
      }
    }
    if (p.y > 80 && p.y < 130 && p.x > 260 && p.x < 320) {
      if (page == 6) {
        sleepinc();
      }
    }
    /*
    // optional buttons
     if (p.y > 3 && p.y < 66 && p.x > 72 && p.x < 126) {
     if (page == 6) {
     option3down();
     }
     }
     if (p.y > 269 && p.y < 324 && p.x > 72 && p.x < 126) {
     if (page == 6) {
     option3up();
     }
     }
     */
  }
  if(currenttime - prevbatt > battcheck) {
    drawbatt();
    prevbatt = currenttime;

  }
}









//------------------------------------------------------------------------------------------------------------------------------
void yled(int xled) { // "flashes" the "yellow" LED
  for(i = xled ; i >= 0; i-=1) { 
    digitalWrite(greenled, LOW);
    digitalWrite(redled, HIGH);
    delay(1);
    digitalWrite(greenled, HIGH);
    digitalWrite(redled, LOW);
    delay(1);    
  }
  digitalWrite(greenled, LOW);
  if (greenflag == 1) {
    digitalWrite(redled, LOW);
    digitalWrite(greenled, HIGH);
  }
  if (redflag == 1) {
    digitalWrite(greenled, LOW);
    digitalWrite(redled, HIGH);
  }
}
void redraw() { // redraw the page
  if ((prevpage != 6) || (page !=7)) {
    clearcenter();
  }
  if (page == 0) {
    clearcenter();
    homescr();
  }
  if (page == 1) {
    menu1();
  }
  if (page == 2) {
    menu2();
  }
  if (page == 3) {
    menu3();
  }
  if (page == 4) {
    menu4();
  }
  if (page == 5) {
    menu5();
  }
  if (page == 6) {
    settingsscr();
  }
}
void clearcenter() { // the reason for so many small "boxes" is that it's faster than filling the whole thing
  tft.drawRect(0, 20, 150, 50, BLACK);
  tft.drawRect(170, 20, 150, 50, BLACK);
  tft.drawRect(0, 80, 150, 50, BLACK);
  tft.drawRect(170, 80, 150, 50, BLACK);
  tft.drawRect(0, 140, 150, 50, BLACK);
  tft.drawRect(170, 140, 150, 50, BLACK);
  /*
  tft.fillRect(5, 25, 145, 45, BLACK);
  tft.fillRect(170, 20, 150, 50, BLACK);
  tft.fillRect(0, 80, 150, 50, BLACK);
  tft.fillRect(170, 80, 150, 50, BLACK);
  tft.fillRect(0, 140, 150, 50, BLACK);
  tft.fillRect(170, 140, 150, 50, BLACK);
  */
  
  tft.fillRect(22, 37, 106, 16, BLACK);
  tft.fillRect(170, 20, 150, 50, BLACK);
  tft.fillRect(22, 97, 106, 16, BLACK);
  tft.fillRect(192, 97, 106, 16, BLACK);
  tft.fillRect(22, 157, 106, 16, BLACK);
  tft.fillRect(192, 157, 106, 16, BLACK);
  
}
void clearsettings() { // this is used to erase the extra drawings when exiting the settings page
  tft.fillRect(0, 20, 320, 110, BLACK);
  delay(500);
  clearmessage();
}
void homescr() {
  
  tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  tft.fillRect(0, 15, 320, 3, BLACK); // status bar
  drawhomeicon(); // draw the home icon
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.print("           Home");
        
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
    tft.setCursor(41, 37);
  tft.print("Backing");
  tft.setCursor(210, 37);
  tft.print("Passing");
  tft.setCursor(41, 97);
  //tft.print("Menu 3");
  tft.setCursor(210, 97);
  //tft.print("Menu 4");
  tft.setCursor(41, 157);
  tft.print("Cam Sel");
  tft.setCursor(200, 157);
  tft.print("Settings");
  

  tft.drawRect(0, 80, 150, 50, BLACK);  // Delete box lines around item 3
  tft.drawRect(170, 80, 150, 50, BLACK);  // Delete box line around item 4
  
  
}
void menu1() {              //BACKING

tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.print("         Backing");
  
  
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print(" Front");
  tft.setCursor(192, 37);
  tft.print("Back & Dn");
  tft.setCursor(22, 97);
  tft.print("  Truck <");
  tft.setCursor(192, 97);
  tft.print("> Truck");
  tft.setCursor(22, 157);
  tft.print("Trailer <");
  tft.setCursor(192, 157);
  tft.print("> Trailer");
}
void menu2() {              //PASSING
tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
tft.fillRect(0, 15, 320, 3, BLACK); // status bar
  
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.print("          Passing");
  
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  //tft.print("Menu 2 B1");
  tft.setCursor(192, 37);
  tft.print("Truck");
  tft.setCursor(22, 97);
  //tft.print("Menu 2 B3");
  tft.setCursor(192, 97);
  tft.print("Trailer");
  tft.setCursor(22, 157);
  //tft.print("Menu 2 B5");
  tft.setCursor(192, 157);
  tft.print("Sideways");
  
  tft.drawRect(0, 20, 150, 50, BLACK);
  //tft.drawRect(170, 20, 150, 50, BLACK);
  tft.drawRect(0, 80, 150, 50, BLACK);
  //tft.drawRect(170, 80, 150, 50, BLACK);
  tft.drawRect(0, 140, 150, 50, BLACK);
  //tft.drawRect(170, 140, 150, 50, BLACK);
}
void menu3() {                            //Trailer Cam slection Options

tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.print("      Trailer Cameras ");
  
  
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Driver");
  tft.setCursor(192, 37);
  tft.print("Passenger");
  tft.setCursor(22, 97);
  //tft.print("Menu 3 B3");
  tft.setCursor(192, 97);
  //tft.print("Menu 3 B4");
  tft.setCursor(22, 157);
  tft.print("Sideways");
  tft.setCursor(192, 157);
  tft.print("Back & Dn");
  
  //tft.drawRect(0, 20, 150, 50, BLACK);
  //tft.drawRect(170, 20, 150, 50, BLACK);
  tft.drawRect(0, 80, 150, 50, BLACK);
  tft.drawRect(170, 80, 150, 50, BLACK);
  //tft.drawRect(0, 140, 150, 50, BLACK);
  //tft.drawRect(170, 140, 150, 50, BLACK);
  
}
void menu4() {                              // TRUCK cam select OPTIONS
tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.print("       Truck Cameras");
  
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  tft.print("Front");
  tft.setCursor(192, 37);
 // tft.print("Menu 4 B2");
  tft.setCursor(22, 97);
  tft.print("Frnt Drv");
  tft.setCursor(192, 97);
  tft.print("Frnt Pas");
  tft.setCursor(22, 157);
  tft.print("Back Drv");
  tft.setCursor(192, 157);
  tft.print("Back Pass");
  
//tft.drawRect(0, 20, 150, 50, BLACK);
  tft.drawRect(170, 20, 150, 50, BLACK);
  //tft.drawRect(0, 80, 150, 50, BLACK);
  //tft.drawRect(170, 80, 150, 50, BLACK);
  //tft.drawRect(0, 140, 150, 50, BLACK);
  //tft.drawRect(170, 140, 150, 50, BLACK);
  
}
void menu5() {  // Camera Select

tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.print("      Camera Selection");
  
  
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  boxes();
  tft.setCursor(22, 37);
  //tft.print("Truck");
  tft.setCursor(192, 37);
  tft.print("Truck");
  tft.setCursor(22, 97);
  //tft.print("Menu 5 B3");
  tft.setCursor(192, 97);
  tft.print("Trailer");
  tft.setCursor(22, 157);
  //tft.print("Menu 5 B5");
  tft.setCursor(192, 157);
 // tft.print("Menu 5 B6");
  
 tft.drawRect(0, 20, 150, 50, BLACK);
  //tft.drawRect(170, 20, 150, 50, BLACK);
  tft.drawRect(0, 80, 150, 50, BLACK);
  //tft.drawRect(170, 80, 150, 50, BLACK);
  tft.drawRect(0, 140, 150, 50, BLACK);
  tft.drawRect(170, 140, 150, 50, BLACK);
  
}
void settingsscr() {
  
  tft.fillRect(0, 0, 320, 15, JJCOLOR); // status bar
  tft.fillRect(0, 15, 320, 3, BLACK); // status bar
  //drawhomeicon(); // draw the home icon
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.print("         Settings");
  
  // backlight level
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.fillRect(0, 20, 60, 50, RED);
  tft.drawRect(0, 20, 60, 50, WHITE);
  tft.drawRect(80, 20, 160, 50, JJCOLOR);
  tft.fillRect(260, 20, 60, 50, GREEN);
  tft.drawRect(260, 20, 60, 50, WHITE);
  tft.setCursor(22, 33);
  tft.print("-");
  tft.setCursor(282, 33);
  tft.print("+");
  tft.setTextSize(1);
  tft.setCursor(120, 31);
  tft.print("Backlight Level");
  tft.drawRect(110, 48, 100, 10, WHITE);
  blbar();
  // sleep time
  tft.setTextSize(3);
  tft.fillRect(0, 80, 60, 50, RED);
  tft.drawRect(0, 80, 60, 50, WHITE);
  tft.drawRect(80, 80, 160, 50, JJCOLOR);
  tft.fillRect(260, 80, 60, 50, GREEN);
  tft.drawRect(260, 80, 60, 50, WHITE);
  tft.setCursor(22, 93);
  tft.print("-");
  tft.setCursor(282, 93);
  tft.print("+");
  tft.setTextSize(1);
  tft.setCursor(130, 91);
  tft.print("Sleep Time");
  showsleep();
  //?? uncomment this if you want a third adjustable option
  /*
  tft.fillRect(0, 140, 60, 50, RED);
   tft.drawRect(0, 140, 60, 50, WHITE);
   tft.drawRect(80, 140, 160, 50, JJCOLOR);
   tft.fillRect(260, 140, 60, 50, GREEN);
   tft.drawRect(260, 140, 60, 50, WHITE);
   tft.print(22, 153, "-", WHITE, 3);
   tft.print(130, 151, "Thing #3", WHITE);
   tft.print(282, 153, "+", WHITE, 3);
   tft.drawRect(110, 168, 100, 10, WHITE);
   
  battv = readVcc(); // read the voltage
  itoa (battv, voltage, 10);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.setCursor(12, 213);
  tft.print(voltage);
  tft.setCursor(60, 213);
  tft.print("mV");
  */
  /*
  battpercent = (battv / 5000) * 100, 2;
  itoa (battpercent, battpercenttxt, 10);
  tft.print(102, 213, battpercenttxt, YELLOW, 2);
  */
}
void sleepinc() { // sleep increese adjustment
  if (sleeptime == 14400000) {
    sleepnever = 1;
    esleep = 12;
    sleeptime = 11111111;
    showsleep();
  }
  if (sleeptime == 3600000) {
    sleeptime = 14400000;
    esleep = 11;
    showsleep();
  }
  if (sleeptime == 1800000) {
    sleeptime = 3600000;
    esleep = 10;
    showsleep();
  }
  if (sleeptime == 1200000) {
    sleeptime = 1800000;
    esleep = 9;
    showsleep();
  }
  if (sleeptime == 600000) {
    sleeptime = 1200000;
    esleep = 8;
    showsleep();
  }
  if (sleeptime == 300000) {
    sleeptime = 600000;
    esleep = 7;
    showsleep();
  }
  if (sleeptime == 120000) {
    sleeptime = 300000;
    esleep = 6;
    showsleep();
  }
  if (sleeptime == 60000) {
    sleeptime = 120000;
    esleep = 5;
    showsleep();
  }
  if (sleeptime == 30000) {
    sleeptime = 60000;
    esleep = 4;
    showsleep();
  }
  if (sleeptime == 20000) {
    sleeptime = 30000;
    esleep = 3;
    showsleep();
  }
  if (sleeptime == 10000) {
    sleeptime = 20000;
    esleep = 2;
    showsleep();
  }
  delay(350);
}
void sleepdec() { // sleep decreese adjustment
  if (sleeptime == 20000) {
    sleeptime = 10000;
    esleep = 1;
    showsleep();
  }
  if (sleeptime == 30000) {
    sleeptime = 20000;
    esleep = 2;
    showsleep();
  }
  if (sleeptime == 60000) {
    sleeptime = 30000;
    esleep = 3;
    showsleep();
  }
  if (sleeptime == 120000) {
    sleeptime = 60000;
    esleep = 4;
    showsleep();
  }
  if (sleeptime == 300000) {
    sleeptime = 120000;
    esleep = 5;
    showsleep();
  }
  if (sleeptime == 600000) {
    sleeptime = 300000;
    esleep = 6;
    showsleep();
  }
  if (sleeptime == 1200000) {
    sleeptime = 600000;
    esleep = 7;
    showsleep();
  }
  if (sleeptime == 1800000) {
    sleeptime = 1200000;
    esleep = 8;
    showsleep();
  }
  if (sleeptime == 3600000) {
    sleeptime = 1800000;
    esleep = 9;
    showsleep();
  }
  if (sleeptime == 14400000) {
    sleeptime = 3600000;
    esleep = 10;
    showsleep();
  }
  if (sleepnever == 1) {
    sleeptime = 14400000;
    sleepnever = 0;
    esleep = 11;
    showsleep();
  }
  delay(350);
}
void showsleep() { // shows the sleep time on the settings page
  tft.fillRect(110, 108, 80, 10, BLACK);
  tft.setTextSize(1);
  Serial.println("showsleep running");
  tft.setTextColor(WHITE);
  if (sleeptime == 10000) {
    tft.setCursor(130, 108);
    tft.print("10 Seconds");
  }
  if (sleeptime == 20000) {
    tft.setCursor(130, 108);
    tft.print("20 Seconds");
  }
  if (sleeptime == 30000) {
    tft.setCursor(130, 108);
    tft.print("30 Seconds");
  }
  if (sleeptime == 60000) {
    tft.setCursor(136, 108);
    tft.print("1 Minute");
  }
  if (sleeptime == 120000) {
    tft.setCursor(133, 108);
    tft.print("2 Minutes");
  }
  if (sleeptime == 300000) {
    tft.setCursor(133, 108);
    tft.print("5 Minutes");
  }
  if (sleeptime == 600000) {
    tft.setCursor(130, 108);
    tft.print("10 Minutes");
  }
  if (sleeptime == 1200000) {
    tft.setCursor(130, 108);
    tft.print("20 Minutes");
  }
  if (sleeptime == 1800000) {
    tft.setCursor(130, 108);
    tft.print("30 Minutes");
  }
  if (sleeptime == 3600000) {
    tft.setCursor(142, 108);
    tft.print("1 Hour");
  }
  if (sleeptime == 14400000) {
    tft.setCursor(139, 108);
    tft.print("4 Hours");
  }
  if (sleepnever == 1) {
    tft.setCursor(133, 108);
    tft.print("Always On");
  }
  
  
  
  
}
void option3down() { // adjust option 3 down in the settings screen
}
void option3up() { // adjust option 3 up in the settings screen
}
//custom defined actions - this is where you put your button functions
void m1b1action() {  //Front
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, HIGH);;
}
void m1b2action() {   //Back and Down
  
  digitalWrite(out1, HIGH);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}

void m1b3action() {    //Truck Driver
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, HIGH);
  
}
void m1b4action() {   // Truck Passenger
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, LOW);
  digitalWrite(out4, HIGH);
}
void m1b5action() {   //Trailer Driver
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, LOW);
}
void m1b6action() {    //Trailer Passenger
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, HIGH);
}

void m2b1action() {   
  

}
void m2b2action() {   // Passing Truck Passenger
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, LOW);
  digitalWrite(out4, HIGH);
}

void m2b3action() {  
  
  
}
void m2b4action() {   // Passing Trailer Passenger
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, HIGH);
}
void m2b5action() {
  
 
}
void m2b6action() {   // Passing Trailer End Sideways
  
  digitalWrite(out1, HIGH);
  digitalWrite(out2, LOW);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, LOW);
}



void m3b1action() {   // Select Trailer Cams
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, LOW);
}
void m3b2action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, HIGH);
}
void m3b3action() {
  
  
}
void m3b4action() {
  
 
}
void m3b5action() {
  
  digitalWrite(out1, HIGH);
  digitalWrite(out2, LOW);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, LOW);
}
void m3b6action() {
  
  digitalWrite(out1, HIGH);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}


void m4b1action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, HIGH);
}
void m4b2action() {
  
 
}
void m4b3action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, LOW);
}
void m4b4action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}
void m4b5action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, HIGH);
  digitalWrite(out4, HIGH);
}
void m4b6action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, HIGH);
  digitalWrite(out3, LOW);
  digitalWrite(out4, HIGH);
}
void m5b1action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}
void m5b2action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}
void m5b3action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}
void m5b4action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}
void m5b5action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}
void m5b6action() {
  
  digitalWrite(out1, LOW);
  digitalWrite(out2, LOW);
  digitalWrite(out3, LOW);
  digitalWrite(out4, LOW);
}




void blightup() { // increase the backlight brightness
  blv = blv + 5;
  if (blv >= 255) {
    blv = 255;
  }
  analogWrite(backlight, blv);
  blbar();
}
void blightdown() { // decrease the backlight brightness
  blv = blv - 5;
  if (blv <= 5) {
    blv = 5;
  }
  analogWrite(backlight, blv);
  blbar();
}
void blbar() { // this function fills the yellow bar in the backlight brightness adjustment
  if (blv < barv) {
    tft.fillRect(111, 49, 98, 8, BLACK);
  }
  backlightbox = map(blv, 1, 255, 0, 98);
  tft.fillRect(111, 49, backlightbox, 8, YELLOW);
  barv = blv;
  delay(25);
}
void ant() {
  tft.fillRect((antpos + 5), 4, 1, 6, WHITE); // draws the "antenna" for the signal indicator
}
void boxes() { // redraw the button outline boxes
  tft.drawRect(0, 20, 150, 50, JJCOLOR);
  tft.drawRect(170, 20, 150, 50, JJCOLOR);
  tft.drawRect(0, 80, 150, 50, JJCOLOR);
  tft.drawRect(170, 80, 150, 50, JJCOLOR);
  tft.drawRect(0, 140, 150, 50, JJCOLOR);
  tft.drawRect(170, 140, 150, 50, JJCOLOR);
}
void signal() { // draws a whit 'signal indicator'
  tft.drawLine((antpos + 4), 4, (antpos + 4), 5, WHITE);
  tft.drawPixel((antpos + 3), 2, WHITE);
  tft.drawPixel((antpos + 3), 7, WHITE);
  tft.drawPixel((antpos + 2), 0, WHITE);
  tft.drawLine((antpos + 2), 3, (antpos + 2), 6, WHITE);
  tft.drawPixel((antpos + 2), 9, WHITE);
  tft.drawPixel((antpos + 1), 1, WHITE);
  tft.drawPixel((antpos + 1), 8, WHITE);
  tft.drawLine(antpos, 2, antpos, 7, WHITE);
  tft.drawLine((antpos + 6), 4, (antpos + 6), 5, WHITE);
  tft.drawPixel((antpos + 7), 2, WHITE);
  tft.drawPixel((antpos + 7), 7, WHITE);
  tft.drawPixel((antpos + 8), 0, WHITE);
  tft.drawLine((antpos + 8), 3, (antpos + 8), 6, WHITE);
  tft.drawPixel((antpos + 8), 9, WHITE);
  tft.drawPixel((antpos + 9), 1, WHITE);
  tft.drawPixel((antpos + 9), 8, WHITE);
  tft.drawLine((antpos + 10), 2, (antpos + 10), 7, WHITE);
}
void signalact() { // draws a red'signal indicator'
  tft.drawLine((antpos + 4), 4, (antpos + 4), 5, RED);
  tft.drawPixel((antpos + 3), 2, RED);
  tft.drawPixel((antpos + 3), 7, RED);
  tft.drawPixel((antpos + 2), 0, RED);
  tft.drawLine((antpos + 2), 3, (antpos + 2), 6, RED);
  tft.drawPixel((antpos + 2), 9, RED);
  tft.drawPixel((antpos + 1), 1, RED);
  tft.drawPixel((antpos + 1), 8, RED);
  tft.drawLine(antpos, 2, antpos, 7, RED);
  tft.drawLine((antpos + 6), 4, (antpos + 6), 5, RED);
  tft.drawPixel((antpos + 7), 2, RED);
  tft.drawPixel((antpos + 7), 7, RED);
  tft.drawPixel((antpos + 8), 0, RED);
  tft.drawLine((antpos + 8), 3, (antpos + 8), 6, RED);
  tft.drawPixel((antpos + 8), 9, RED);
  tft.drawPixel((antpos + 9), 1, RED);
  tft.drawPixel((antpos + 9), 8, RED);
  tft.drawLine((antpos + 10), 2, (antpos + 10), 7, RED);
}
void drawhomeicon() { // draws a white home icon
  tft.drawLine(280, 219, 299, 200, WHITE);
  tft.drawLine(300, 200, 304, 204, WHITE);
  tft.drawLine(304, 203, 304, 200, WHITE);
  tft.drawLine(305, 200, 307, 200, WHITE);
  tft.drawLine(308, 200, 308, 208, WHITE);
  tft.drawLine(309, 209, 319, 219, WHITE);
  tft.drawLine(281, 219, 283, 219, WHITE);
  tft.drawLine(316, 219, 318, 219, WHITE);
  tft.drawRect(284, 219, 32, 21, WHITE);
  tft.drawRect(295, 225, 10, 15, WHITE);
}
void drawhomeiconred() { // draws a red home icon
  tft.drawLine(280, 219, 299, 200, RED);
  tft.drawLine(300, 200, 304, 204, RED);
  tft.drawLine(304, 203, 304, 200, RED);
  tft.drawLine(305, 200, 307, 200, RED);
  tft.drawLine(308, 200, 308, 208, RED);
  tft.drawLine(309, 209, 319, 219, RED);
  tft.drawLine(281, 219, 283, 219, RED);
  tft.drawLine(316, 219, 318, 219, RED);
  tft.drawRect(284, 219, 32, 21, RED);
  tft.drawRect(295, 225, 10, 15, RED);
}
void clearmessage() {
  tft.fillRect(12, 213, 226, 16, BLACK); // black out the inside of the message box
}
void drawbatt() {
  battv = readVcc(); // read the voltage
  if (battv < battold) { // if the voltage goes down, erase the inside of the battery
    tft.fillRect(298, 2, 18, 6, BLACK);
  }
  battfill = map(battv, 3000, 4850, 2, 18); // map the battery voltage 3000 nis the low, 4150 is the high
  if (battfill > 7) { // if the battfill value is between 8 and 18, fill with green
    tft.fillRect(298, 2, battfill, 6, GREEN);
  }
  else { // if the battfill value is below 8, fill with red
    tft.fillRect(298, 2, battfill, 6, RED);
  }
  battold = battv; // this helps determine if redrawing the battfill area is necessary
}