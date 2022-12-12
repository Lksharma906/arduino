#include <EEPROM.h>
#include <String.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>

SoftwareSerial Gsm(10,9);//SIM800L Tx & Rx
char phone_no[]="+918287184812";
String strs[20];
int StringCount = 0;
String pwd = "1234";
int pwdaddr = 0;
boolean getResponse(String expected_answer, unsigned int timeout,boolean reset);

#define pwdLength 4
#define RESET_PIN 4
//#define resetPin 12

void setup() {
//  digitalWrite(resetPin, HIGH);
//  delay(200);
  // put your setup code here, to run once:
  pinMode(RESET_PIN, OUTPUT);

  Serial.begin(115200);
  Gsm.begin(9600);
  Serial.println("default pwd"+pwd);
  pwd = readFromEEPROM(pwdaddr);
  if(pwd.length() < 4)
  {
    pwd ="1234";
  }
  Serial.println("pwd modified"+pwd);
  
  Serial.println("Initializing....");
  initModule("AT","OK",1000);                //Once the handshake test is successful, it will back to OK
  initModule("ATE1","OK",1000);              //this command is used for enabling echo
  initModule("AT+CPIN?","READY",1000);       //this command is used to check whether SIM card is inserted in Gsm Module or not
  initModule("AT+CMGF=1","OK",1000);         //Configuring TEXT mode
  initModule("AT+CNMI=2,2,0,0,0","OK",1000); //Decides how newly arrived SMS messages should be handled  
  Gsm.println("AT+CMGDA= \"DEL ALL\"");       //Delete all essages
  Serial.println("Initialized Successfully"); 

  sendSMS(phone_no,"Network Connected.");

  delay(1000); 
  wdt_enable(WDTO_4S);

}

void loop() {
  // put your main code here, to run repeatedly:
  wdt_reset();  
  delay(1000);
  readSMS();


}

void readSMS(){
  while(Gsm.available()){
    delay(2000);
    String sms = Gsm.readString();
    Serial.println(sms);
  //__________________________________________________________________________
  //if there is an incoming SMS
  if(sms.indexOf("+CMT:") > -1){
    // String callerID = getCallerID(response); //No Need for Caller ID.
    String cmd = getMsgContent(sms);
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    //this if statement will execute only if sim800l received "r" command 
    //and there will be no any phone number stored in the EEPROM
      // if(cmd.equals("p")){
      // // String admin_phone = readFromEEPROM(offsetPhone[0]);
      // // if(admin_phone.length() != phoneLength){
      // //   RegisterPhoneNumber(1, callerID, callerID);
      // //   break;
      // }
      // else if(cmd.equals("n")) {
          doAction(cmd,sms);
        // String text = "Error: Admin phone number is failed to register";
        // Serial.println(text);
        // Reply(text, callerID);
        // break;
      // }
      // else{

      // }
    }  
  }
}  

void sendSMS(char *number, char *msg){
Gsm.print("AT+CMGS=\"");Gsm.print(number);Gsm.println("\"\r\n"); //AT+CMGS=”Mobile Number” <ENTER> - Assigning recipient’s mobile number
delay(500);
Gsm.println(msg); // Message contents
delay(500);
Gsm.write(byte(26)); //Ctrl+Z  send message command (26 in decimal).
delay(5000);  
}

void initModule(String cmd, char *res, int t){
while(1){
    Serial.println(cmd);
    Gsm.println(cmd);
    delay(100);
    while(Gsm.available()>0){
       if(Gsm.find(res)){
        Serial.println(res);
        delay(t);
        return;
       }else{Serial.println("Error");}}
    delay(t);
  }
}

/*******************************************************************************
 * resetSIM800L function
 ******************************************************************************/
boolean resetSIM800L(){
  digitalWrite(RESET_PIN, LOW);
  delay(100);
  digitalWrite(RESET_PIN, HIGH);
  
  boolean flag = false;
  for(int i=1; i<=20; i++){
    Gsm.println("ATE");
    Serial.print(".");
    if(getResponse("NULL", 1000, true) == true)
      {flag = true; break;}
  }
  Gsm.println("ATE");
  if(flag == true){
    for(int i=1; i<=20; i++){
      if(getResponse("SMS Ready", 15000,false) == true)
        {return true;}
    }
  }
  return false;
}

/*******************************************************************************
 * getMsgContent function:
 ******************************************************************************/
String getMsgContent(String buff){
  //+CMT: "+923001234567","","22/05/20,11:59:15+20"
  //Hello  
  unsigned int index, index2;
  index = buff.lastIndexOf("\"");
  index2 = buff.length();

  String data = buff.substring(index+1,index2);
  
  StringCount = 0;
  for(int i =0; i<20;i++)
  {
    strs[i] = "";
  }
  Serial.println("data:"+data);
  
  while (data.length() > 0)
    {
        int new_index = data.indexOf(" ");

        if (new_index == -1) 
        {
          strs[StringCount++] = data;
          break;
        }
        else
        {
          strs[StringCount++] = data.substring(0, new_index);
          data = data.substring(new_index+1);
        }
    }

    // Show the resulting substrings
    for (int i = 0; i < StringCount; i++)
    {
      Serial.print(i);
      Serial.print(": \"");
      Serial.print(strs[i]);
      Serial.println("\"");
    }
  
  String command = strs[0];
  command.trim();
  command.toLowerCase();

  if(command == "a")
  {
    strs[2].trim();
    strs[2].toLowerCase();
  }
//  Serial.print("index+1= "); Serial.println(index+1);
//  Serial.print("index2= "); Serial.println(index2);
//  Serial.print("length= "); Serial.println(buff.length());
  Serial.println("Command:"+command);

  return command;
}
/*******************************************************************************
 * getResponse function
 ******************************************************************************/
