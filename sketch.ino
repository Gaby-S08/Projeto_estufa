#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"

// =====================================================
// AGROSENSE - MINI-ESTUFA IoT
// ESP32 + DHT22 + LDR + LEDs + Buzzer
// =====================================================

// -------------------- Wi-Fi ---------------------------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Opcional: coloque sua Write API Key do ThingSpeak.
// Se deixar o texto abaixo, o projeto continua funcionando
// normalmente no Wokwi, apenas não enviará para a nuvem.
const char* THINGSPEAK_API_KEY = "COLOQUE_SUA_WRITE_API_KEY";
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";

// -------------------- Pinos ---------------------------
const int PIN_DHT = 15;
const int PIN_LDR = 34;
const int PIN_LED_VERDE = 2;
const int PIN_LED_VERMELHO = 4;
const int PIN_BUZZER = 5;

DHTesp dht;

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_CLOUD = 15000;

// -------------------- Limites -------------------------
const float TEMP_MIN = 18.0;
const float TEMP_MAX = 30.0;
const float HUM_MIN = 40.0;
const float HUM_MAX = 80.0;
const float LUZ_MIN = 30.0;

// -------------------------------------------------------
void conectarWiFi() {
  Serial.println();
  Serial.print("Conectando ao Wi-Fi ");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tentativas = 0;

  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Nao foi possivel conectar ao Wi-Fi.");
  }
}

// -------------------------------------------------------
float lerLuminosidade() {
  int leitura = analogRead(PIN_LDR);

  // O modulo Wokwi entrega AO como leitura analogica.
  // Transformamos a leitura em uma escala didatica de 0 a 100%.
  float luminosidade = map(leitura, 0, 4095, 0, 100);
  luminosidade = constrain(luminosidade, 0, 100);

  return luminosidade;
}

// -------------------------------------------------------
void controlarAlerta(bool alerta) {
  if (alerta) {
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_VERMELHO, HIGH);

    // Buzzer ativo durante o alerta.
    tone(PIN_BUZZER, 1000);
  } else {
    digitalWrite(PIN_LED_VERDE, HIGH);
    digitalWrite(PIN_LED_VERMELHO, LOW);

    noTone(PIN_BUZZER);
  }
}

// -------------------------------------------------------
void enviarThingSpeak(float temperatura,
                      float umidade,
                      float luminosidade,
                      bool alerta) {

  // Não tenta enviar se a API Key ainda não foi configurada.
  if (String(THINGSPEAK_API_KEY) == "COLOQUE_SUA_WRITE_API_KEY") {
    Serial.println("ThingSpeak: API Key ainda nao configurada.");
    return;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ThingSpeak: Wi-Fi desconectado.");
    return;
  }

  HTTPClient http;

  String url = String(THINGSPEAK_URL);
  url += "?api_key=";
  url += THINGSPEAK_API_KEY;
  url += "&field1=";
  url += String(temperatura, 2);
  url += "&field2=";
  url += String(umidade, 2);
  url += "&field3=";
  url += String(luminosidade, 2);
  url += "&field4=";
  url += String(alerta ? 1 : 0);

  http.begin(url);

  int codigo = http.GET();

  Serial.print("ThingSpeak HTTP: ");
  Serial.println(codigo);

  http.end();
}

// -------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_VERMELHO, LOW);
  noTone(PIN_BUZZER);

  // Inicializa o DHT22 no GPIO 15.
  dht.setup(PIN_DHT, DHTesp::DHT22);

  Serial.println();
  Serial.println("================================");
  Serial.println("       AGROSENSE - IoT");
  Serial.println("   MINI-ESTUFA INTELIGENTE");
  Serial.println("================================");

  conectarWiFi();
}

// -------------------------------------------------------
void loop() {

  TempAndHumidity dados = dht.getTempAndHumidity();

  // Verifica se a leitura do DHT22 e valida.
  if (isnan(dados.temperature) || isnan(dados.humidity)) {
    Serial.println("ERRO: nao foi possivel ler o DHT22.");
    digitalWrite(PIN_LED_VERDE, LOW);
    digitalWrite(PIN_LED_VERMELHO, HIGH);
    tone(PIN_BUZZER, 1200);
    delay(2000);
    return;
  }

  float temperatura = dados.temperature;
  float umidade = dados.humidity;
  float luminosidade = lerLuminosidade();

  bool alertaTemperatura =
    temperatura < TEMP_MIN || temperatura > TEMP_MAX;

  bool alertaUmidade =
    umidade < HUM_MIN || umidade > HUM_MAX;

  bool alertaLuminosidade =
    luminosidade < LUZ_MIN;

  bool alerta =
    alertaTemperatura ||
    alertaUmidade ||
    alertaLuminosidade;

  controlarAlerta(alerta);

  Serial.println();
  Serial.println("--------------------------------");
  Serial.printf("Temperatura : %.2f C\n", temperatura);
  Serial.printf("Umidade     : %.2f %%\n", umidade);
  Serial.printf("Luminosidade: %.2f %%\n", luminosidade);
  Serial.print("Status      : ");

  if (alerta) {
    Serial.println("ALERTA 🔴");
  } else {
    Serial.println("NORMAL 🟢");
  }

  Serial.print("Motivos     : ");

  if (!alerta) {
    Serial.println("nenhum");
  } else {
    if (alertaTemperatura) Serial.print("temperatura ");
    if (alertaUmidade) Serial.print("umidade ");
    if (alertaLuminosidade) Serial.print("luminosidade ");
    Serial.println();
  }

  // Envio para ThingSpeak a cada 15 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_CLOUD) {

    if (WiFi.status() != WL_CONNECTED) {
      conectarWiFi();
    }

    enviarThingSpeak(
      temperatura,
      umidade,
      luminosidade,
      alerta
    );

    ultimoEnvio = millis();
  }

  delay(2000);
}
