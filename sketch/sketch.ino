/*
  Codigo para o trabalho Neosymbiota lucens 
  Descricao: Esse programa controla
  * Um sensor ultrassonico para medir distancia de obstaculo (e desviar)
  * Microfone para receber chamado (farol da atração). A frequencia padrao do chamado sera 3000 Hz.
  * Motores que serao acionados para desviar de obstaculos e ir em direcao ao chamado
  * Bluetooth para controle remoto via celular  
*/

#include "arduinoFFT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Dabble.h>
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <AFMotor.h>

// Expansor de pinos GPIO
const int pcfAddress = 0x3F;

/****************************************************
 * Sensor ultrassônico JSN-SR04T / AJ-SR04M         *
 * a prova d'agua para medir distancia do obstáculo *
 *****************************************************/
#define ECHO 11 //  Pino ECHO do sensor esta conectado ao pino 11 do arduino.
#define TRIG 12 // Pino TRIG do sensor esta conectado ao pino 12 do arduino.
int dist = 0;   // distancia medida pelo sensor.
long tempo = 0; // intervalo de tempo da reflezão da onda no sensor.

/***************************
 * Controle dos motores DC *
 ***************************/
AF_DCMotor motor3(3);  // motor da esquerda
AF_DCMotor motor1(1);  // motor da direita

/**********************************************
 * Microfone Módulo Amplificador De Som LM393 *
 *********************************************/
#define MIC1_ANALOG A0
#define MIC1_DIGITAL 7
// A partir do sinal capturado pelo microfone, sera feita a transformada FFT (Fast Fourier Transform)
// para analisar as frequencias e verificar se tem alto sinal na frequencia de 3000 Hz
// Definicao dos parametros da transformada:
#define SAMPLES 64
#define SAMPLING_FREQ 10000
int SAMPLING_PERIOD_US = round(1000000 * (1.0 / SAMPLING_FREQ));
// Objeto da transformada FFT e dados.
double Real1[SAMPLES], Imag1[SAMPLES];
ArduinoFFT<double> FFT1 = ArduinoFFT<double>(Real1, Imag1, SAMPLES, SAMPLING_FREQ, true);
float targetFreq = 3000.0;  // frequencia de interesse.
float magnitudeThreshold = 73.0;

void setup() {
	// put your setup code here, to run once:
	pinMode(ECHO, INPUT);   // pino ECHO do sensor esta configurado como entrada recebendo o sinal refletido.
	pinMode(TRIG, OUTPUT);  // pino TRIG do sensor esta configurado como saida, acionando a emissao da onda.
	Serial.begin(9600);
	Dabble.begin(9600);
	// microfone
	pinMode(MIC1_ANALOG, INPUT);
	pinMode(MIC1_DIGITAL, INPUT);
	// inicializacao dos motores
	motor3.setSpeed(0);
	motor1.setSpeed(0);
}

void loop() {
	// put your main code here, to run repeatedly:
	medeDistancia();
	ouveChamado();
	leControle();
}


void medeDistancia() {
  digitalWrite(TRIG, LOW);      // define pino TRIG como LOW para garantir que comece como zero.
  delay(10);                     // espera 4 milissegundos antes de enviar o pulso.
  digitalWrite(TRIG, HIGH);     // define pino TRIG como HIGH para enviar a onda ultrassonica.
  delay(10);                    // espera 10 milissegundos para garantir que onda seja emitida.
  digitalWrite(TRIG, LOW);      // define pino TRIG como LOW para finalizar emissao da onda.
  tempo = pulseIn(ECHO, HIGH);  // pino ECHO e ativado quando o sinal refletido e recebido. pulseIn mede o tempo em que o pino ECHO esta em HIGH.
  dist = (tempo * 0.034) / 2;    // calcula a distancia, considerando o tempo de ida e volta da onda.
  Serial.print("Distancia medida: ");
  Serial.print(dist);
  Serial.println(" cm");
  delay(500);
}


// mapeamento da frequencia para FFT bin
int frequencyToBin(float freq, float samplingFreq, int numSamples) {
	return round((freq * numSamples) / samplingFreq);
}

