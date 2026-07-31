#include <ESP8266WiFi.h>
#include <DHT11.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>

// DHT11 센서 설정
int pin = 2;
DHT11 dht11(pin);
float temp, humi;

Servo servo; // 서보 모터 객체 생성

// 모터 드라이버 설정
#define INAPIN 0 // 모터 드라이버의 INA 핀 (GPIO0)
#define INBPIN 4 // 모터 드라이버의 INB 핀 (GPIO4)

// 서보 모터 핀 설정
#define SERVO_PIN D1 // 서보 모터 핀 (GPIO5)

// WiFi 설정
const char* ssid = "Tear"; // 자신의 SSID로 변경
const char* password = "kangwonseok"; // 자신의 비밀번호로 변경

WiFiServer server(80);

bool motorState = false; // 모터 상태

void setup() {
  Serial.begin(115200);
  delay(10);

  servo.attach(SERVO_PIN); // 서보 모터 핀 설정

  pinMode(INAPIN, OUTPUT);
  pinMode(INBPIN, OUTPUT);
  digitalWrite(INAPIN, LOW); // 초기에는 모터를 끔
  digitalWrite(INBPIN, LOW); // 초기에는 모터를 끔

  // WiFi 연결
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  dht11.read(humi, temp);
  
  // 조건 확인: 온도가 30도 이상이고 습도가 70% 이상일 때
  if (temp >= 30 && humi >= 70) {
    servo.write(0); // 서보 모터 닫기
    digitalWrite(INAPIN, HIGH); // 모터를 켜고 정방향 회전
    digitalWrite(INBPIN, LOW);
    motorState = true;
  }

  WiFiClient client = server.available(); // 클라이언트 연결 대기

  if (client) {
    Serial.println("New client");
    String currentLine = "";
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        request += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // HTTP 헤더 전송
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // HTML 페이지 전송
            client.print("<!DOCTYPE html><html>");
            client.print("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.print("<link rel=\"icon\" href=\"data:,\">");
            client.print("<style>html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}");
            client.print("h1 { font-size: 2.0rem; } p { font-size: 1.5rem; }</style></head>");
            client.print("<body><h1>HATE SUMMER SMART SYSTEM</h1>");

            // DHT11 센서에서 온도 및 습도 읽기
            String h, t;
            h = String(humi);
            t = String(temp);

            client.print("<p>Temperature: ");
            client.print(t);
            client.print(" &deg;C</p>");
            client.print("<p>Humidity: ");
            client.print(h);
            client.print(" %</p>");

            // 시리얼 모니터에서 온도 확인
            Serial.print("Temperature: ");
            Serial.print(t);
            Serial.println(" °C");

            // 모터 제어 버튼
            client.print("<p>Motor State: ");
            if (motorState) {
              client.print("ON</p>");
              client.print("<p><a href=\"/motoroff\"><button>Turn Motor OFF</button></a></p>");
            } else {
              client.print("OFF</p>");
              client.print("<p><a href=\"/motoron\"><button>Turn Motor ON</button></a></p>");
            }

            // 서보 제어 버튼
            client.print("<p>Servo Control: </p>");
            client.print("<p><a href=\"/servoopen\"><button>Open</button></a></p>");
            client.print("<p><a href=\"/servoclose\"><button>Close</button></a></p>");

            client.print("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    // 모터 제어
    if (request.indexOf("GET /motoron") != -1) {
      motorState = true;
      digitalWrite(INAPIN, HIGH); // 모터를 켜고 정방향 회전
      digitalWrite(INBPIN, LOW);
    } else if (request.indexOf("GET /motoroff") != -1) {
      motorState = false;
      digitalWrite(INAPIN, LOW); // 모터를 끔
      digitalWrite(INBPIN, LOW);
    }

    // 서보 제어
    if (request.indexOf("GET /servoopen") != -1) {
      servo.write(140); // 90도로 서보 모터 회전 (열기)
    } else if (request.indexOf("GET /servoclose") != -1) {
      servo.write(0); // 0도로 서보 모터 회전 (닫기)
    }

    client.stop();
    Serial.println("Client Disconnected.");
  }
}
