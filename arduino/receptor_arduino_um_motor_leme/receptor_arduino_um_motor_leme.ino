#include <avr/wdt.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define ACELERADOR_MOTOR 6

#define DIRECAO_MOTOR_1 7
#define DIRECAO_MOTOR_2 8

#define IR_PARA_FRENTE() do { digitalWrite(DIRECAO_MOTOR_1, HIGH); digitalWrite(DIRECAO_MOTOR_2, LOW); } while(false)
#define IR_PARA_TRAS() do { digitalWrite(DIRECAO_MOTOR_1, LOW); digitalWrite(DIRECAO_MOTOR_2, HIGH); } while(false)

#define ACELERA(VELOCIDADE) do { pwmMotor = VELOCIDADE; analogWrite(ACELERADOR_MOTOR, VELOCIDADE); } while(false)
#define FREIO() { ACELERA(0); digitalWrite(DIRECAO_MOTOR_1, LOW); digitalWrite(DIRECAO_MOTOR_2, LOW); } while(false)

// Velocidade armazenada PWM
volatile int pwmMotor = 0;

// Tempo
volatile unsigned long millisAnterior = 0;

// Estado do led
boolean estadoLed = HIGH;

// Mensagens de movimento
volatile char ladoStr[4];
volatile char veloStr[4];

// Leme
Servo lemeServo;

// Debug por serial
SoftwareSerial debug(3, 2);

// Ativa o debug por serial
#define SOFTWARE_DEBUG

void setup() {
  // Configuração da Comunicação Serial
  Serial.begin(9600);
  debug.begin(9600);

  // Configuração dos pinos da Ponte H
  pinMode(DIRECAO_MOTOR_1, OUTPUT);
  pinMode(DIRECAO_MOTOR_1, OUTPUT);

  // Inicializa os aceleradores
  pinMode(ACELERADOR_MOTOR, OUTPUT);

  // Ativa o watchdog em 1S
  wdt_enable(WDTO_1S);

  // Ativa o LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Adiciona o \0 padrão
  ladoStr[3] = '\0';
  veloStr[3] = '\0';

#ifdef SOFTWARE_DEBUG
  debug.println("Iniciando receptor...");
#endif

  // Configur o leme para a posição inicial
  lemeServo.attach(11);
  lemeServo.write(90);

  // Tenta encontrar o controle
  while (true) {
    // Se encontrarmos um * enviamos de volta para o controle o $ e conectamos
    if (Serial.available() > 0 && Serial.read() == '*') {
      Serial.write('$');
      delay(5);
      Serial.write('$');
      delay(5);
      break;
    }

    // Atualiza o tempo para piscar o led sem delay
    unsigned long millisAtual = millis();

    // Pisca o led em intervalos de 500 em 500ms
    if (millisAtual - millisAnterior >= 500) {
      // Salva o novo tempo
      millisAnterior = millisAtual;

      // Inverte o estado
      estadoLed = !estadoLed;

      // Pisca o led
      digitalWrite(LED_BUILTIN, estadoLed);
    }
  }

  // Reseta o watchdog
  wdt_reset();

  // Ativa ir para frente
  IR_PARA_FRENTE();

  // Liga o LED para indicar que está funcionando
  digitalWrite(LED_BUILTIN, HIGH);

#ifdef SOFTWARE_DEBUG
  debug.println("Receptor Inicializado!");
#endif
}

//#define DEBUG

// Espera por caractere da serial
#define esperaCaractereSerial() while (!(Serial.available() > 0))

// Ignora um caracter válido
#define ignoraCaracterSerial() do{ esperaCaractereSerial(); Serial.read(); } while(false)

// Pega caractere válido da serial e põe em VAR
#define pegaCaractereSerialPara(VAR) do{ esperaCaractereSerial(); VAR = Serial.read(); } while(false)

void loop() {
  if (Serial.available() > 0 && Serial.read() == '>') {
    // Espera 5 millisegundos e lê a mensagem até o \n
    delay(50);

    // Lê o valor do lado
    for (int i = 0; i < 3; i++) {
      pegaCaractereSerialPara(ladoStr[i]);
    }

    // Ignora a ','
    ignoraCaracterSerial();

    // Lê a velocidade
    for (int i = 0; i < 3; i++) {
      pegaCaractereSerialPara(veloStr[i]);
    }

    // Ignora o '\n'
    ignoraCaracterSerial();

    // Reseta o timer para esperar pela próxima mensagem
    wdt_reset();

    // Envia o código de resposta para o controle
    Serial.write('#');
    delay(5);
    Serial.write('#');
    delay(5);

    // Converte os inteiros
    int lado = atoi(ladoStr);
    int velocidade = atoi(veloStr);

#ifdef DEBUG
    Serial.print("sid ");
    Serial.print(lado);
    Serial.print(", vel");
    Serial.println(velocidade);
#endif

#ifdef SOFTWARE_DEBUG
    debug.print(lado);
    debug.write(',');
    debug.println(velocidade);
#endif

    // Converte a velocidade para pwm
    int velocidadePwm = map(abs(velocidade), 0, 99, 0, 255);

    // Move o servomotor
    if (lado > 0) {
      // Gira Direita
      lemeServo.write(map(lado, 0, 99, 90, 0));
    } else if (lado < 0) {
      // Gira Esquerda
      lemeServo.write(map(lado, 0, -99, 180, 90));
    }

    // Define se o movimento do barco é para trás ou para frente
    if (velocidade > 0) {
      IR_PARA_FRENTE();
    } else {
      IR_PARA_TRAS();
    }

    // Atualiza as velocidades
    ACELERA(velocidadePwm);
  }
}
