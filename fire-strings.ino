#define SAMPLE_SIZE 50
#define DELAY 10
#define THRESHOLD 2000
#define MAX_BOUND 10000
#define SOL_DURATION 200
#define SOL_REFRACTORY 1000

int LED_Pin = 13;
int vibr_Pin =3;
int SOL_Pin = 10;
int i = 0;

double measurements[SAMPLE_SIZE];
double bufferResult = 0;
double peak = 0;
bool readVibration = true;

void setup(){
  pinMode(LED_Pin, OUTPUT);
  pinMode(SOL_Pin, OUTPUT);
  pinMode(vibr_Pin, INPUT); //set vibr_Pin input for measurment
  Serial.begin(9600); //init serial 9600

  reset_buffer();
}

void reset_buffer() {
  Serial.println("Resetting buffer....");
  for (int j=0; j<SAMPLE_SIZE; j++) {
    measurements[j]=0;
  }
  bufferResult = 0;
}

void trigger_solenoid() {
    digitalWrite(SOL_Pin, HIGH);
    Serial.println("Actuation.....");
    delay(SOL_DURATION);    
    digitalWrite(SOL_Pin, LOW);    
    Serial.println("Refractory.....");
    delay(SOL_REFRACTORY); 
}

void loop(){  
     
    if (readVibration) {
      long measurement = TP_init();
      delay(DELAY);
      // Serial.print("measurment = ");
      next_moving_average(measurement);
      //Serial.println("Average: ");
      Serial.println(bufferResult);
  
      if (bufferResult > THRESHOLD){
        digitalWrite(LED_Pin, HIGH);
        trigger_solenoid();
        reset_buffer();
      } else {
        digitalWrite(LED_Pin, LOW); 
      }
    }
}

long TP_init(){
  long measurement = pulseIn (vibr_Pin, HIGH);  //wait for the pin to get HIGH and returns measurement
  return measurement;
}

// implement circular buffer
void next_moving_average(long measurement) {

  double newVal = (double)measurement / (double)SAMPLE_SIZE;  
  double oldVal = measurements[i];
  
  //Serial.println(i);
  //Serial.println(oldVal);
  bufferResult = bufferResult + newVal - oldVal; 
  measurements[i] = newVal;
  
  //TODO: Improve smoothing function - add an upper bound, 
  //reset the function if the buffer result drops below a certain lower bound
  
  i++;
  if (i>=SAMPLE_SIZE) { i=0;}

}


