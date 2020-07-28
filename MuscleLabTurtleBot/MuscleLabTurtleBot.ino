#define ESP8266_LED 2
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
//#define D9 3
//#define D10 1
//#define D11 9
//#define D12 10

///* ROS Test with ESP Soft Access Point
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>



#include <ros.h>
#include <std_msgs/Int8.h>
#include <std_msgs/String.h>



IPAddress    apIP(42, 42, 42, 42);  // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1

const char *ssid = "ESP8266";
const char *password = "ESP8266Test";

// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);


int dir = 0;
int ySpeed = 1; //0 = Back, 1 = Stop, 2 = Forward
int xSpeed = 1; //0 = Left, 1 = Stop, 2 = Right

void (*Move[3][3]) (int t);



bool checkLeftDist = false;
bool checkRightDist = false;

// defines variables
long duration = 0;
int leftDistance, rightDistance;

enum Drive {
  forward, back, brake
};

ros::NodeHandle n;

std_msgs::Int8 value;
ros::Publisher espValue("espValue", &value);

std_msgs::String msg;
ros::Publisher espMsg("espMsg", &msg);


void espValueCb(const std_msgs::Int8& cbValue) {
  Serial.print("Value: ");
  Serial.println(cbValue.data);
}

ros::Subscriber<std_msgs::Int8> espValueSub("valueFromEsp", &espValueCb, 1000);


void espMsgCb(const std_msgs::String& cbMsg) {
  Serial.print("Value: ");
  Serial.println(cbMsg.data);
}

ros::Subscriber<std_msgs::String> espMsgSub("msgFromEsp", &espMsgCb, 1000);


void LeftSide(Drive dir) {
  switch (dir) {
    case forward:
      digitalWrite(D0, HIGH);
      digitalWrite(D1, LOW);
      break;
    case back:
      digitalWrite(D0, LOW);
      digitalWrite(D1, HIGH);
      break;
    case brake:
    default:
      digitalWrite(D0, LOW);
      digitalWrite(D1, LOW);
      break;
  }
}

void RightSide(Drive dir) {
  switch (dir) {
    case forward:
      digitalWrite(D2, HIGH);
      digitalWrite(D3, LOW);
      break;
    case back:
      digitalWrite(D2, LOW);
      digitalWrite(D3, HIGH);
      break;
    case brake:
    default:
      digitalWrite(D2, LOW);
      digitalWrite(D3, LOW);
      break;
  }
}

void Forward(int t) { //t = time
  LeftSide(forward);
  RightSide(forward);
  delay(t);
}

void Reverse(int t) { //t = time
  LeftSide(back);
  RightSide(back);

  delay(t);
}

void Brake(int t) {
  LeftSide(brake);
  RightSide(brake);

  delay(t);
}

void Right(int t) { //t = time
  LeftSide(forward);
  RightSide(back);
  
  delay(t);
}

void Left(int t) { //t = time
  LeftSide(back);
  RightSide(forward);
  
  delay(t);
}

void ForwardRight(int t) {
  LeftSide(forward);
  RightSide(brake);

  delay(t);
}

void ForwardLeft(int t) {
  LeftSide(brake);
  RightSide(forward);

  delay(t);
}

void ReverseRight(int t) {
  LeftSide(back);
  RightSide(brake);
 
  delay(t);
}

void ReverseLeft(int t) {
  LeftSide(brake);
  RightSide(back);
  
  delay(t);
}

void Right() {
  Right(1000);
}

void Left() {
  Left(1000);
}

void Rect() {
  Forward(500);
  ForwardRight(2000);
  Forward(500);
  ForwardLeft(2000);
  Forward(500);
  ForwardRight(1000);
  ReverseLeft(1000);
  Reverse(500);
  ReverseRight(2000);
  Reverse(500);
  ReverseRight(2000);
  Reverse(500);
  ReverseLeft(2000);
  Reverse(500);
  ReverseLeft(2000);
  Reverse(500);
  ReverseRight(2000);
}

