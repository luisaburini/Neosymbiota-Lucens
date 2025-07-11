
/*
  Codigo para o trabalho Neosymbiota lucens 
  Descricao: Esse programa controla
  * Um sensor ultrassonico para medir distancia de obstaculo (e desviar)
  * Microfone para receber chamado (farol da atração). A frequencia padrao do chamado sera 3000 Hz.
  * Motores que serao acionados para desviar de obstaculos e ir em direcao ao chamado
  Extras: Bluetooth para controle remoto via celular => 
*/

#include "arduinoFFT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Dabble.h>
#include <Servo.h>

// Sensor ultrassônico JSN-SR04T / AJ-SR04M.
// a prova d'agua para medir distancia do obstáculo.
#define ECHO 11  //  Pino ECHO do sensor esta conectado ao pino 11 do arduino.
#define TRIG 12  // Pino TRIG do sensor esta conectado ao pino 12 do arduino.
int dist = 0;    // distancia medida pelo sensor.
long tempo;      // intervalo de tempo da reflezão da onda no sensor.


// Controle dos servo motores
#define SERVO_ESQ 9
#define SERVO_DIR 10
Servo esq; // motor da esquerda
Servo dir; // motor da direita

void setup() {
  // put your setup code here, to run once:
  pinMode(ECHO, INPUT);   // pino ECHO do sensor esta configurado como entrada recebendo o sinal refletido.
  pinMode(TRIG, OUTPUT);  // pino TRIG do sensor esta configurado como saida, acionando a emissao da onda.
	esq.attach(SERVO_ESQ);
	dir.attach(SERVO_DIR);
  Serial.begin(9600);
  Dabble.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  medeDistancia();
  ouveChamado();
  leControle();
}


void medeDistancia() {
  digitalWrite(TRIG, LOW);      // define pino TRIG como LOW para garantir que comece como zero.
  delay(4);                     // espera 4 milissegundos antes de enviar o pulso.
  digitalWrite(TRIG, HIGH);     // define pino TRIG como HIGH para enviar a onda ultrassonica.
  delay(10);                    // espera 10 milissegundos para garantir que onda seja emitida.
  digitalWrite(TRIG, LOW);      // define pino TRIG como LOW para finalizar emissao da onda.
  tempo = pulseIn(ECHO, HIGH);  // pino ECHO e ativado quando o sinal refletido e recebido. pulseIn mede o tempo em que o pino ECHO esta em HIGH.
  dist = (tempo * 0.034) / 2;    // calcula a distancia, considerando o tempo de ida e volta da onda.
  Serial.print("Distancia medida: ");
  Serial.print(dist);
  Serial.println(" cm");
  delay(50);
}


// Microfone Módulo Amplificador De Som Max9814.
#define MIC1 A0
// A partir do sinal capturado pelo microfone, sera feita a transformada FFT (Fast Fourier Transform)
// para analisar as frequencias e verificar se tem alto sinal na frequencia de 3000 Hz
// Definicao dos parametros da transformada:
#define SAMPLES 64
#define SAMPLING_FREQ 10000
int SAMPLING_PERIOD_US = round(1000000 * (1.0 / SAMPLING_FREQ));
// Objeto da transformada FFT e dados.
double Real1[SAMPLES], Imag1[SAMPLES];
ArduinoFFT<double> FFT1 = ArduinoFFT<double>(Real1, Imag1, SAMPLES, SAMPLING_FREQ, true);
float targetFreq = 3000;  // frequencia de interesse.
float magnitudeThreshold = 73.0;

// mapeamento da frequencia para FFT bin
int frequencyToBin(float freq, float samplingFreq, int numSamples) {
  return round((freq * numSamples) / samplingFreq);
}

void ouveChamado() {
  long microseconds = micros();  // comeca tempo para coletar amostras.
  for (int i = 0; i < SAMPLES; i++) {
    Real1[i] = analogRead(MIC1);  // amostra do MIC1;
    Imag1[i] = 0;
  }
  // espera tempo de coleta de amostras terminar.
  while ((micros() - microseconds) < SAMPLING_PERIOD_US);
  microseconds += SAMPLING_PERIOD_US;
  // FFT.
  FFT1.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  FFT1.compute(FFTDirection::Forward);
  FFT1.complexToMagnitude();
  // obtem as magnitudes para a frequencia de interesse.
  int targetBin = frequencyToBin(targetFreq, SAMPLING_FREQ, SAMPLES);
  float freqMag = Real1[targetBin];
  if (freqMag > magnitudeThreshold) {
    Serial.println("Tem alguem chamando");
  }
}

void leControle() {
Dabble.processInput();
	if(GamePad.isPressed(0)) {
		esq.write(-360);
		// esq.run(BACKWARD);
		dir.write(360);
		// dir.run(FORWARD);
	}
	else {
		if(GamePad.isPressed(1)) {
			esq.write(360);
			// esq.run(FORWARD);
			dir.write(-360);
			// dir.run(BACKWARD);
		}
		else {
			if(GamePad.isPressed(2)) {
				esq.write(0);
				// esq.run(BACKWARD);
				dir.write(360);
				// dir.run(FORWARD);
			}
			else {
				if(GamePad.isPressed(3)) {
					esq.write(-360);
					// esq.run(BACKWARD);
					dir.write(0);
					// dir.run(FORWARD);
				}
				else {
					if(GamePad.isPressed(9)) {
						esq.write(360);
						// esq.run(FORWARD);
						dir.write(360);
						// dir.run(FORWARD);
					}
					else {
						if(GamePad.isPressed(7)) {
							esq.write(-360);
							// esq.run(BACKWARD);
							dir.write(-360);
							// dir.run(BACKWARD);
						}
						else {
							esq.write(360);
							// esq.run(BACKWARD);
							dir.write(360);
							// dir.run(FORWARD);
						}
					}
				}
			}
		}
	}
}