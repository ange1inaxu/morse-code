#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

char network[] = "MIT";
char password[] = "";
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

const char USER[] = "angelx";

const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const uint8_t BUTTON1 = 45; //pin connected to button
const uint8_t BUTTON2 = 39; //pin connected to button
const uint8_t BUTTON3 = 38; //pin connected to button
const uint8_t BUTTON4 = 34; //pin connected to button

uint8_t button_state = 0; //state of Morse code entry
uint8_t post_state = 0; //state of posting

char morse_code[100] = ""; // words to be updated with Morse code entry
char current_letter[100] = "";
char deciphered[100] = "";
int valid = 1; //valid Morse code translation; initialize as true


void setup() {
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background

  Serial.begin(115200); //begin serial comms
  delay(100); //wait a bit (100 ms)
  connect_wifi();

  pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input!
  pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input!
  pinMode(BUTTON3, INPUT_PULLUP); //set input pin as an input!
  pinMode(BUTTON4, INPUT_PULLUP); //set input pin as an input!
}


void loop() {
  button_fsm();

  // reset INVALID or POSTED on screen
  if (strlen(morse_code) > 0){
    tft.setCursor(0, 80);
    tft.println("               ");
  }

  tft.setCursor(0, 0, 4);
  tft.println(morse_code); //print morse code
  post_reporter_fsm();
}


/*----------------------------------
Button state machine, uses buttons 1, 2, 3 as input
*/
int last_button3_read = 0;
void button_fsm() {
  if (button_state==0){ //button idle
    
    if (!digitalRead(BUTTON1)){ //pushed
      strcat(morse_code, ".");
      strcat(current_letter, ".");
      button_state = 1;

    } else if (!digitalRead(BUTTON2)){ //pushed
      strcat(morse_code, "_");
      strcat(current_letter, "_");
      button_state = 2;

    } else if (!digitalRead(BUTTON3)){ //pushed
      if (millis() - last_button3_read < 1000){ //double click for space
        strcat(morse_code, " ");
        strcat(deciphered, " ");
      }
      button_state = 3;
      last_button3_read = millis();
    }
  }

  else if (button_state==1){ //button1 released
    if (digitalRead(BUTTON1)){ //unpushed
      button_state = 0;
    }
  }

  else if (button_state==2){ //button2 released
    if (digitalRead(BUTTON2)){ //unpushed
      button_state = 0;
    }
  }

  else if (button_state==3){ //new letter and translation
    if (digitalRead(BUTTON3)){ //unpushed
      button_state = 0;
      Serial.println("current_letter");
      Serial.println(current_letter);
      map_letter();
    }
  }
}


/*----------------------------------
Given current_letter, find the correct mapping and concatenate to deciphered
*/
void map_letter(){
  // returns 0 if equal, 1 if not
  if (strcmp(current_letter, "._")==0){strcat(deciphered, "A");}
  else if (strcmp(current_letter, "_...")==0){strcat(deciphered, "B");}
  else if (strcmp(current_letter, "_._.")==0){strcat(deciphered, "C");}
  else if (strcmp(current_letter, "_..")==0){strcat(deciphered, "D");}
  else if (strcmp(current_letter, ".")==0){strcat(deciphered, "E");}
  else if (strcmp(current_letter, ".._.")==0){strcat(deciphered, "F");}
  else if (strcmp(current_letter, "__.")==0){strcat(deciphered, "G");}
  else if (strcmp(current_letter, "....")==0){strcat(deciphered, "H");}
  else if (strcmp(current_letter, "..")==0){strcat(deciphered, "I");}
  else if (strcmp(current_letter, ".___")==0){strcat(deciphered, "J");}
  else if (strcmp(current_letter, "_._")==0){strcat(deciphered, "K");}
  else if (strcmp(current_letter, "._..")==0){strcat(deciphered, "L");}
  else if (strcmp(current_letter, "__")==0){strcat(deciphered, "M");}
  else if (strcmp(current_letter, "_.")==0){strcat(deciphered, "N");}
  else if (strcmp(current_letter, "___")==0){strcat(deciphered, "O");}
  else if (strcmp(current_letter, ".__.")==0){strcat(deciphered, "P");}
  else if (strcmp(current_letter, "__._")==0){strcat(deciphered, "Q");}
  else if (strcmp(current_letter, "._.")==0){strcat(deciphered, "R");}
  else if (strcmp(current_letter, "...")==0){strcat(deciphered, "S");}
  else if (strcmp(current_letter, "_")==0){strcat(deciphered, "T");}
  else if (strcmp(current_letter, ".._")==0){strcat(deciphered, "U");}
  else if (strcmp(current_letter, "..._")==0){strcat(deciphered, "V");}
  else if (strcmp(current_letter, ".__")==0){strcat(deciphered, "W");}
  else if (strcmp(current_letter, "_.._")==0){strcat(deciphered, "X");}
  else if (strcmp(current_letter, "_.__")==0){strcat(deciphered, "Y");}
  else if (strcmp(current_letter, "__..")==0){strcat(deciphered, "Z");}
  else if (strcmp(current_letter, ".____")==0){strcat(deciphered, "1");}
  else if (strcmp(current_letter, "..___")==0){strcat(deciphered, "2");}
  else if (strcmp(current_letter, "...__")==0){strcat(deciphered, "3");}
  else if (strcmp(current_letter, "...._")==0){strcat(deciphered, "4");}
  else if (strcmp(current_letter, ".....")==0){strcat(deciphered, "5");}
  else if (strcmp(current_letter, "_....")==0){strcat(deciphered, "6");}
  else if (strcmp(current_letter, "__...")==0){strcat(deciphered, "7");}
  else if (strcmp(current_letter, "___..")==0){strcat(deciphered, "8");}
  else if (strcmp(current_letter, "____.")==0){strcat(deciphered, "9");}
  else if (strcmp(current_letter, "_____")==0){strcat(deciphered, "0");}
  else if (strcmp(current_letter, "")==0){strcat(deciphered, "");}
  else {valid = 0;} //not decipherable; valid = 0;

  Serial.println("MORSE CODE:");
  Serial.println(morse_code);

  if (valid){
    Serial.println("TRANSLATION:");
    Serial.println(deciphered);
  } else {Serial.println("INVALID TRANSLATION");}

  memset(current_letter, 0, sizeof(current_letter));

}