void handleRoot() {
  //digitalWrite (LED_BUILTIN, 0); //turn the built in LED on pin DO of NodeMCU on
  
  dir = server.arg("dir").toInt();

  switch (dir) {
    case 0:
      ySpeed = 1;
      xSpeed = 1;
      break;
    case 1:
      ySpeed --;
      break;
    case 2: 
      xSpeed ++;
      break;
    case 3: 
      ySpeed ++;
      break;
    case 4: 
      xSpeed --;
      break;
    case 5:
      Rect();
      break;
    default:
      break;
  }

  if (ySpeed > 2) {
    ySpeed = 2;
  } else if (ySpeed < 0) {
    ySpeed = 0;
  }

  if (xSpeed > 2) {
    xSpeed = 2;
  } else if (xSpeed < 0) {
    xSpeed = 0;
  }

  (*Move[ySpeed][xSpeed])(1000);

  Serial.println(dir);

  char controls[179] = "<a href=\"/?dir=1\">Up</a>\ 
<a href=\"/?dir=4\">Left</a>  <a href=\"/?dir=0\">Stop</a> <a href=\"/?dir=2\">Right</a>\ 
<a href=\"/?dir=3\">Back</a>  <a href=\"/?dir=5\">Rect</a>";


  char html[1000];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  
// Build an HTML page to display on the web-server root address
  snprintf ( html, 1000,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>ESP8266 WiFi Network</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }\
      h1 { Color: #AA0000; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 Wi-Fi Access Point and Web Server Demo</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>%s<p>\
    <p>This page refreshes every 10 seconds. Click <a href=\"javascript:window.location.reload();\">here</a> to refresh the page now.</p>\
  </body>\
</html>",

    hr, min % 60, sec % 60,
    controls
  );
  server.send ( 200, "text/html", html );
}

void handleNotFound() {
  digitalWrite ( LED_BUILTIN, 0 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void setup() {
  Move[0][0] = ForwardLeft;
  Move[0][1] = Forward;
  Move[0][2] = ForwardRight;
  Move[1][0] = Left;
  Move[1][1] = Brake;
  Move[1][2] = Right;
  Move[2][0] = ReverseLeft;
  Move[2][1] = Reverse;
  Move[2][2] = ReverseRight;
  
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);
  
  delay(1000);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Configuring access point...");

  //set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  
  // You can remove the password parameter if you want the AP to be open. 
  WiFi.softAP(ssid);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
 
  server.on ( "/", handleRoot );
  server.on ( "/dir=0", handleRoot);
  server.on ( "/dir=1", handleRoot);
  server.on ( "/dir=2", handleRoot);
  server.on ( "/dir=3", handleRoot);
  server.on ( "/dir=4", handleRoot);
  server.on ( "/dir=5", handleRoot);
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  
  server.begin();
  Serial.println("HTTP server started");

  n.initNode();
  
  n.advertise(espValue);
  n.advertise(espMsg);

  value.data = 12;
  msg.data = "Message";

  n.subscribe(espValueSub);
  n.subscribe(espMsgSub);
}

void loop() {
  server.handleClient();

  espValue.publish(&value);
  espMsg.publish(&msg);
  n.spinOnce();

  digitalWrite(D5, LOW);
  delayMicroseconds(2);
  digitalWrite(D5, HIGH);  
  delayMicroseconds(10);
  digitalWrite(D5, LOW);

  duration = pulseIn(D7, HIGH);
  rightDistance= duration*0.034/2;

  delay(10);
  
  digitalWrite(D6, LOW);
  delayMicroseconds(2);
  digitalWrite(D6, HIGH);  
  delayMicroseconds(10);
  digitalWrite(D6, LOW);

  duration = pulseIn(D8, HIGH);
  leftDistance= duration*0.034/2;
  
  Serial.print("Distance: ");
  Serial.print(leftDistance);
  Serial.print(", ");
  Serial.println(rightDistance);

  
  Forward(1);

  if (leftDistance < 25) {
    if (checkLeftDist == false) {
      checkLeftDist = true;
    } else {
      Serial.println("Close Left");
      ReverseLeft(100);
    }
  } else if (checkLeftDist == true) {
    checkLeftDist = false;
  }


  if (rightDistance < 25) {
    if (checkRightDist == false) {
      checkRightDist = true;
    } else if (checkLeftDist == true) {
      Reverse(100);
    } else {
      Serial.println("Close Right");
      ReverseRight(100);
    }
  } else if (checkRightDist == true) {
    checkRightDist = false;
  }
}
