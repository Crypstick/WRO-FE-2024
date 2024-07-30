#include <Evo.h>
#include <Tones.h>
#include <Magnometer.h>
#include <EV3Motor.h>



#define magnometerPin 1
#define leftSensor 4
#define rightSensor 5


EVO evo;
// Declares Motor and selects port (M1-4), flipped?
EV3Motor steeringMotor(M1, true);
EV3Motor drivingMotor(M2, true);

// Declares Magnometer
Magnometer magnometer(I2C2);
int maxSteer;


//The number of ToF sensors
const int sensorCount = 2;

// associated I2C channel to the sensors
const int sensors[sensorCount] = {leftSensor, rightSensor}; // TO CHANGE ASAP

int leftSensorReading = 0;
int rightSensorReading = 0;

void steerToAngle(int angle, int time);
void resetSteering();
int limit(int value, int min, int max);

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
  evo.begin();
  evo.beginDisplay();
  // Initializes motor
  steeringMotor.begin();
  drivingMotor.begin();
  // Initalises Magnometer
  magnometer.begin();
  magnometer.setTargetHeading();
  // Reset encoder
  steeringMotor.resetAngle();
  drivingMotor.resetAngle();

  //Gets battery Level and write to display
  Serial.print("Battery Voltage: ");
  Serial.println(evo.getBattery());
  Serial.println("Do not let batteries go below 6.0V");
  evo.writeToDisplay(evo.getBattery(), 0);
  evo.writeToDisplay("halloooo chat ", 2, false);
  evo.writeToDisplay("i love kieren", 3, false);

  //Plays the buzzer for 300ms
  evo.playTone(NOTE_G4, 300);

  steeringMotor.brake();
  drivingMotor.brake();

}

void loop() {
  
  for (int i = 0; i < sensorCount; i++) {
    
    evo.selectI2CChannel(sensors[i]);
//  Wire.read(); to read the transmission
  }

}

void steerToAngle(int angle, int time) {
  int now = millis();
  while ((millis() - now) <time) {
    steeringMotor.run((steeringMotor.getAngle()-angle)*-5);
  }
  steeringMotor.brake();
}


int limit(int value, int min, int max) {
  if (value < min) return min;
  else if (value > max) return max;
  else return value;
}