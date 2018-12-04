#define FOSC 16000000// Clock Speed 16MHz
#define BAUD 9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>

// BAUD RATE DEFINITION
int MYUBRR = FOSC/16/BAUD-1;

// Pins used for ultrasonic sensor
// manipulation (triggers and echoes).
int trigpin = 22;
int echopin = 23;
int trigpin2 = 24;
int echopin2 = 25;

// LEDs for visual feedback
int ledPin = 26;
int ledPin2 = 27;

char buffer1[16]; // Transmission BUFFER

// Boolead for measure behaviour
// true = system performs two measures
bool doubleMeasure;

void setup() {
  // Serial communication for terminal
  Serial.begin(9600);

  // USART Config for TRXC1, RXC1 at 9600
  UBRR1H = (uint8_t)((MYUBRR)>>8);
  UBRR1L = (uint8_t)MYUBRR;
  UCSR1B |= (1<<TXEN1);
  UCSR1C = (1 << UCSZ11) | (1<<UCSZ10);

  // Pin configuration
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  pinMode(trigpin2, OUTPUT);
  pinMode(echopin2, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  // Starts using only one sensors
  doubleMeasure = false;

  // Enable INT0
  EIMSK |= (1<<INT0);
  sei();
}

void loop() {
  // Distance Variables
  long distance = measure(trigpin, echopin);
  
  // LED logic
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin2, LOW);

  // Measure with two sensors if the variable is set
  if(doubleMeasure){
    distance += measure(trigpin2, echopin2) + 2.54;
    digitalWrite(ledPin2, HIGH);
  }
  Serial.println("Distancia sumada");
  Serial.println((long) distance);
  //Serial.print(distance);
  //Serial.println(" cm");
  //transmit((char) distance);
  sprintf(buffer1,"distancia %d \n",distance);
  transmit(buffer1);
  _delay_ms(1000);
  
}

void transmit( char *data ) {
  Serial.println(data);
  for(int x=0;x<strlen(data);x++)
  {
    while (!( UCSR1A & (1<<UDRE1)));
    UDR1 = data[x];
  }
}

long measure(int tp, int ep){
  // Perform calculation for distance, accounting
  // for time and sound's speed
  long duration, distance;
  digitalWrite(tp, HIGH);
  delayMicroseconds(1000);
  digitalWrite(tp, LOW);
  duration = pulseIn(ep, HIGH);
  distance = (duration/2)/32.3;
  return distance;
}

ISR(INT0_vect, ISR_BLOCK){
  // Toggle measuring state and add delay for 
  // getting rid of signal rebound
  doubleMeasure =  !doubleMeasure;
  _delay_ms(100);
}
