
int capMuxPins[8] = {22, 21, 20, 19, 4, 3, 2, 1};
int capMuxReadPins[2] = {18, 0};
float SnapMultiplier = 0.001;
float error;

int capMux1EnablePin = 23;
int capMux1Pin1 = 22;
int capMux1Pin2 = 21;
int capMux1Pin3 = 20;
int capMux1Pin4 = 19;
int capMux1ReadPin = 18;

int capMux2EnablePin = 5;
int capMux2Pin1 = 4;
int capMux2Pin2 = 3;
int capMux2Pin3 = 2;
int capMux2Pin4 = 1;
int capMux2ReadPin = 0;

int capValues[24];

int note_base=65;
int notes[128];

void setup() {
  Serial.begin(9600);
  pinMode(capMux1EnablePin, OUTPUT); 
  pinMode(capMux1Pin1, OUTPUT); 
  pinMode(capMux1Pin2, OUTPUT); 
  pinMode(capMux1Pin3, OUTPUT); 
  pinMode(capMux1Pin4, OUTPUT); 

  pinMode(capMux2EnablePin, OUTPUT);
  pinMode(capMux2Pin1, OUTPUT); 
  pinMode(capMux2Pin2, OUTPUT); 
  pinMode(capMux2Pin3, OUTPUT); 
  pinMode(capMux2Pin4, OUTPUT); 

  digitalWrite(capMux2EnablePin, LOW);
  digitalWrite(capMux1EnablePin, LOW);

  int i;
  for (i=0;i<128;i++) notes[i]=0;
}
//
void loop() {
  readCapMux(0);
//  readCapMux(1);
}

void readCapMux(int number){ //function to read the sensors through the multiplexors
  Serial.print(number); //write sensor number in the serial monitor
  Serial.print(":  ");
  int i;
  for(i = 0; i < 12; i++){ //tell the multiplexor which sensor to read
    digitalWrite(capMuxPins[0 + (4 * number)], bitRead(i, 0)); 
    digitalWrite(capMuxPins[1 + (4 * number)], bitRead(i, 1));
    digitalWrite(capMuxPins[2 + (4 * number)], bitRead(i, 2));
    digitalWrite(capMuxPins[3 + (4 * number)], bitRead(i, 3));
    capValues[i + (12 * number)] = touchRead(capMuxReadPins[number]); // read the sensor, and write its value to its place in the array
    
    Serial.print(capValues[i + (12 * number)]); //write the sensors value to the serial monitor
    Serial.print("  ");
  }
  Serial.println();
  int afterTouch;
  int temp;
  for (i=0; i<13; i++) { //cycle over the 12 notes / sensors
    if (capValues[i]>138){// Minimum value that makes sound
      if(notes[note_base+i]==0){ //if current note is not already sounding
        notes[note_base+i]=1; //write a "1" to the notes place in the array
        usbMIDI.sendNoteOn(note_base+i, 127, 1); // and send the "note on"
      }
      else{   //if note is already sounding
        afterTouch = map(capValues[i], 138, 200, 0, 127); //calculate aftertouch (or CC2 = "wind controller")
        if(afterTouch > 127){ //limit after touch to MIDI range
          afterTouch = 127;
        }
        else if(afterTouch < 0){
          afterTouch = 0;
        }
        usbMIDI.sendControlChange(2, afterTouch, 1); //send after-touch for current note
      }
    }
    
    else if (capValues[i]>115 && capValues[i]<130) { //if value of sensor is between these values
      notes[note_base+i]=0; //write a "0" in the notes place in the array
      usbMIDI.sendNoteOn(note_base+i, 0, 1); // and send a MIDI  note-off for the note
    }
  }
}



// EXPONENTIAL FILTER:

float snapCurve(float x){
  float y = 1.0 / (x + 1.0);
  y = (1.0 - y) * 2.0; //Multiplier here
  if(y > 1.0) {
    return 1.0;
  }
  return y;
}

int expFilter(int newValue, int lastValue, int resolution, float snapMult){
  unsigned int diff = abs(newValue - lastValue);
  error += ((newValue - lastValue) - error) * 0.4;
  float snap = snapCurve(diff * snapMult);
  float outputValue = lastValue;
  outputValue  += (newValue - lastValue) * snap;
  if(outputValue < 0.0){
    outputValue = 0.0;
  }
  else if(outputValue > resolution - 1){
    outputValue = resolution - 1;
  }
  return (int)outputValue;
}

void isort(int *a, int n){
  // *a is an array pointer, n is the array size
  for (int i = 1; i < n; ++i)
  {
    int j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--)
    {
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
}
