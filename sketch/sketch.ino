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


/****************************************************
 * Sensor ultrassônico JSN-SR04T / AJ-SR04M         *
 * a prova d'agua para medir distancia do obstáculo *
 *****************************************************/
#define ECHO1 11 //  Pino ECHO1 do sensor esta conectado ao pino 11 do arduino.
#define TRIG1 12 // Pino TRIG1 do sensor esta conectado ao pino 12 do arduino.
#define ECHO2 5 //  Pino ECHO2 do sensor esta conectado ao pino 5 do arduino.
#define TRIG2 7 // Pino TRIG2 do sensor esta conectado ao pino 7 do arduino.
int dist1 = 0;   // distancia medida pelo sensor.
int dist2 = 0;   // distancia medida pelo sensor.
long tempo1 = 0; // intervalo de tempo1 da reflexão da onda no sensor.
long tempo2 = 0; // intervalo de tempo1 da reflexão da onda no sensor.
bool obstaculo1Proximo = false;
bool obstaculo2Proximo = false;

/***************************
 * Controle dos motores DC *
 ***************************/
#define MOTOR1_IN1 4
#define MOTOR1_IN2 8
#define MOTOR1_ENABLE 6

#define MOTOR2_IN1 9
#define MOTOR2_IN2 10
#define MOTOR2_ENABLE 13

/**********************************************
 * Microfone Módulo Amplificador De Som LM393 *
 *********************************************/
#define MIC1_ANALOG A0
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
float magnitudeThreshold = 96.0;
float previous = 0;

void setup() {
	// put your setup code here, to run once:
	pinMode(ECHO1, INPUT);   // pino ECHO1 do sensor esta configurado como entrada recebendo o sinal refletido.
	pinMode(TRIG1, OUTPUT);  // pino TRIG1 do sensor esta configurado como saida, acionando a emissao da onda.
	pinMode(ECHO2, INPUT);   // pino ECHO2 do sensor esta configurado como entrada recebendo o sinal refletido.
	pinMode(TRIG2, OUTPUT);  // pino TRIG2 do sensor esta configurado como saida, acionando a emissao da onda.
	Serial.begin(9600);
	Dabble.begin(9600);
	// microfone
	pinMode(MIC1_ANALOG, INPUT);
	// inicializacao dos motores
	pinMode(MOTOR1_IN1, INPUT);
	pinMode(MOTOR1_IN2, INPUT);
	pinMode(MOTOR1_ENABLE, INPUT);
	pinMode(MOTOR2_IN1, INPUT);
	pinMode(MOTOR2_IN2, INPUT);
	pinMode(MOTOR2_ENABLE, INPUT);
	desligaMotor1();
	desligaMotor2();
}

void loop() {
	// put your main code here, to run repeatedly:
	//medeDistancia();
	ouveChamado();
	//leControle();
}

void medeDistancia(){
	medeDistancia1();
	//medeDistancia2();
}


void medeDistancia1() {
  digitalWrite(TRIG1, LOW);      // define pino TRIG1 como LOW para garantir que comece como zero.
  delay(10);                     // espera 4 milissegundos antes de enviar o pulso.
  digitalWrite(TRIG1, HIGH);     // define pino TRIG1 como HIGH para enviar a onda ultrassonica.
  delay(10);                    // espera 10 milissegundos para garantir que onda seja emitida.
  digitalWrite(TRIG1, LOW);      // define pino TRIG1 como LOW para finalizar emissao da onda.
  tempo1 = pulseIn(ECHO1, HIGH);  // pino ECHO1 e ativado quando o sinal refletido e recebido. pulseIn mede o tempo1 em que o pino ECHO1 esta em HIGH.
  dist1 = (tempo1 * 0.034) / 2;    // calcula a dist1ancia, considerando o tempo1 de ida e volta da onda.
  Serial.print("distancia 1 medida: ");
  Serial.print(dist1);
  Serial.println(" cm");
	if (dist1 < 5) {
		obstaculo1Proximo = true;
		Serial.println("OBSTACULO 1 MUITO PROXIMO");
		ligaMotor1();
		paraFrenteMotor1();
	} else {
		obstaculo1Proximo = false;
		Serial.println("obstaculo 1 muito longe");
		desligaMotor1();
	}
  delay(500);
}


