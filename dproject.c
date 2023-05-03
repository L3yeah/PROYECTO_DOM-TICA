#include <WiFi.h>//para conectar arduino a wifi
#include <WebServer.h>//para el server web
#include <HTTPClient.h>//para lo de SQL para que se pueda conectar con la bd
#include <Wire.h>//para display
#include <LiquidCrystal_I2C.h>//para backpack LCD
#include <TB6612FNG.h>//para el puente H


//para el motor
// 27 - Standby pin
// 14 - AIN1 pin //entradas que indican si se va a mover adelante o para atras
// 12 - AIN2 pin //entradas que indican si se va a mover adelante o para atras
// 13 - PWMA pin //potencia
Tb6612fng motor(27, 14, 12, 13);


//para el LCD


LiquidCrystal_I2C lcd(0x27, 16, 2);


//para la cisterna
int pinTrigger = 17;
int pinEcho = 34;
//const int LED_1      = 10;
//const int LED_2      = 9;
//const int LED_3      = 8;
long distancia;
long duracion;


// SSID and password of Wifi connection:
const char* ssid = "CasaDomotica";
const char* password = "93833431";




//SQL por http
String HOST_NAME = "http://192.168.0.101";  // change to your PC's IP address
String PATH_NAME = "/casadomotica/insert.php";


WiFiServer server(80);
uint8_t timeout, intentos; //para conexion del WIFI
bool errorconexion = false; //para conexion del WIFI


//Declaramos LEDS
const int CUARTO1 = 18;
const int CUARTO2 = 19;
const int COCINA = 32;
const int SALA = 33;
const int BANO = 23;
const int BUZZER = 5;
const int SENSOR = 25;
int stateSensor; //estado del sensor magnetico para mostrar si esta abierto o cerrado
const int freq = 5000; // tone del buzzer
const int ledChannel = 0; //tone del buzzer
const int resolution = 8; //tone del buzzer
const int sensorPIR = 4; //pin central del pir al arduino
const int PIRled = 16;
int val = 0; //para el pir si hay movimiento o no hay movimiento






//Variables cliente
char linebuf[80]; //para el msj cuando un cliente se conecta
int charcount = 0; //para el msj cuando un cliente se conecta


void setup() {
  Serial.begin(115200);
  lcd.init();  // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Nivel de agua:");
  motor.begin();//iniciar puente H
  pinMode(sensorPIR, INPUT);
  pinMode(PIRled, OUTPUT);
  pinMode(CUARTO1, OUTPUT);
  pinMode(CUARTO2, OUTPUT);
  pinMode(COCINA, OUTPUT);
  pinMode(SALA, OUTPUT);
  pinMode(BANO, OUTPUT);
  pinMode(BUZZER, OUTPUT); //ventana
  pinMode(SENSOR, INPUT_PULLUP); //ventana
  pinMode(pinTrigger, OUTPUT); //sensor ultrasonico
  pinMode(pinEcho, INPUT);//sensor ultrasonico
  ledcSetup(ledChannel, freq, resolution); //ventana
  ledcAttachPin(BUZZER, ledChannel);//ventana
  for (int i = 0; i > 30; i++)  //Utilizamos un for para calibrar el sensor depende del tipo de sensor que utilicemos va a cambiar el tiempo de calibración
  {
    delay(500);
  }


  WiFi.begin(ssid, password);
  Serial.println("Establishing connection to WiFi with SSID: " + String(ssid));


  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  server.begin();
}


void InsertarSQL(String acc) {


    HTTPClient http;


    http.begin(HOST_NAME + PATH_NAME + "?accion='" + acc);  //HTTP
    int httpCode = http.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }


  http.end();
}


