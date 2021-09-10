//BEAR Code for Arduino LED (Blinking)

// Defining User Parameters
void setup() {
  
  LED = 5; //Sets the LED to pin 5
  pinMode(LED, OUTPUT); //Sets the LED pin to output
  
}


//Infinitely loops the blinking LED
void loop() {
  
  digitalWrite(LED, HIGH);   // Turns the LED on
  delay(1000);               // for 1 second
 
  digitalWrite(LED, LOW);    // Turns the LED off
  delay(1000);               // for 1 second
  
}
