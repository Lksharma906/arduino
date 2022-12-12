#include <avr/wdt.h>

void setup() {
  // put your setup code here, to run once:
  Serial.println("Iniializing...");
  delay(1000);
  Serial.begin(9600);
  Serial.println("Enabling WDT");
  wdt_enable(WDTO_4S);

}

void loop() {
  // put your main code here, to run repeatedly:
  wdt_reset();
  delay(1000);
  Serial.println("PING");
  if(Serial.available()>0)
  {
    delay(100);
    if(Serial.find("RESET"))
    {
      Serial.println("found condition doing wdt reset");
      while(1)
      {
        Serial.write("*");
        delay(100);
      }
    }
  }
}