void loop(){
  /*cisterna*/
  digitalWrite(pinTrigger, LOW);
  delay(2);
  digitalWrite(pinTrigger, HIGH);
  delay(1);
  digitalWrite(pinTrigger, LOW);
  duracion = pulseIn(pinEcho, HIGH);
  distancia = (duracion/2)/29;//convertir a CM
  distancia = distancia - 2;//calibrando offset
  distancia = (distancia * 100)/7;
  distancia = distancia - 100;
  distancia = abs(distancia); //funcion de c++ para hacer positivo el porcentaje
  lcd.setCursor(1, 1);
  lcd.print("             ");  // limpiar pantalla
  lcd.setCursor(1, 1);
  lcd.print(String(distancia) + String("%"));
  InsertarSQL(String(distancia) + String("%"));


  /* ventana */
  stateSensor = digitalRead(SENSOR);
  if (stateSensor == HIGH) {
    //Serial.println("La ventana esta abierta");
    ledcWriteTone(ledChannel, 261.626);
    InsertarSQL("Ventana_abierta");
  } else {
    //Serial.println("La ventana esta cerrada");
    ledcWriteTone(ledChannel, 0);
  }
  delay(200);


  /* pagina*/
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Nuevo Cliente");
    memset(linebuf, 0, sizeof(linebuf));
    charcount = 0;
    boolean currentLineIsBlank = true;


    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        linebuf[charcount] = c;
        if (charcount < sizeof(linebuf) - 1) charcount++;


        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: Close");
          client.println("");
          client.println("<!DOCTYPE HTML><html><head>");
          client.println("<title>Doomotica :D </title>");
          client.println("<meta http-equiv='Content-Type' content='text/html;charset=UTF-8'>");
          client.println("<meta name='viewport' content='widht=device-width, initial-scale=1'>");
          //client.println("<link rel='stylesheet' href='file:///C:\xampp\htdocs\casadomotica\bootstrap.min.css' integrity='sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T' crossorigin='anonymous'>");
          client.println("</head>");
          client.println("<body>");
          client.println("<nav class = 'navbar navbar-danger bg-danger'>");
          client.println("<a class='navbar-brand' href='#'>");
          //client.println("<img src='https://upload.wikimedia.org/wikipedia/commons/thumb/e/e0/ArduinoLogo_%C2%AE.svg/250px-ArduinoLogo_%C2%AE.svg.png' width='card-img-top' width='500' height='300'>");
          client.println("<span class='text-white'>Monitoreando</span>");
          client.println("</a>");
          client.println("</nav>");
          client.println("<div class='conteiner'>");
          client.println("<div class='row'>");
          client.println("<div class='col-4'>");
          client.println("<div class='card' style='width: 18rem;'>");
          //client.println("<img src='https://hips.hearstapps.com/hmg-prod.s3.amazonaws.com/images/piso-familiar-coleccionsita-arte-sao-paulo-dormitorio-alfombras-superpuestas-1543243868.jpg' class='card-img-top' width='500' height='300'>");client.println("<div class='card-body'>");
          client.println("<div class='card-body'>");
          client.println("<h5 class='card-title'>Recamara Principal</h5>");
          client.println("<p class='card-text'>El control en tus manos.</p>");
          client.println("<a href='on1' class='btn btn-sucess'>ON</a>");
          client.println("<a href='off1' class='btn btn-sucess'>OFF</a>");
          client.println("</div>");
          client.println("</div>");
          client.println("</div>");
          client.println("<div class='col-4'>");
          client.println("<div class='card' style='width: 18rem;'>");
          //client.println("<img src='https://img.buzzfeed.com/buzzfeed-static/static/2017-06/7/19/asset/buzzfeed-prod-fastlane-01/sub-buzz-12096-1496877184-6.jpg?resize=990:990' class='card-img-top' width='500' height='300'>");
          client.println("<div class='card-body'>");
          client.println("<h5 class='card-title'>Recamara Secundaria</h5>");
          client.println("<p class='card-text'>El control en tus manos.</p>");
          client.println("<a href='on2' class='btn btn-sucess'>ON</a>");
          client.println("<a href='off2' class='btn btn-sucess'>OFF</a>");
          client.println("</div></div></div>");
          client.println("<div class='col-4'>");
          client.println("<div class='card' style='width: 18rem;'>");
          //client.println("<img src='https://img.buzzfeed.com/buzzfeed-static/static/2017-06/7/19/asset/buzzfeed-prod-fastlane-01/sub-buzz-12096-1496877184-6.jpg?resize=990:990' class='card-img-top' width='500' height='300'>");
          client.println("<div class='card-body'>");
          client.println("<h5 class='card-title'>COCINA</h5>");
          client.println("<p class='card-text'>El control en tus manos.</p>");
          client.println("<a href='on3' class='btn btn-sucess'>ON</a>");
          client.println("<a href='off3' class='btn btn-sucess'>OFF</a>");
          client.println("</div></div></div>");
          client.println("<div class='col-4'>");
          client.println("<div class='card' style='width: 18rem;'>");
          //client.println("<img src='https://img.buzzfeed.com/buzzfeed-static/static/2017-06/7/19/asset/buzzfeed-prod-fastlane-01/sub-buzz-12096-1496877184-6.jpg?resize=990:990' class='card-img-top' width='500' height='300'>");
          client.println("<div class='card-body'>");
          client.println("<h5 class='card-title'>SALA</h5>");
          client.println("<p class='card-text'>El control en tus manos.</p>");
          client.println("<a href='on4' class='btn btn-sucess'>ON</a>");
          client.println("<a href='off4' class='btn btn-sucess'>OFF</a>");
          client.println("</div></div></div>");
          client.println("<div class='col-4'>");
          client.println("<div class='card' style='width: 18rem;'>");
          //client.println("<img src='https://img.buzzfeed.com/buzzfeed-static/static/2017-06/7/19/asset/buzzfeed-prod-fastlane-01/sub-buzz-12096-1496877184-6.jpg?resize=990:990' class='card-img-top' width='500' height='300'>");
          client.println("<div class='card-body'>");
          client.println("<h5 class='card-title'>BAÑO</h5>");
          client.println("<p class='card-text'>El control en tus manos.</p>");
          client.println("<a href='on5' class='btn btn-sucess'>ON</a>");
          client.println("<a href='off5' class='btn btn-sucess'>OFF</a>");
          client.println("</div></div></div>");
          //cochera
          client.println("<div class='card-body'>");
          client.println("<h5 class='card-title'>COCHERA</h5>");
          client.println("<p class='card-text'>El control en tus manos.</p>");
          client.println("<a href='on6' class='btn btn-sucess'>Abrir</a>");
          client.println("<a href='off6' class='btn btn-sucess'>Cerrar</a>");
          client.println("</div></div></div>");
          client.println("</div>");
          client.println("</div>");
          //client.println("<script src='https://code.jquery.com/jquery-3.3.1.slim.min.js' integrity='sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo' crossorigin='anonymous'></script>");
          //client.println("<script src='https://cdn.jsdelivr.net/npm/popper.js@1.14.7/dist/umd/popper.min.js' integrity='sha384-UO2eT0CpHqdSJQ6hJty5KVphtPhzWj9WO1clHTMGa3JDZwrnQq4sF86dIHNDz0W1' crossorigin='anonymous'></script>");
          //client.println("<script src='https://cdn.jsdelivr.net/npm/bootstrap@4.3.1/dist/js/bootstrap.min.js' integrity='sha384-JjSmVgyd0p3pXB1rRibZUAYoIIy6OrQ6VrjIEaFf/nJGzIxFDsf4x0xIM+B07jRM' crossorigin='anonymous'></script>");
          client.println("</body></html>");
          client.println("");
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
          if (strstr(linebuf, "GET /on1") > 0) {
            Serial.println("Cuarto 1 ON");
            digitalWrite(CUARTO1, HIGH);
            InsertarSQL("Luz_Recamara_Principal_Encendida");
          } else if (strstr(linebuf, "GET /off1") > 0) {
            Serial.println("Cuarto 1 OFF");
            digitalWrite(CUARTO1, LOW);
            InsertarSQL("Luz_Recamara_Principal_Apagada");
          } else if (strstr(linebuf, "GET /on2") > 0) {
            Serial.println("Cuarto 2 ON");
            digitalWrite(CUARTO2, HIGH);
            InsertarSQL("Luz_Recamara_Secundaria_Encendida");
          } else if (strstr(linebuf, "GET /off2") > 0) {
            Serial.println("Cuarto 2 OFF");
            digitalWrite(CUARTO2, LOW);
            InsertarSQL("Luz_Recamara_Secundaria_Apagada");
          } else if (strstr(linebuf, "GET /on3") > 0) {
            Serial.println("Cocina ON");
            digitalWrite(COCINA, HIGH);
            InsertarSQL("Luz_Cocina_Encendida");
          } else if (strstr(linebuf, "GET /off3") > 0) {
            Serial.println("Cocina OFF");
            digitalWrite(COCINA, LOW);
            InsertarSQL("Luz_Cocina_Apagada");
          } else if (strstr(linebuf, "GET /on4") > 0) {
            Serial.println("Sala ON");
            digitalWrite(SALA, HIGH);
            InsertarSQL("Luz_Sala_Encendida");
          } else if (strstr(linebuf, "GET /off4") > 0) {
            Serial.println("Sala OFF");
            digitalWrite(SALA, LOW);
            InsertarSQL("Luz_Sala_Apagada");
          } else if (strstr(linebuf, "GET /on5") > 0) {
            Serial.println("Baño ON");
            digitalWrite(BANO, HIGH);
            InsertarSQL("Luz_Baño_Encendida");
          } else if (strstr(linebuf, "GET /off5") > 0) {
            Serial.println("Baño OFF");
            digitalWrite(BANO, LOW);
            InsertarSQL("Luz_Baño_Apagada");
          } else if (strstr(linebuf, "GET /on6") > 0) {
            motor.drive(-16, 1000); //velocidad y tiempo para abrir el motor
            InsertarSQL("Garage_Abierto");
          } else if (strstr(linebuf, "GET /off6") > 0) {
            motor.drive(16, 1000); //velocidad y tiempo para cerrar el motor
            InsertarSQL("Garage_Cerrado");
          }
          currentLineIsBlank = true;
          memset(linebuf, 0, sizeof(linebuf));
          charcount = 0;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    //delay(1);
    //client.stop();
    //Serial.println("Cliente desconectado");
  }
  /* sensor pir */
  val = digitalRead(sensorPIR);  //Lee el valor de la variable (val)
  if (val == HIGH)               //Si detecta que hay movimiento manda activar el led
  {
    digitalWrite(PIRled, HIGH);
    //Serial.println("hay movimiento");
    //delay(3000);
    InsertarSQL("Movimiento_En_La_Entrada");


  } else  //Si la condición anterior no se cumple manda apagar el led
  {
    digitalWrite(PIRled, LOW);
    //Serial.println("no hay movimiento");
  }
}
