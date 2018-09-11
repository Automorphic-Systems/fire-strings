#include <SimpleTimer.h>

#define SAMPLE_SIZE 50
#define THRESHOLD 1.0
#define CHANNELS 4

#define REFRACT_COEFFICIENT 4.0
#define DEBUG true
#define PATTERN_MODE false
#define AUTO_MODE false

// Pins
int VIBR_PIN[CHANNELS] = { A0, A1, A2, A3 };
int SOL_PIN[CHANNELS] = { 10, 11, 8, 9 };
int CTRL_SENSITIVITY = A6;
int CTRL_DURATION = A7;
int PATTERN_PIN = 4;
int AUTO_PIN = 3;
int LED_PIN = 13;

float SENSITIVITY_MAX = 250.0;
float DURATION_MAX = 500.0;
float DURATION_MIN = 50.0;

// Real-time data structures
float measurements[CHANNELS][SAMPLE_SIZE];
float bufferResult[CHANNELS];
int state[CHANNELS] = { 0, 0, 0, 0 };
int iter[CHANNELS] = { 0, 0, 0, 0 };
bool readAnalogVibration = true;
bool timerActive = false;
SimpleTimer solenoidTimer;
int timerId;
int senseVal;
int durationVal;

void setup(){
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600); //init serial 9600
  
  // clear buffers and set pinmode
  for (int i=0; i<CHANNELS; i++) {
    reset_buffer(i);
    pinMode(SOL_PIN[i], OUTPUT);
  }

  pinMode(3, INPUT);
  pinMode(4, INPUT);
  
  timerId = solenoidTimer.setInterval(1000, trigger_solenoids);
}

void reset_buffer(int channel) {
  for (int j=0; j<SAMPLE_SIZE; j++) 
     measurements[channel][j]=0;
    
  bufferResult[channel] = 0;
}

void trigger_solenoids() {
    int solCount = 0;

    for(int i=0; i<CHANNELS; i++) {
      if (state[i]==1) {
//        Serial.println("Solenoid triggered..");
//        Serial.println(i);
        digitalWrite(SOL_PIN[i], HIGH); 
        solCount++;
      }
    }
    
    Serial.println("Actuation.....");
    if (solCount > 0) { delay(durationVal); }

    Serial.println("Resetting buffer....");  
    for(int i=0; i<CHANNELS; i++) {      
      digitalWrite(SOL_PIN[i], LOW);        
      reset_buffer(i); 
      state[i]=0;           
    }

    Serial.println("Refractory.....");
    if (solCount > 0) { delay(durationVal * REFRACT_COEFFICIENT); } 
}

void loop(){  
    int isAuto = digitalRead(AUTO_PIN);
    int isPattern = digitalRead(PATTERN_PIN);
    Serial.print(isAuto);
    Serial.print(",");
    Serial.println(isPattern);

    
    senseVal = SENSITIVITY_MAX * analogRead(CTRL_SENSITIVITY) / 1023;
    durationVal = DURATION_MIN + ((DURATION_MAX - DURATION_MIN) * analogRead(CTRL_DURATION)) / 1023;
    //Serial.print(senseVal);
    //Serial.print(",");
    //Serial.println(durationVal);
    
    if (readAnalogVibration) {
      //TODO: make the timer invoke a callback function starting from when the first sensor passes threshold value
      solenoidTimer.run();
            
      for (int k=0; k<CHANNELS; k++) {
        int piezoVal = analogRead(VIBR_PIN[k]);
  
        //TODO: modify moving average function
        piezo_moving_average(k, piezoVal / 1023.0 * senseVal);

        if (bufferResult[k] > THRESHOLD){
          state[k] = 1;
        } else {
          digitalWrite(LED_PIN, LOW);
        }
      }

      // output to serial monitor
      if (DEBUG) {         
        Serial.print(bufferResult[0]);
        Serial.print(",");
        Serial.print(bufferResult[1]);
        Serial.print(",");
        Serial.print(bufferResult[2]);
        Serial.print(",");
        Serial.println(bufferResult[3]);
      }
    }     
}


// implement circular buffer for moving average
void piezo_moving_average(int channel, float measurement) {
  int curPos = iter[channel];                
  float newVal = measurement / (float)SAMPLE_SIZE;  
  float oldVal = measurements[channel][curPos];
  
  //Serial.println(i);
  //Serial.println(oldVal);
  bufferResult[channel] = bufferResult[channel] + newVal - oldVal; 
  measurements[channel][curPos] = newVal;
  
  //TODO: Improve smoothing function - add an upper bound, 
  //reset the function if the buffer result drops below a certain lower bound
  
  iter[channel]++;
  if (iter[channel]>=SAMPLE_SIZE) { iter[channel]=0; }

}


