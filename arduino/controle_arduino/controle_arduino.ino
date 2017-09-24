#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>

// Pinos do controle
#define CONTROLE_ACELERACAO A0
#define CONTROLE_DIRECAO A1
#define CONTROLE_RESET 2

// Numero de interrupções
#define NUMERO_INTERVALO 10

// Numero de checagens
#define NUMERO_CHECAGENS 2

// Frequência de cada intervalo
#define FREQUENCIA_INTERVALO (1000000 / (NUMERO_INTERVALO * NUMERO_CHECAGENS))

// Estado do led
boolean estadoLed = LOW;

// Centro do controle
int centroAceleracao = 0;
int centroDirecao = 0;

// Mensagem
char mensagem[9];

void setup() {
  // Configura a comunicação Serial
  Serial.begin(9600);

  // Ativa o LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Pino do reset do controller
  pinMode(CONTROLE_RESET, INPUT_PULLUP);

  // Armazena o millis atual
  unsigned long millisAnteriorLed = millis();
  unsigned long millisAnteriorMensagem = millis();

  // Tenta encontrar o receptor
  while (true) {
    // Atualiza os tempos
    unsigned long millisAtualLed = millis();
    unsigned long millisAtualMensagem = millis();

    // Enviamos a mensagem em intervalos de 100ms
    if (millisAtualMensagem - millisAnteriorMensagem >= 100) {
      // Salva o novo tempo
      millisAnteriorMensagem = millisAtualMensagem;

      // Enviamos vários '*' enquanto não recebermos um '$'
      Serial.write('*');
    }

    // Se encontrarmos um '$' então encontramos um receptor
    if (Serial.available() > 0 && Serial.read() == '$') {
      break;
    }

    // Pisca o led em intervalos de 500 em 500ms
    if (millisAtualLed - millisAnteriorLed >= 500) {
      // Salva o novo tempo
      millisAnteriorLed = millisAtualLed;

      // Inverte o estado
      estadoLed = !estadoLed;

      // Pisca o led
      digitalWrite(LED_BUILTIN, estadoLed);
    }
  }

  // Deixa o led acesso
  digitalWrite(LED_BUILTIN, HIGH);

  // Ativa o watchdog em 1S
  wdt_enable(WDTO_1S);

  // Numero de intervalos
  Timer1.initialize(FREQUENCIA_INTERVALO);
  Timer1.attachInterrupt(processaIntervalos);

  // Interupção do reset do controle
  attachInterrupt(digitalPinToInterrupt(CONTROLE_RESET), resetaCentro, FALLING);

  // Salva o centro do controle
  centroAceleracao = analogRead(CONTROLE_ACELERACAO);
  centroDirecao = analogRead(CONTROLE_DIRECAO);

  // Inicializa a parte constante da mensagem
  mensagem[0] = '>';
  mensagem[4] = ',';
  mensagem[8] = '\0';
}

// Numero de intervalos
volatile unsigned long numeroIntervalos = 0;

// Processa os intervalos
void processaIntervalos() {
  numeroIntervalos++;

  // Executa a leitura duas vezes menos que o recebimento dos ack's
  if (numeroIntervalos & 1) {
    enviaComandos();
  }

  // Recebe o ack
  recebeAck();
}

// Recebe o acknowledge do receptor e reseta o contador do watchdog
void recebeAck() {
  if (Serial.available() > 0 && Serial.read() == '#') {
    wdt_reset();
  }
}

// Converte de logaritmico para linear
int converteLogLin(int data) {
  // TODO
  return data;
}

// Envia os comandos dentro dos intervalos especificados
void enviaComandos() {
  // Pega os valores do joystick
  int aceleracao = analogRead(CONTROLE_ACELERACAO);
  int direcao = analogRead(CONTROLE_DIRECAO);

  // Mapeia o range da velocidade
  int mapAceleracao = 0;

  // Converte a aceleracao analógica para a aceleração lógica
  if (aceleracao <= centroAceleracao) {
    mapAceleracao = map(converteLogLin(aceleracao), 0, converteLogLin(centroAceleracao), -99, 0);
  } else {
    mapAceleracao = map(converteLogLin(aceleracao), converteLogLin(centroAceleracao), 1023, 0, 99);
  }

  // Mapeia o range da direção
  int mapDirecao = 0;

  if (direcao <= centroDirecao) {
    mapDirecao = map(converteLogLin(direcao), 0, converteLogLin(centroDirecao), -99, 0);
  } else {
    mapDirecao = map(converteLogLin(direcao), converteLogLin(centroDirecao), 1023, 0, 99);
  }

  // Mensagem temporaria vai armazenar o numero a ser convertido
  char temp[4];

  // Prepara a direção para a mensagem
  itoa(abs(mapDirecao), temp, 10);

  // Adiciona o '-' a mensagem
  if (mapDirecao < 0) {
    mensagem[1] = '-';
  } else {
    mensagem[1] = ' ';
  }

  // Copia o numero convertido para a mensagem
  if (abs(mapDirecao) < 10) {
    mensagem[2] = ' ';
    mensagem[3] = temp[0];
  } else {
    mensagem[2] = temp[0];
    mensagem[3] = temp[1];
  }

  // Prepara a velocidade para a mensagem
  itoa(abs(mapAceleracao), temp, 10);

  // Adiciona o '-' a mensagem
  if (mapAceleracao < 0) {
    mensagem[5] = '-';
  } else {
    mensagem[5] = ' ';
  }

  // Adiciona os primeiros numeros a mensagem
  if (abs(mapAceleracao) < 10) {
    mensagem[6] = ' ';
    mensagem[7] = temp[0];
  } else {
    mensagem[6] = temp[0];
    mensagem[7] = temp[1];
  }

  // Envia a mensagem pela serial
  Serial.println(mensagem);
}

// Reseta o centro do controle
void resetaCentro() {
  centroAceleracao = analogRead(CONTROLE_ACELERACAO);
  centroDirecao = analogRead(CONTROLE_DIRECAO);
}

void loop() {}
