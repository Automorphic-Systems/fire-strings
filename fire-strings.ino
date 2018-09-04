#define SAMPLE_SIZE 50
#define DELAY 10
#define THRESHOLD 1.0
#define MAX_BOUND 10000
#define SOL_DURATION 400
#define SOL_REFRACTORY 1500
#define CHANNELS 2

int LED_Pin = 13;
int VIBR_PIN[CHANNELS] = { A0, A1 };
int SOL_PIN[CHANNELS] = { 10, 11 };

float measurements[CHANNELS][SAMPLE_SIZE];
float bufferResult[CHANNELS];
int state[CHANNELS] = { 0, 1 };
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

void loop(){  
    if (readAnalogVibration) {
      
      for (int k=0; k<CHANNELS; k++) {
        //int piezoADC[CHANNELS] =  { analogRead(VIBR_PIN[0]) , analogRead(VIBR_PIN[1]) };

        int piezoVal = analogRead(VIBR_PIN[k]);
        //Serial.println(piezoVal);     
        
        piezo_moving_average(k, piezoVal / 1023.0 * 25.0);

        if (bufferResult[k] > THRESHOLD){
          digitalWrite(LED_Pin, HIGH);
          trigger_solenoid(k);
          reset_buffer(k);
        } else {
          digitalWrite(LED_Pin, LOW);
        }
      }

      // output to serial monitor
      Serial.print(bufferResult[0]);
      Serial.print(",");
      Serial.println(bufferResult[1]);

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
  
  state[channel]++;
  if (state[channel]>=SAMPLE_SIZE) { state[channel]=0; }

}