boolean getResponse(String expected_answer, unsigned int timeout=1000,boolean reset=false){
  boolean flag = false;
  String response = "";
  unsigned long previous;
  //*************************************************************
  for(previous=millis(); (millis() - previous) < timeout;){
    while(Gsm.available()){
      response = Gsm.readString();
      //----------------------------------------
      //Used in resetSIM800L function
      //If there is some response data
      if(response != ""){
        Serial.println(response);
        if(reset == true)
          return true;
      }
      //----------------------------------------
      if(response.indexOf(expected_answer) > -1){
        return true;
      }
    }
  }
  //*************************************************************
  return false;
}
/*******************************************************************************
 * writeToEEPROM function:
 * Store pwd in EEPROM
 ******************************************************************************/
void writeToEEPROM(int addrOffset, const String &strToWrite)
{
  //byte phoneLength = strToWrite.length();
  //EEPROM.write(addrOffset, phoneLength);
  for (int i = 0; i < pwdLength; i++)
    { EEPROM.write(addrOffset + i, strToWrite[i]); }
}

String readFromEEPROM(int addrOffset)
{
  //byte phoneLength = strToWrite.length();
  char data[pwdLength + 1];
  for (int i = 0; i < pwdLength; i++)
    { data[i] = EEPROM.read(addrOffset + i); }
  data[pwdLength] = '\0';
  return String(data);
}
/*******************************************************************************
 * doAction function:
 * Performs action according to the received sms
 ******************************************************************************/
void doAction(String cmd,String buff){
   
  unsigned int index, index2;

  if(cmd == "p")//pwd
  {
      Serial.println(strs[1]);
      Serial.println(strs[2]);

      if(strs[1].equals(pwd))
      {
       Serial.println("Password Matched"); 
       //write to eeprom
       pwd = strs[2];
       Serial.println("Password is:"+pwd);
       writeToEEPROM(pwdaddr,pwd);
       sendSMS(phone_no,"PWD CHANGED");

       Serial.println("Hold for few seconds system will reboot");
       while(1)
       {
        Serial.write("*");
        delay(100);                                    
       }
  
                            //
  //    delay(10);
      //digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    //  Serial.println("on");
      //delay(1000);               // wait for a second
      //digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
      //Serial.println("off");
//      delay(100);               // wait for a second
//      Serial.println("resetting");
//      delay(10);
//      digitalWrite(resetPin, LOW);
//      Serial.println("this never happens");
      }
      else
      {
        Serial.println("Password Mismatch");
      }
  }
  else if(cmd == "a")//act
  {    

      if(strs[1].equals(pwd))
      {
       Serial.println("Password Matched"); 
        if(strs[2] == "1on")
        {
          Serial.println("RCVD 1on");
          //sendSMS(phone_no,"RCVD 1on.");
        }
        if (strs[2] == "2on")
        {
           Serial.println("RCVD 2on");
           //sendSMS(phone_no,"RCVD 2on");
        }
        if(strs[2] == "1off" )
        {
           Serial.println("RCVD 1off");
           //sendSMS(phone_no,"RCVD 1off");
        }
        if (strs[2] == "2off" )
        {
           Serial.println("RCVD 2off");
           //sendSMS(phone_no,"RCVD 2off");
        }
       
      }
      else
      {
        Serial.println("Password Mismatch");
      }
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  if(cmd == "1on"){
//    //controlRelayGSM(1, RELAY_1,STATE_RELAY_1 = HIGH,caller_id);
//    Serial.println("1on rcvd");
//  }
//  else if(cmd == "1off"){
//    //controlRelayGSM(1, RELAY_1,STATE_RELAY_1 = LOW,caller_id);
//    Serial.println("1off rcvd");
//  }
//  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  else if(cmd == "2on"){  
//    //controlRelayGSM(2, RELAY_2,HIGH,caller_id);
//    //STATE_RELAY_2 = HIGH;
//    Serial.println("2on rcvd");
//  }
//  else if(cmd == "2off"){
//    //controlRelayGSM(2, RELAY_2,LOW,caller_id);
//    //STATE_RELAY_2 = LOW;
//    Serial.println("3On rcvd");
//  }
//  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  else if(cmd == "stat=1"){
//    //sendStatus(1, STATE_RELAY_1,caller_id);
//  }
//  else if(cmd == "stat=2"){
//    //sendStatus(2, STATE_RELAY_2,caller_id);
//  }
//  else if(cmd == "stat=3"){
//    //sendStatus(3, STATE_RELAY_3,caller_id);
//  }
//  else if(cmd == "stat=4"){
//    //sendStatus(4, STATE_RELAY_4,caller_id);
//  }
//  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  // else if(cmd.indexOf("r2=") > -1){
//  //   RegisterPhoneNumber(2, getNumber(cmd), caller_id);
//  // }
//  // else if(cmd.indexOf("r3=") > -1){
//  //   RegisterPhoneNumber(3, getNumber(cmd), caller_id);
//  // }
//  // else if(cmd.indexOf("r4=") > -1){ 
//  //   RegisterPhoneNumber(4, getNumber(cmd), caller_id);
//  // }
//  // else if(cmd.indexOf("r5=") > -1){
//  //   RegisterPhoneNumber(5, getNumber(cmd), caller_id);
//  // }
//  // //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  // else if(cmd == "list"){
//  //   String list = GetRegisteredPhoneNumbersList();
//  //   Serial.println(list);
//  //   Reply(list, caller_id);
//  // }
//  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//  //else if(cmd == "del=1"){  
//    //DeletePhoneNumber(1, caller_id);
//  //}
//  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else{
    String text = "Error: Unknown command: "+cmd;
    Serial.println(text);
    // Reply(text);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}