void medeDistancia2() {
  digitalWrite(TRIG2, LOW);      // define pino TRIG1 como LOW para garantir que comece como zero.
  delay(10);                     // espera 4 milissegundos antes de enviar o pulso.
  digitalWrite(TRIG2, HIGH);     // define pino TRIG1 como HIGH para enviar a onda ultrassonica.
  delay(10);                    // espera 10 milissegundos para garantir que onda seja emitida.
  digitalWrite(TRIG2, LOW);      // define pino TRIG1 como LOW para finalizar emissao da onda.
  tempo2 = pulseIn(ECHO2, HIGH);  // pino ECHO1 e ativado quando o sinal refletido e recebido. pulseIn mede o tempo1 em que o pino ECHO1 esta em HIGH.
  dist2 = (tempo2 * 0.034) / 2;    // calcula a dist1ancia, considerando o tempo1 de ida e volta da onda.
  Serial.print("distancia 2 medida: ");
  Serial.print(dist2);
  Serial.println(" cm");
	if (dist2 < 5) {
		obstaculo2Proximo = true;
		Serial.println("OBSTACULO 2 MUITO PROXIMO");
		ligaMotor2();
		paraFrenteMotor2();
	} else {
		obstaculo2Proximo = false;
		Serial.println("obstaculo 2 muito longe");
		desligaMotor2();
	}
  delay(500);
}


// mapeamento da frequencia para FFT bin
int frequencyToBin(float freq, float samplingFreq, int numSamples) {
	return round((freq * numSamples) / samplingFreq);
}

void ouveChamado() {
	long microseconds = micros();  // comeca tempo1 para coletar amostras.
	for (int i = 0; i < SAMPLES; i++) {
		Real1[i] = analogRead(MIC1_ANALOG);  // amostra do MIC1;
		// Serial.println("Real");
		// Serial.println(Real1[i]);
		Imag1[i] = 0;
		// espera tempo1 de coleta da amostra terminar.
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
	if ((freqMag+previous)/2 > magnitudeThreshold) {
		Serial.println(freqMag);
		Serial.println("ANALOG: TEM ALGUEM CHAMANDO");
		ligaMotor1();
		paraTrasMotor1();
	} else {
		Serial.println(freqMag);
		Serial.println("analog: ninguem chamando");
		if (!obstaculo1Proximo){
			desligaMotor1();
		}
		if (!obstaculo2Proximo) {
			desligaMotor2();
		}
	}
	previous = freqMag;
	delay(50);
}

void leControle() {
	Dabble.processInput();
	if (GamePad.isPressed(0)) {
		Serial.println("Pressionou 0");
		// motor3.setSpeed(255);
		// motor3.run(BACKWARD);
		ligaMotor1();
		paraFrenteMotor1();
		return;
	} 
	// pressionou seta para cima
	if (GamePad.isPressed(1)) {
		Serial.println("Pressionou seta para cima (1)");
		// motor3.setSpeed(255);
		// motor3.run(FORWARD);
		ligaMotor1();
		paraTrasMotor1();
		return;
	} 
	// pressionou seta para baixo
	if (GamePad.isPressed(2)) {
		Serial.println("Pressionou seta para baixo (2)");
	// 	motor3.setSpeed(0);
	// 	motor3.run(BACKWARD);
		ligaMotor1();
		paraFrenteMotor1();
		return;
	} 
	// pressionou seta para esquerda
	if (GamePad.isPressed(3)) {
		Serial.println("Pressionou seta para esquerda (3)");
		if (!obstaculo1Proximo) {
			desligaMotor1();
		}
		ligaMotor2();
		paraTrasMotor2();
		return;
	} 
	// pressionou botao select
	if (GamePad.isPressed(9)) {
		Serial.println("Pressionou botão select (9)");
		ligaMotor1();
		paraFrenteMotor1();
		ligaMotor2();
		paraFrenteMotor2();
		return;
	} 
	// pressionou quadrado
	if (GamePad.isPressed(7)) {
		Serial.println("Pressionou quadrado (7)");
		ligaMotor1();
		paraTrasMotor1();
		ligaMotor2();
		paraTrasMotor2();
		return;
	} 
	Serial.println("Nenhum botao apertado");
}

void ligaMotor1() {
	digitalWrite(MOTOR1_ENABLE, HIGH);
}

void ligaMotor2() {
	digitalWrite(MOTOR2_ENABLE, HIGH);
}


void desligaMotor1() {
	digitalWrite(MOTOR1_IN1, LOW);
	digitalWrite(MOTOR1_IN2, LOW);
	digitalWrite(MOTOR1_ENABLE, LOW);
}

void desligaMotor2() {
	digitalWrite(MOTOR2_IN1, LOW);
	digitalWrite(MOTOR2_IN2, LOW);
	digitalWrite(MOTOR2_ENABLE, LOW);
}

void paraFrenteMotor1() {
	digitalWrite(MOTOR1_IN1, HIGH);
	digitalWrite(MOTOR1_IN2, LOW);
}

void paraFrenteMotor2() {
	digitalWrite(MOTOR2_IN1, HIGH);
	digitalWrite(MOTOR2_IN2, LOW);
}

void paraTrasMotor1() {
	digitalWrite(MOTOR1_IN1, LOW);
	digitalWrite(MOTOR1_IN2, HIGH);
}

void paraTrasMotor2() {
	digitalWrite(MOTOR2_IN1, LOW);
	digitalWrite(MOTOR2_IN2, HIGH);
}


