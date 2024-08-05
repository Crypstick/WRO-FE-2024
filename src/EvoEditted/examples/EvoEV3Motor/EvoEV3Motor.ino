#include <Evo.h>
#include <EV3Motor.h>

EVO evo;
// Declares Motor and selects port (M1-4), flipped?
EV3Motor motor(M1, true);
void setup()
{
  Serial.begin(115200);
  evo.begin();
  evo.beginDisplay();
  // Initializes motor
  motor.begin();

  // Reset encoder
  motor.resetAngle();
  // Runs motor at speed(-255 to 255) for degrees
  motor.runDegrees(200, 1000);
  // Runs motor for time
  motor.run(200);
  delay(1000);
  motor.brake();
}

void loop()
{
  // put your main code here, to run repeatedly:
  evo.writeToDisplay(motor.getAngle(), 1, true);
}