void ouveChamado() {
	long microseconds = micros();  // comeca tempo para coletar amostras.
	for (int i = 0; i < SAMPLES; i++) {
		Real1[i] = analogRead(MIC1_ANALOG);  // amostra do MIC1;
		Serial.println("Real");
		Serial.println(Real1[i]);
		Imag1[i] = 0;
		// espera tempo de coleta da amostra terminar.
    while ((micros() - microseconds) < SAMPLING_PERIOD_US);
    microseconds += SAMPLING_PERIOD_US;
	}
	// FFT.
	FFT1.windowing(FFTWindow::Hamming, FFTDirection::Forward);
	FFT1.compute(FFTDirection::Forward);
	FFT1.complexToMagnitude();
	// obtem as magnitudes para a frequencia de interesse.
	int targetBin = frequencyToBin(targetFreq, SAMPLING_FREQ, SAMPLES);
	float freqMag = Real1[targetBin];
	if (freqMag > magnitudeThreshold) {
		Serial.println("ANALOG: Tem alguem chamando");
	// 		motor3.setSpeed(255);
	// 		motor3.run(FORWARD);
	// 		motor1.setSpeed(255);
	// 		motor1.run(FORWARD);
	} else {
	// 		motor3.setSpeed(0);
	// 		motor3.run(FORWARD);
	// 		motor1.setSpeed(0);
	// 		motor1.run(FORWARD);
		Serial.println(freqMag);
		Serial.println("ANALOG: Ninguem chamando");
	}
	delay(100);
	bool digital = digitalRead(MIC1_DIGITAL);
	if (digital == HIGH) {
		Serial.println("DIGITAL HIGH: Tem alguem chamando");
// 		motor3.setSpeed(255);
// 		motor3.run(FORWARD);
// 		motor1.setSpeed(255);
// 		motor1.run(FORWARD);
	} else {
		Serial.println("digital low: ninguem chamando");
	}
	delay(500);
}

void leControle() {
	Dabble.processInput();
	if (GamePad.isPressed(0)) {
		// Serial.println("Pressionou 0");
		motor3.setSpeed(255);
		motor3.run(BACKWARD);
		motor1.setSpeed(255);
		motor1.run(FORWARD);
		return;
	} 
	// pressionou seta para cima
	if (GamePad.isPressed(1)) {
		// Serial.println("Pressionou seta para cima (1)");
		// motor3.setSpeed(255);
		// motor3.run(FORWARD);
		// motor1.setSpeed(255);
		// motor1.run(BACKWARD);
		return;
	} 
	// pressionou seta para baixo
	if (GamePad.isPressed(2)) {
	// 	Serial.println("Pressionou 2");
	// 	motor3.setSpeed(0);
	// 	motor3.run(BACKWARD);
	// 	motor1.setSpeed(255);
	// 	motor1.run(FORWARD);
		return;
	} 
	// pressionou seta para esquerda
	if (GamePad.isPressed(3)) {
		Serial.println("Pressionou seta para esquerda (3)");
		// motor3.setSpeed(255);
		// motor3.run(BACKWARD);
		// motor1.setSpeed(0);
		// motor1.run(FORWARD);
		return;
	} 
	// pressionou botao select
	if (GamePad.isPressed(9)) {
		Serial.println("Pressionou botão select (9)");
		// motor3.setSpeed(255);
		// motor3.run(FORWARD);
		// motor1.setSpeed(255);
		// motor1.run(FORWARD);
		return;
	} 
	// pressionou quadrado
	if (GamePad.isPressed(7)) {
		Serial.println("Pressionou quadrado (7)");
	// 	motor3.setSpeed(255);
	// 	motor3.run(BACKWARD);
	// 	motor1.setSpeed(255);
	// 	motor1.run(BACKWARD);
		return;
	} 
	motor3.setSpeed(0);
	motor3.run(FORWARD);
	// motor1.setSpeed(0);
	// motor1.run(FORWARD);
	// Serial.println("Nenhum botao apertado");
}