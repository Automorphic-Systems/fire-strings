#include <SimpleTimer.h>

#define SAMPLE_SIZE 50
#define THRESHOLD 1.0
#define MAX_VALUE 3.0
#define CHANNELS 4

#define REFRACT_COEFFICIENT 4.0
#define PATTERN_MODE false
#define AUTO_MODE false

// Pins
int VIBR_PIN[CHANNELS] = { A0, A1, A2, A3 };
int SOL_PIN[CHANNELS] = { 10, 11, 8, 9 };
int CTRL_SENSITIVITY = A6;
int CTRL_DURATION = A7;
int PATTERN_PIN = 7; // switch
int AUTO_PIN = 6; // button
int LED_PIN = 13;

float SENSITIVITY_MAX = 250.0;
float DURATION_MAX = 500.0;
float DURATION_MIN = 50.0;

// state
float measurements[CHANNELS][SAMPLE_SIZE];
float bufferResult[CHANNELS];
int state[CHANNELS] = { 0, 0, 0, 0 };
int iter[CHANNELS] = { 0, 0, 0, 0 };
bool readAnalogVibration = true;
bool timerActive = false;
int senseVal;
int durationVal;

// timers
SimpleTimer solenoidTimer;
SimpleTimer solenoidSerTimer;
int timerId;
int timerSerId;

// debugging
bool debugBtns = true;
bool debugSensors = false;

void setup(){
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  
  // clear buffers and set pinmodes for solenoids
  for (int i=0; i<CHANNELS; i++) {
    reset_buffer(i);
    pinMode(SOL_PIN[i], OUTPUT);
  }

  // set pinmodes for button input
  pinMode(PATTERN_PIN, INPUT);
  pinMode(AUTO_PIN, INPUT);
  for (int i=2; i<6; i++) {
    pinMode(i, INPUT);  
    digitalWrite(i, HIGH);
  }

  // set timers
  timerId = solenoidTimer.setInterval(1000, trigger_solenoids);
  timerSerId = solenoidSerTimer.setInterval(250, trigger_solenoids_serial);

  // set random seed from some pin input
  randomSeed(analogRead(CTRL_SENSITIVITY));
}

// clears the buffer of sensor data
void reset_buffer(int channel) {
  for (int j=0; j<SAMPLE_SIZE; j++) 
     measurements[channel][j]=0;
    
  bufferResult[channel] = 0;
}

// actuates all of the solenoids whose state is set to 1
// refractory period exists to restrict gas flow and limit re-actuation
void trigger_solenoids() {
    int solCount = 0;

    for(int i=0; i<CHANNELS; i++) {
      if (state[i]==1) {
        digitalWrite(SOL_PIN[i], HIGH); 
        solCount++;
      }
    }
    
    if (solCount > 0) { 
      Serial.println("Actuation....."); 
      delay(durationVal); 
    }

    Serial.println("Resetting buffer....");  
    for(int i=0; i<CHANNELS; i++) {      
      digitalWrite(SOL_PIN[i], LOW);        
      reset_buffer(i); 
      state[i]=0;           
    }
 
    if (solCount > 0) {
      Serial.println("Refractory.....");
      delay(durationVal * REFRACT_COEFFICIENT); 
    }     
}

// actuates all of the solenoids whose state is set to 1 but in sequence
// refractory period exists to restrict gas flow and limit re-actuation
void trigger_solenoids_serial() {
    int solCount = 0;
    
    Serial.println("Serial actuation....."); 
    for(int i=0; i<CHANNELS; i++) {
      if (state[i]==1) { 
        digitalWrite(SOL_PIN[i], HIGH); 
        delay(durationVal);  
        digitalWrite(SOL_PIN[i], LOW);         
        reset_buffer(i); 
        state[i]=0;   
        solCount++;
      }
    }      
    
    if (solCount > 0) {
      Serial.println("Refractory.....");
      delay(durationVal * REFRACT_COEFFICIENT); 
    } 
}

void loop(){  
    int isAuto = digitalRead(AUTO_PIN); // Auto or manual actuation
    int isPattern = digitalRead(PATTERN_PIN); // If auto actuation, pattern or sensor

    senseVal = SENSITIVITY_MAX * analogRead(CTRL_SENSITIVITY) / 1023;
    durationVal = DURATION_MIN + ((DURATION_MAX - DURATION_MIN) * analogRead(CTRL_DURATION)) / 1023;

    if (debugBtns) {
      Serial.print(isAuto); 
      Serial.print(",");
      Serial.print(isPattern);
      Serial.print(",");
      Serial.print(durationVal);
      Serial.print(",");
      Serial.println(senseVal);
    }

    // if manual, read input from buttons
    if (!isAuto) {
        solenoidSerTimer.run();
          
        if (debugBtns) {
          Serial.print(digitalRead(2)); // Valve 1
          Serial.print(",");
          Serial.print(digitalRead(3)); // Valve 2
          Serial.print(",");
          Serial.print(digitalRead(4)); // Valve 3
          Serial.print(",");
          Serial.println(digitalRead(5)); // Valve 4
        }
      
        state[0] = 1 - digitalRead(2);
        state[1] = 1 - digitalRead(3);
        state[2] = 1 - digitalRead(4);
        state[3] = 1 - digitalRead(5);
    
        return;
    }
    
    if (isAuto) {
      // just kidding, not a pattern, just random shit
      if (isPattern) {
          
          state[0] = random(2);
          state[1] = random(2);
          state[2] = random(2);
          state[3] = random(2);
        
          Serial.print(state[0]);
          Serial.print(",");
          Serial.print(state[1]);
          Serial.print(",");
          Serial.print(state[2]);
          Serial.print(",");
          Serial.println(state[3]); 
          
          trigger_solenoids_serial();
          //int sleepVal = random(1000,10000);
          //Serial.println(sleepVal);
          delay(random(1000,10000));
      } else {   
          //TODO: Implement callback/event function
          solenoidTimer.run();
          
          for (int k=0; k<CHANNELS; k++) {
            int piezoVal = analogRead(VIBR_PIN[k]);
  
            piezo_moving_average(k, piezoVal / 1023.0 * senseVal);

            if (bufferResult[k] > THRESHOLD){
              state[k] = 1;
            } else {
              digitalWrite(LED_PIN, LOW);
            }
          }

          // output to serial monitor
          if (debugSensors) {         
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
}


// implements circular buffer for moving average
void piezo_moving_average(int channel, float measurement) {
  int curPos = iter[channel];                
  float newVal = measurement / (float)SAMPLE_SIZE;  
  float oldVal = measurements[channel][curPos];
  float newSum = bufferResult[channel] + newVal - oldVal;
  //Serial.println(i);
  //Serial.println(oldVal);
  // push in new result, discard old result
  bufferResult[channel] = (newSum > MAX_VALUE) ? MAX_VALUE : bufferResult[channel] + newVal - oldVal;  
  //Serial.println(bufferResult[channel]);
  measurements[channel][curPos] = newVal;
  
  iter[channel]++;
  if (iter[channel]>=SAMPLE_SIZE) { iter[channel]=0; }

}