/*----------------------------------
Post reporting state machine, uses button4 as input
use post_state for your state variable!
*/
void post_reporter_fsm() {
  if (post_state == 0){ //idle
    if (!digitalRead(BUTTON4)){ //button down
      post_state = 1;
    }

  } else if (post_state == 1){ // pressed
    if (digitalRead(BUTTON4)){ //button up
      post_state = 2;
    }

  } else if (post_state == 2){ // post
    post_state = 0;
    
    // only allow posting if valid translation and the message is not empty
    if (valid && strlen(deciphered) > 0){
      char body[100]; //for body
      sprintf(body,"{\"user\":\"%s\",\"message\":\"%s\"}",USER,deciphered);//generate body, posting to User, 1 step
      int body_len = strlen(body); //calculate body length (for header reporting)
      sprintf(request_buffer,"POST /sandbox/morse_messenger HTTP/1.1\r\n");
      strcat(request_buffer,"Host: 608dev.net\r\n");
      strcat(request_buffer,"Content-Type: application/json\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //new line
      Serial.println(request_buffer);
      do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);

      tft.fillScreen(TFT_BLACK); //fill background
      tft.setCursor(0, 80);
      tft.println("POSTED");

    }
    else {
      tft.fillScreen(TFT_BLACK); //fill background
      tft.setCursor(0, 80);
      tft.println("INVALID");
    }

    //reset variables
    valid = 1;
    memset(morse_code, 0, sizeof(morse_code));
    memset(deciphered, 0, sizeof(deciphered));
    memset(current_letter, 0, sizeof(current_letter));

  }
}


/*----------------------------------
 * connect_wifi Function:
 * Helper Function to connect to wifi
 * Return value:
 *    void (none)
 */
void connect_wifi(){
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(network, password);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
}

/*----------------------------------
 * char_append Function:
 * Arguments:
 *    char* buff: pointer to character array which we will append a
 *    char c: 
 *    uint16_t buff_size: size of buffer buff
 *    
 * Return value: 
 *    boolean: True if character appended, False if not appended (indicating buffer full)
 */
uint8_t char_append(char* buff, char c, uint16_t buff_size) {
        int len = strlen(buff);
        if (len>buff_size) return false;
        buff[len] = c;
        buff[len+1] = '\0';
        return true;
}


/*----------------------------------
 * do_http_request Function:
 * Arguments:
 *    char* host: null-terminated char-array containing host to connect to
 *    char* request: null-terminated char-arry containing properly formatted HTTP request
 *    char* response: char-array used as output for function to contain response
 *    uint16_t response_size: size of response buffer (in bytes)
 *    uint16_t response_timeout: duration we'll wait (in ms) for a response from server
 *    uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
 * Return value:
 *    void (none)
 */
void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}
