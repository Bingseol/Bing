#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>

#define DHTPIN A0           // 온습도 센서를 아날로그 핀 A0번에 연결
#define DHTTYPE DHT11       // DHTTYPE을 DHT11로 설정
#define LEDPIN 9            // LED 핀
#define BOOZER 8            // 부저 핀
#define DUST_SENSOR A1      // 미세먼지 핀
#define GASPIN 4            // 가스 센서  
#define MOTORPIN 5          // 모터 핀 

DHT dht(DHTPIN, DHTTYPE);          // 온습도 센서 DHT 설정

byte mac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x5};
IPAddress ip(xxx,xxx,xxx,xxx); 
// 가로() 안에 이더넷 쉴드의 IP 주소 입력. 컴마(,) 사용에 주의해주세요. 
// 이더넷 라이브러리 초기화
// 사용할 IP 주소 와 포트 입력
// ('port 80' 은 HTTP 의 기본 값 입니다.):
EthernetServer server(80);

void client();

float dust_value = 0; // 센서에서 입력 받은 미세먼지 값
float dustDensityug =  0; // ug/m^3 값을 계산

int GasVal = 1; // 가스센서 값
int ledcheck = 0; // 실시간 LED 상태 확인
int windowCheck = 0; // 창문 열림 확인 
Servo myservo;

void setup()
{
  // 각각의 연결 핀들을 입력, 출력에 맞게 설정
  pinMode(DHTPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BOOZER, OUTPUT);
  pinMode(GASPIN, INPUT);
  myservo.attach(MOTORPIN);

  SPI.begin();
  Serial.begin(9600);

  while (!Serial)
  {
    ; // 포트 연결까지 기다리기. 
  }

  // 이더넷 서버 연결 시작:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  EthernetClient client = server.available();

  if (client){
    boolean currentLineIsBlank = true;
    String buffer ="";
    
    while (client.connected()){
      if (client.available()){
        char c = client.read();
        buffer += c;

        if (c == '\n' && currentLineIsBlank){
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          // 브라우저 5초마다 새로고침
          client.println("<meta http-equiv=\"refresh\" content=\"1\">");
          client.println("<title>");
          client.print("Welcom ARDUINO");
          client.println("</title>");
          client.println("<center>");
          client.println("<h1>");
          client.print("IoT SMART HOME");
          client.println("</h1>");

          // 온습도 센서
          int h = dht.readHumidity();
          int t = dht.readTemperature();
          client.println("<h2>");
          client.print("REAL-TIME TEMPERATURE AND HUMIDITY");
          client.println("</h2>");
          client.println("<h3>");
          client.print("Temperature : ");
          client.print(t);
          client.print("'C");
          client.print("<br>");
          client.print("Humidity : ");
          client.print(h);
          client.print("%");
          client.println("<br>");
          client.println("<br>");

          if(t > 10){
            tone(BOOZER, 523, 3000); // BOOZER 핀, 523 주파수, 3초간 
          }
          delay(100);

          // LED 센서 
          client.println("<h2>");
          client.print("REAL-TIME LED CONTROL");
          client.println("</h2>");
          client.println("<h3>");
          client.print("LED : ");
          
          if(ledcheck == 1){
            client.print("ON");
          }
          else if(ledcheck == 0){
            client.print("OFF");
          }
          client.println("</h3>");
          client.println("<a href=\"/LEDON\"\"><button> LED ON</button></a>");
          client.println("<a href=\"/LEDOFF\"\"><button> LED OFF</button></a>");
          client.println("<br>");
          client.println("<br>");                                      

          // LED ON OFF
          if(buffer.indexOf("/LEDON") != -1){ //checks for LEDON
              digitalWrite(LEDPIN, HIGH); // set pin high
              ledcheck = 1;
          }
          else if(buffer.indexOf("/LEDOFF") != -1){ // checks for LEDOFF
            digitalWrite(LEDPIN, LOW);
            ledcheck = 0;
          }
          delay(100);
          
          // 미세먼지 센서
          dust_value = analogRead(DUST_SENSOR);
          dustDensityug = (0.17 * (dust_value * (5.0 / 1024)) - 0.1) * 1000;
          Serial.println(dustDensityug);
          client.println("<h2>");
          client.print("REAL-TIME DUST DETECTING");
          client.println("</h2>");
          client.println("<h3>");
          client.print("Dust Sensor : ");
          client.print(dustDensityug);
          client.print("[ug/m3]");
          client.print("<br>");
          if(dustDensityug > -90.0){
            client.println("BAD");

            if(windowCheck == 1){
              windowOpen();
            }
          }
          else{
            client.println("GOOD");
          }
          client.println("</h3>");
          client.println("<br>");
          client.println("<br>");
          delay(100);

          // 가스 센서
          client.println("<h2>");
          client.print("REAL-TIME GAS DETECTING");
          client.println("</h2>");
          client.println("<h3>");
          client.println("Gas Sensor : ");
          
          GasVal = digitalRead(GASPIN);
          if(GasVal == 1){ // 가스가 감지되지 않았을 때 
            client.println("GAS NO DETECTING");
          }
          else if(GasVal == 0){ // 가스가 감지되었을 때 
            client.println("GAS WARNING");
            if(windowCheck == 0){
              windowOpen(); 
            }
            tone(BOOZER, 523, 3000); // BOOZER 핀, 523 주파수, 3초간
          }
          client.println("</h3>");
          client.println("<br>");
          client.println("<br>");
          delay(100);

          client.println(("    </form>"));
          client.println("<center>");
          client.println("</html>");

          break;
        }
        if(c == '\n'){
          currentLineIsBlank = true;
        }
        else if(c != '\r'){
          currentLineIsBlank = false;
        }
      }
    }
    
    // 연결 종료:
    client.stop();
    Serial.println("client disonnected"); // 브라우저 데이터 받는 시간
  }
}

void windowOpen(){
  if(windowCheck == 1){ // 창문 열려있으면 
    myservo.write(90);
    windowCheck = 0; // 닫힌 상태 0 
  }
  else if(windowCheck == 0){ // 창문 열기 
    myservo.write(180);
    windowCheck = 1; // 열린 상태 1
  }
}
