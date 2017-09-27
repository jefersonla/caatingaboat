#include <avr/wdt.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define ACELERADOR_DIREITA 5
#define ACELERADOR_ESQUERDA 6

//  Definição das entradas para os motores de cada lado, observem que alguns robôs precisaram ter os pinos trocados devido aos módulos de comunicação que utilizam.
//  Vejam os 4 fios unidos da ponte H que são conectados no Arduino para saberem o padrão do robô que vocês tem em mãos.
#define DIRECAO_DIREITA_1 7   //4
#define DIRECAO_DIREITA_2 8   //7

#define DIRECAO_ESQUERDA_1 9  //8
#define DIRECAO_ESQUERDA_2 10 //9

#define IR_PARA_FRENTE_DIREITA() do { digitalWrite(DIRECAO_DIREITA_1, HIGH); digitalWrite(DIRECAO_DIREITA_2, LOW); } while(false)
#define IR_PARA_FRENTE_ESQUERDA() do { digitalWrite(DIRECAO_ESQUERDA_1, HIGH); digitalWrite(DIRECAO_ESQUERDA_2, LOW); } while(false)
#define IR_PARA_FRENTE() do { IR_PARA_FRENTE_DIREITA(); IR_PARA_FRENTE_ESQUERDA(); } while(false)

#define IR_PARA_TRAS_DIREITA() do { digitalWrite(DIRECAO_DIREITA_1, LOW); digitalWrite(DIRECAO_DIREITA_2, HIGH); } while(false)
#define IR_PARA_TRAS_ESQUERDA() do { digitalWrite(DIRECAO_ESQUERDA_1, LOW); digitalWrite(DIRECAO_ESQUERDA_2, HIGH); } while(false)
#define IR_PARA_TRAS() do { IR_PARA_TRAS_DIREITA(); IR_PARA_TRAS_ESQUERDA(); } while(false)

#define ACELERA_DIREITA(VELOCIDADE) do { pwmDireita = VELOCIDADE; analogWrite(ACELERADOR_DIREITA, VELOCIDADE); } while(false)
#define ACELERA_ESQUERDA(VELOCIDADE) do { pwmEsquerda = VELOCIDADE; analogWrite(ACELERADOR_ESQUERDA, VELOCIDADE); } while(false)
#define ACELERA(VELOCIDADE) do { ACELERA_DIREITA(VELOCIDADE); ACELERA_ESQUERDA(VELOCIDADE); } while(false)

#define FREIO_DIREITA() { ACELERA_DIREITA(0); digitalWrite(DIRECAO_DIREITA_1, LOW); digitalWrite(DIRECAO_DIREITA_2, LOW); } while(false)
#define FREIO_ESQUERDA() { ACELERA_ESQUERDA(0); digitalWrite(DIRECAO_ESQUERDA_1, LOW); digitalWrite(DIRECAO_ESQUERDA_2, LOW); } while(false)
#define FREIO() do { FREIO_DIREITA(); FREIO_ESQUERDA(); } while(false)

// Velocidade armazenada PWM
volatile int pwmDireita = 0;
volatile int pwmEsquerda = 0;

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
  pinMode(DIRECAO_DIREITA_1, OUTPUT);
  pinMode(DIRECAO_DIREITA_2, OUTPUT);
  pinMode(DIRECAO_ESQUERDA_1, OUTPUT);
  pinMode(DIRECAO_ESQUERDA_2, OUTPUT);

  // Inicializa os aceleradores
  pinMode(ACELERADOR_DIREITA, OUTPUT);
  pinMode(ACELERADOR_ESQUERDA, OUTPUT);

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

    // Salva as velocidades
    int velocidadeDireita = velocidadePwm;
    int velocidadeEsquerda = velocidadePwm;

    if (lado > 0) {
      // Gira Direita
      velocidadeEsquerda -= ((double)abs(velocidade) / 99.0) * map(lado, 0, 99, 0, 255);
    } else if (lado < 0) {
      // Gira Esquerda
      velocidadeDireita -= ((double)abs(velocidade) / 99.0) * map(lado, 0, -99, 0, 255);
    }

    // Define se o movimento do barco é para trás ou para frente
    if (velocidade > 0) {
      IR_PARA_FRENTE();
    } else {
      IR_PARA_TRAS();
    }

    // Atualiza as velocidades
    ACELERA_DIREITA(velocidadeDireita);
    ACELERA_ESQUERDA(velocidadeEsquerda);
  }
}
