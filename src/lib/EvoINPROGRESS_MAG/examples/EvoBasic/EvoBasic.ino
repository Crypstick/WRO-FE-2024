#include <Evo.h>

//Declare EVO object
EVO evo;
void setup() {
  Serial.begin(115200);
  while (!Serial)
  //Initializes the Evolution X1
  evo.begin();
  //Initializes the display
  evo.beginDisplay();
  
  //Gets battery Level and write to display
  Serial.print("Battery Voltage: ");
  Serial.println(evo.getBattery());
  Serial.println("Do not let batteries go below 6.0V");
  evo.writeToDisplay("Battery:", 0, true);
  evo.writeToDisplay(evo.getBattery(), 1);

  //Plays the buzzer for 300ms
  evo.playTone(NOTE_G4, 300);

  //Select I2C channel (0-4)
  //Required before communicating with any sensor on the respective I2C port
  evo.selectI2CChannel(1);


}

void loop() {
  // put your main code here, to run repeatedly:

}
