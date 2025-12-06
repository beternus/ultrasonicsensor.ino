// PROJETO GB - INSTRUMENTAÇÃO ELETROELETRÔNICA
// UNISINOS 2025/2
// PEDRO BRANDELLI E BERNARDO TERNUS

#include <LiquidCrystal.h>
#include <Ultrasonic.h>

// --- LCD sem I2C (RS, E, D4, D5, D6, D7) ---
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// --- Sensor ultrassônico ---
const int trigPin = 12;
const int echoPin = 13;
Ultrasonic ultrasonic(12, 13);

// --- LEDs e buzzer ---
const int ledLow = 8;
const int ledHigh = 9;
const int buzzer = 10;

// --- Botão e potenciômetro ---
const int botao = A1;
const int pot = A0;

// --- Variáveis globais ---
int limiteInferior = 10;
int limiteSuperior = 20;
int modo = 0;  // 0 = set inferior, 1 = set superior, 2 = medir

// --- Controle de botão ---
bool botaoPressionado = false;

// --- Controle do buzzer ---
unsigned long ultimoBeep = 0;
bool beepAtivo = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ledLow, OUTPUT);
  pinMode(ledHigh, OUTPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(botao, INPUT_PULLUP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Limite Inferior:");
  delay(1000);
}

void loop() {
  // --- Leitura do botão com detecção de borda ---
  bool estadoBotao = (digitalRead(botao) == LOW);

  if (estadoBotao && !botaoPressionado) {
    botaoPressionado = true;

    if (modo == 0) {
      modo = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Set Limite Superior:");
      delay(500);
    }
    else if (modo == 1) {
      if (limiteSuperior < limiteInferior)
        limiteSuperior = limiteInferior + 5;

      modo = 2;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Medida:");
      delay(500);
    }
  }

  if (!estadoBotao)
    botaoPressionado = false;

  // --- Modo de configuração dos limites ---
  if (modo == 0 || modo == 1) {
    int valor = map(analogRead(pot), 0, 1023, 2, 100);

    if (modo == 0)
      limiteInferior = valor;
    else
      limiteSuperior = valor;

    lcd.setCursor(0, 1);
    lcd.print("                ");

    lcd.setCursor(0, 1);
    lcd.print(valor);
    lcd.print(" cm");

    delay(150);
    return;
  }

  // --- Modo de medição ---
  if (modo == 2) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);
    float distance = duration * 0.034 / 2;

    float distanceAj = 1.0287 * distance + 0.8997;

    Serial.println(distanceAj);

    lcd.setCursor(0, 0);
    lcd.print("Medida:       ");

    lcd.setCursor(9, 0);
    lcd.print(distanceAj);
    lcd.print("cm ");

    lcd.setCursor(0, 1);
    lcd.print("I:");
    lcd.print(limiteInferior);
    lcd.print("  S:");
    lcd.print(limiteSuperior);
    lcd.print("   ");

    // --- LEDs por limites ---
    if (distanceAj < limiteInferior) {
      digitalWrite(ledLow, HIGH);
      digitalWrite(ledHigh, LOW);
    }
    else if (distanceAj > limiteSuperior) {
      digitalWrite(ledHigh, HIGH);
      digitalWrite(ledLow, LOW);
    }
    else {
      digitalWrite(ledLow, LOW);
      digitalWrite(ledHigh, LOW);
    }

    sensorDeRe(distanceAj);

    delay(50);
  }
}

// =====================================================================
//  Função de buzzer estilo sensor de ré
// =====================================================================
void sensorDeRe(float distancia) {
  unsigned long agora = millis();

  if (distancia > 20 || distancia <= 0) {
    noTone(buzzer);
    beepAtivo = false;
    return;
  }

  if (distancia <= 5) {
    tone(buzzer, 2000);
    return;
  }

  int intervalo = map(distancia, 5, 20, 100, 600);

  if (!beepAtivo && (agora - ultimoBeep >= intervalo)) {
    tone(buzzer, 2000);
    beepAtivo = true;
    ultimoBeep = agora;
  }
  else if (beepAtivo && (agora - ultimoBeep >= 100)) {
    noTone(buzzer);
    beepAtivo = false;
  }
}
