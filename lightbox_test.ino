#include "constants.h"
#include "LedControl.h"
#include <Bounce.h>

#define PIN_DIN 3
#define PIN_CLK 5
#define PIN_LOAD 6
#define PIN_BUTTON 12

#define MAX_INTENSITY 15

#define PIC_LEN 3
#define SELECTION_TIMEOUT 2000
#define SELECTION_FLICKER 250

#include "symbols.h"
#include "picture.h"

LedControl lc = LedControl(PIN_DIN, PIN_CLK, PIN_LOAD, 1);
Bounce bouncer = Bounce( PIN_BUTTON,5 ); 

int current_picture = 0;
Picture* p = 0;
Picture* pictures[PIC_LEN] = {new H_Lines(lc), new H_Lines(lc), new H_Lines(lc)};

long msecs = millis();

void setup() {
  pinMode(PIN_BUTTON, INPUT);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  
  lc.shutdown(0,false);
  lc.setScanLimit(0, MAX_ROW);
  lc.setIntensity(0, MAX_INTENSITY);
  lc.clearDisplay(0);
  
  // startupCycle();

}

void loop() {  
  static boolean display_menu = true;
  static long selection_timer = -1;
  
  long msecs_old = msecs;
  msecs = millis();
  long delta = msecs - msecs_old;
  
  bouncer.update();
  
  if (bouncer.fallingEdge()) {
    digitalWrite(13, LOW);
    
    // Cycle picture
    current_picture = (++current_picture) % PIC_LEN;
    
    Serial.println("Current picture: " + String(current_picture));
    
    lc.clearDisplay(0);
    selection_timer = SELECTION_TIMEOUT;
    display_menu = true;
  } else if (bouncer.risingEdge()) {
    digitalWrite(13, HIGH);
  } else if (bouncer.duration() == 3000) {
//    testCycle();
//    lc.clearDisplay(0);
  }
  
  if (display_menu && selection_timer <= 0) {
    display_menu = false;
    lc.clearDisplay(0);
    Serial.println("Hide menu");
    if (p != 0) {
      p->leave();
    }
    p = pictures[current_picture];
    p->enter();
  }
  
  if (selection_timer <= 0) {
    if (delta >= p->getDelay()) {
      p->loop();
    }
  } else {
    // Display selection "menu"
    displayMenu(current_picture);
    
    selection_timer -= delta;
  }
}

void startupCycle() {
  //stack();

  lc.clearDisplay(0);
  
  const prog_uint8_t* ver[13] = {L, i, g, h, t, b, o, x, spc, v, one, dot, zero};
  scrollSymbols(lc, ver, 13, 150);

  testCycle();
  
  for (int i=0; i<PIC_LEN; ++i) {
    pictures[i]->setup();
    Serial.println(String("") + "Initialized " + (i+1) + " of " + PIC_LEN + ".");
  }
  
  lc.clearDisplay(0);
}

void testCycle() {
  lightAll(); 
  delay(1000);
  lightEvenRows();
  delay(1000);
  lightOddRows();
  delay(1000);
  lightEvenCols();
  delay(1000);
  lightOddCols();
}

void displayMenu(int selectedItem) {
  static boolean selection_mode = false;
  static long old_millis = millis();
  static int millis_since_last_change = SELECTION_FLICKER;
  
  long delta = millis() - old_millis;
  old_millis = millis();
  
  millis_since_last_change -= delta;
  if (millis_since_last_change <= 0) {
    millis_since_last_change = SELECTION_FLICKER;
    selection_mode = !selection_mode;
  }
    
  for (int i=0; i<NUM_OF_LEDS; ++i) {
    int row = i / (MAX_COL+1);
    int col = i % (MAX_COL+1);
    
    
    boolean light_mode;
    if (i == selectedItem) {
      light_mode = selection_mode;
    } else {
      light_mode = (i < PIC_LEN);
    }
    lc.setLed(0, row, col, light_mode);
  }
}

void lightAll() {
  for (int row=0; row<=MAX_ROW; ++row) {
    for (int col=0; col<=MAX_COL; ++col) {
      lc.setLed(0, row, col, true);
    }
  }
}

void lightEvenRows() {
  lc.clearDisplay(0);
  
  for (int row=0;row<=MAX_ROW;row+=2) {
    for (int col=0; col<=MAX_COL; ++col) {
      lc.setLed(0, row, col, true);
    }
  }
}


void lightOddRows() {
  lc.clearDisplay(0);
  
  for (int row=1;row<=MAX_ROW;row+=2) {
    for (int col=0; col<=MAX_COL; ++col) {
      lc.setLed(0, row, col, true);
    }
  }
}


void lightEvenCols() {
  lc.clearDisplay(0);
  
  for (int row=0;row<=MAX_ROW;row++) {
    for (int col=0; col<=MAX_COL; col+=2) {
      lc.setLed(0, row, col, true);
    }
  }
}


void lightOddCols() {
  lc.clearDisplay(0);
  
  for (int row=0;row<=MAX_ROW;row++) {
    for (int col=1; col<=MAX_COL; col+=2) {
      lc.setLed(0, row, col, true);
    }
  }
}

void stack() {
  for (int i=0; i<NUM_OF_LEDS; ++i) {
    
    for (int j=0; j<NUM_OF_LEDS-i; ++j) {
      lineSnake(j, true);
      if (j>0) lineSnake(j-1, false);
      
      delay(100);
    }
    
    
  }
}

void lineSnake(int c_step, boolean light_mode) {
  int row = c_step / (MAX_COL+1);
  
  int col = c_step % (MAX_COL+1);
  if (row%2 != 0) {
    col = MAX_COL - col;
  }
  

  
  lc.setLed(0, row, col, light_mode);
}

void breathe() {
  for (int i=0; i<16; ++i) {
    lc.setIntensity(0,i);
    delay(100);
  }
  for (int i=15; i>=0; --i) {
    lc.setIntensity(0,i);
    delay(100);
  }
}
