#include <ESP8266WiFi.h>


const char* ssid = "ENTER YOUR SSID";           
const char* password = "ENTER YOUR SSID PASSWORD";

WiFiServer server(80); 


const int ledPin = D1; 
const int motorPinA = D5; 
const int motorPinB = D6;


const int trigPin = D2;
const int echoPin = D3;

long duration;
int distance;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT); 
  digitalWrite(ledPin, LOW);

  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi!");
  Serial.println(WiFi.localIP());
  server.begin(); 
}

void loop() {
  WiFiClient client = server.available(); 

  if (client) {
    String request = client.readStringUntil('\r'); 
    Serial.println("Request: " + request);
    client.flush();

    if (request.indexOf("DROWSY") != -1) {
      Serial.println("Driver is drowsy!");
      digitalWrite(ledPin, HIGH);

      distance = measureDistance();
      Serial.print("Distance ahead: ");
      Serial.print(distance);
      Serial.println(" cm");

      if (distance > 30) { 
        slowDownMotors();
      } else {
        Serial.println("Obstacle too close! Staying in lane.");
        stopMotors();
      }
    }

    if (request.indexOf("NOT_DROWSY") != -1) {
      Serial.println("Driver is not drowsy.");
      digitalWrite(ledPin, LOW);
      resumeMotors(); 
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>Signal received!</h1>");
    client.stop(); 
  }
}

void slowDownMotors() {
  for (int speed = 1023; speed >= 0; speed -= 100) {
    analogWrite(motorPinA, speed);  
    analogWrite(motorPinB, speed);  
    delay(200);                     
  }
  stopMotors();
  Serial.println("Motors slowed down and stopped.");
}

void resumeMotors() {
  analogWrite(motorPinA, 1023);
  analogWrite(motorPinB, 1023);
  Serial.println("Motors resumed.");
}

void stopMotors() {
  analogWrite(motorPinA, 0);  
  analogWrite(motorPinB, 0);  
}

int measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  int cm = duration * 0.034 / 2;
  return cm;
}
