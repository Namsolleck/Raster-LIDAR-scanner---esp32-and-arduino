#include <Servo.h>

Servo servoX;
Servo servoY;

int posX = 45;
int posY = 120; 
int stepX = 1;

void setup() {

  Serial1.begin(9600); 
  servoX.attach(9);
  servoY.attach(10);
   
  servoX.write(posX);
  servoY.write(posY);
  delay(1000); 
}

void loop() {
  if (Serial1.available() > 0) {
    char cmd = Serial1.read();
    

    if (cmd == 'G') {
      posX += stepX;

      if (posX > 135 || posX < 45) {
        stepX = -stepX; 
        posX += stepX;  
        posY -= 1;      
      }

      if (posY < 60) {  
        posY = 120; 
        posX = 45;
        stepX = 1;
      }

      servoX.write(posX);
      servoY.write(posY);
      
      delay(35);
      
      Serial1.print("R,");
      Serial1.print(posX);
      Serial1.print(",");
      Serial1.println(posY);
    }
  }
}
