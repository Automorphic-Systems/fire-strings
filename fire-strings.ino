#include <SimpleTimer.h>

#define SAMPLE_SIZE 50
#define DELAY 10
  #define THRESHOLD 1.0
#define MAX_BOUND 10000
#define SOL_DURATION 400
#define SOL_REFRACTORY 1500
#define CHANNELS 2
#define DEBUG true

int LED_Pin = 13;
int VIBR_PIN[CHANNELS] = { A0, A1 };
int SOL_PIN[CHANNELS] = { 10, 11 };

float measurements[CHANNELS][SAMPLE_SIZE];
float bufferResult[CHANNELS];
int state[CHANNELS] = { 0, 0 };
int iter[CHANNELS] = { 0, 0 };
bool readAnalogVibration = true;

void setup(){
  pinMode(LED_Pin, OUTPUT);
  pinMode(SOL_PIN[0], OUTPUT);
  pinMode(SOL_PIN[1], OUTPUT);
  Serial.begin(9600); //init serial 9600

  reset_buffer(0);
  reset_buffer(1);
}

void reset_buffer(int channel) {
  Serial.println("Resetting buffer....");  
  for (int j=0; j<SAMPLE_SIZE; j++) 
     measurements[channel][j]=0;
    
  bufferResult[channel] = 0;
}

void trigger_solenoid(int channel) {
    digitalWrite(SOL_PIN[channel], HIGH);
    Serial.println("Actuation.....");
    delay(SOL_DURATION);    
    digitalWrite(SOL_PIN[channel], LOW);    
    Serial.println("Refractory.....");
    delay(SOL_REFRACTORY); 
}

void trigger_solenoids() {
    for(int i=0; i<CHANNELS; i++) {
      if (state[i]) {
        digitalWrite(SOL_PIN[i], HIGH); 
        reset_buffer(i);            
      }
    }
    
    Serial.println("Actuation.....");
    delay(SOL_DURATION);    

    for(int i=0; i<CHANNELS; i++) {
      digitalWrite(SOL_PIN[i], LOW);        
    }
    Serial.println("Refractory.....");
    delay(SOL_REFRACTORY); 
}

void loop(){  
    if (readAnalogVibration) {
      
      for (int k=0; k<CHANNELS; k++) {
        int piezoVal = analogRead(VIBR_PIN[k]);

        //TODO: modify moving average function
        piezo_moving_average(k, piezoVal / 1023.0 * 25.0);

        //TODO: if a channel crosses a threshold, set state to 1
        //if timer is active and > 1 sec, stop timer and trigger solenoids
        //if timer is inactive, start timer
        if (bufferResult[k] > THRESHOLD){
          digitalWrite(LED_Pin, HIGH);
          trigger_solenoid(k);
          reset_buffer(k);
        } else {
          digitalWrite(LED_Pin, LOW);
        }
      }

      // output to serial monitor
      if (DEBUG) { 
        Serial.print(bufferResult[0]);
        Serial.print(",");
        Serial.println(bufferResult[1]);
      }
    }     
}


// implement circular buffer for moving average
void piezo_moving_average(int channel, float measurement) {
  int curPos = state[channel];                
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


