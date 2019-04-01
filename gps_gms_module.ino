

 #include <SoftwareSerial.h>

SoftwareSerial sim808(7,8);
char phone_no[] = "xxxxxxx"; 
String content = "";
char character;
String data[5];
byte buffer[64]; //Buffer to receive sms data
int count=0; 
byte pos = 0;
int led13 = 10;
#define DEBUG true

String timegps, latitude, longitude;
int state = 0;
const int buttonPin = 5;     // the number of the pushbutton pin
#define phonenumber  "+27742842345"
// variables will change:
int buttonState = 0;
int count1 = 0;
int count2 = 5;

void setup()
{

  sim808.begin(19200);   // Setting the baud rate of GSM Module  
  Serial.begin(19200);    // Setting the baud rate of Serial Monitor (Arduino)
  delay(100);
  sim808.println("AT+CSQ");
  delay(1000);
  sim808.println("AT+CGATT?");
  delay(1000);
  sim808.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(1000);
  sim808.println("AT+SAPBR=3,1,\"APN\",\"Cell C internet\"");
  delay(1000);
  sim808.println("AT+SAPBR=1,1");
  delay(1000);
  sim808.println("AT+HTTPINIT");
  delay(1000);
  
  sim808.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  delay(1000);
  sim808.print("AT+CMGF=1\r");
  delay(400);
  sendData("AT+CGNSPWR=1", 1000, DEBUG);
  delay(50);
  sendData("AT+CGNSSEQ=RMC", 1000, DEBUG);
  delay(150);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  pinMode(led13,OUTPUT);
  

  
}


void loop()
{


  getGPSData();
  
  delay(5000);
    //check for sms
  if (sim808.available())              
  {
    while(sim808.available())         
    {
      buffer[count++]=sim808.read();     
      if(count == 64)break;
  }
    Serial.write(buffer,count);            
    Cmd_Read_Act();                        
    clearBufferArray();              
    count = 0;                       
 
 
  }
  if (Serial.available())           
    sim808.write(Serial.read());  
       
  
  

}

void sendSMSLocation (){
    sim808.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
    delay(1000);  // Delay of 1 second
    sim808.println("AT+CMGS=\"+27614917061\"\r"); // Replace x with mobile number
    delay(1000);
    sim808.print(latitude);
    sim808.print(",");
    sim808.print (longitude);
    delay(100);
    sim808.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module 
    delay(1000);
    Serial.write(sim808.read());
}

void sendSMS (String textSms){
    sim808.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
    delay(1000);  // Delay of 1 second
    sim808.println("AT+CMGS=\"+27614917061\"\r"); // Replace x with mobile number
    delay(1000);
    sim808.print(textSms);
    delay(100);
    sim808.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module 
    delay(1000);
    Serial.write(sim808.read());
}

void notifyPolice (){
    sim808.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
    delay(1000);  // Delay of 1 second
    sim808.println("AT+CMGS=\"+27611557995\"\r"); // Replace x with mobile number
    delay(1000);
    sim808.print("Vehicle Brand: BWM\n Vehicle color: Red\n Plate number: LD-948323-YA\n Vehicle mmodel: SS34 ");
    sim808.print("http://maps.google.com/maps?q=loc:");
    sim808.print(latitude);
    sim808.print(",");
    sim808.print (longitude);
    delay(100);
    sim808.println((char)26);// ASCII code of CTRL+Z for saying the end of sms to  the module 
    delay(1000);
    Serial.write(sim808.read());
}


void getGPSData(){
sendTabData("AT+CGNSINF",1000,DEBUG);
  if (state > 0) {
    
      //read the state of the pushbutto
  if (count1 <= 0){
    
      sentHttp();
      delay(1000);
      count1++;
      count2 = 5;
  }
  
  if (count2 <= 1){
    count1 = 0;
  }else{
    count2--;
  }

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    notifyPolice();
    digitalWrite(led13, HIGH);
  } else {
    // turn LED off:
    digitalWrite(led13, LOW);
  }
  
    
    Serial.println("State  :"+String(state));
    Serial.println("Time  :"+timegps);
    Serial.println("Latitude  :"+latitude);
    Serial.println("Longitude  :"+longitude);
      

  } else {
    Serial.println("GPS Initialising...");
    delay(3000);
  }
  
}


void sendTabData(String command , const int timeout , boolean debug){

  sim808.println(command);
  long int time = millis();
  int i = 0;

  while((time+timeout) > millis()){
    while(sim808.available()){
      char c = sim808.read();
      if (c != ',') {
         data[i] +=c;
         delay(100);
      } else {
        i++;  
      }
      if (i == 5) {
        delay(100);
        goto exitL;
      }
    }
  }exitL:
  if (debug) {
    state = data[1].toInt();
    timegps = data[2];
    latitude = data[3];
    longitude =data[4];  
    resetBuffer();
    data[1] = "";
    data[2] ="";
    data[3] ="";
    data[4] ="";
  }
}
String sendData (String command , const int timeout ,boolean debug){
  String response = "";
  sim808.println(command);
  long int time = millis();
  int i = 0;

  while ( (time+timeout ) > millis()){
    while (sim808.available()){
      char c = sim808.read();
      response +=c;
    }
  }
  if (debug) {
     Serial.print(response);
     }
     return response;
}

// Limpia el buffer
void clearBufferArray()              
{
  for (int i=0; i<count;i++)
    { buffer[i]=NULL;}                  // borrar todos los índices del arreglo
}

//////Esta función lee los SMS enviados al escudo SIM900 y actua en base a esa orden.
void Cmd_Read_Act(void)          
{  
  char buffer2[64];
  char comparetext[25];    
  for (int i=0; i<count;i++)
  { buffer2[i]=char(buffer[i]);}  
  memcpy(comparetext,buffer2,25); 
  //if (strstr(comparetext,phonenumber)){
///////////LED13///////////////////////////////////////    
  if (strstr(buffer2,"on")){
      digitalWrite(led13, HIGH);
      //check gps status
      
      if (state > 0){
         sendSMSLocation();
        
      }else{
        sendSMS("Waiting for GPS Signal try in few minutes");
        
      }
     
    
      
    }
  if (strstr(buffer2,"getgps")){
    if (state != 0){
      sendSMSLocation();
    }
    
  }
    
 // }
}

void resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void sentHttp(){
  sim808.print("AT+HTTPPARA=\"URL\",\"still-savannah-78552.herokuapp.com/api/locations?");// setting the httppara
  sim808.print("&latitude=");
  sim808.print(latitude);
  sim808.print("&longitude=");
  sim808.print(longitude);
  sim808.print("&user_id=1");
  sim808.print("&vehicel_id=1");
  sim808.println("\"");
  delay(1000);
  sim808.println("AT+HTTPACTION=1");//submit post request
  delay(1000);
}

