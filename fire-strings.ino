#include <SimpleTimer.h>

#define SAMPLE_SIZE 50

#define THRESHOLD 1.0
#define SOL_DURATION 400
#define SOL_REFRACTORY 1500
#define DEBUG true
#define PATTERN_MODE false
#define SENSITIVITY 100.0
#define CHANNELS 4

int LED_PIN = 13;
int VIBR_PIN[CHANNELS] = { A0, A1, A2, A3 };
int SOL_PIN[CHANNELS] = { 10, 11, 8, 9 };

float measurements[CHANNELS][SAMPLE_SIZE];
float bufferResult[CHANNELS];
int state[CHANNELS] = { 0, 0, 0, 0 };
int iter[CHANNELS] = { 0, 0, 0, 0 };
bool readAnalogVibration = true;
bool timerActive = false;
SimpleTimer solenoidTimer;
int timerId;

void setup(){
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600); //init serial 9600

  // clear buffers and set pinmode
  for (int i=0; i<CHANNELS; i++) {
    reset_buffer(i);
    pinMode(SOL_PIN[i], OUTPUT);
  }

  timerId = solenoidTimer.setInterval(1000, trigger_solenoids);
}

void reset_buffer(int channel) {
  Serial.println("Resetting buffer....");  
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
    
   // Serial.println("Actuation.....");
    if (solCount > 0) { delay(SOL_DURATION); }

    for(int i=0; i<CHANNELS; i++) {
      digitalWrite(SOL_PIN[i], LOW);        
      reset_buffer(i); 
      state[i]=0;           
    }

   // Serial.println("Refractory.....");
    if (solCount > 0) { delay(SOL_REFRACTORY); } 
}

void loop(){  
    if (readAnalogVibration) {
      //TODO: make the timer invoke a callback function starting from when the first sensor passes threshold value
      solenoidTimer.run();
            
      for (int k=0; k<CHANNELS; k++) {
        int piezoVal = analogRead(VIBR_PIN[k]);
  
        //TODO: modify moving average function
        piezo_moving_average(k, piezoVal / 1023.0 * SENSITIVITY);

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


