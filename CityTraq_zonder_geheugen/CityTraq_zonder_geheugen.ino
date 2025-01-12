#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

const int potPin1 = A0;
const int potPin2 = A1;
const int mainButtonPin = 2;
const int sensorPins[] = {3, 4, 5};
const int numSensors = 3;
int minimumDoel = 70;
int maximumDoel = 4500;



#define NUM_LEDS2 480
#define DATA_PIN 7
#define LED_PIN 6
#define NUM_LEDS 60
#define LEDS_PER_RING 24
#define NUM_RINGS 20

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
CRGB leds[NUM_LEDS2];

int potValue1 = 0, potValue2 = 0, resultValue = 0;
int doel = 0, fietsOmwentelingen = 0, aantalRingen = 0;
bool doelBereikt = false;

int sensorStates[numSensors] = {HIGH, HIGH, HIGH};
int lastSensorStates[numSensors] = {HIGH, HIGH, HIGH};

void setup() {
    Serial.begin(9600);

    pinMode(mainButtonPin, INPUT);
    for (int i = 0; i < numSensors; i++) {
        pinMode(sensorPins[i], INPUT);
    }

    strip.begin();
    strip.show();

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS2);
    FastLED.clear();
}

void loop() {
    readPotentiometers();
    checkSensors();
    if (digitalRead(mainButtonPin) == HIGH) handleMainButton();
    updateLedRings();
    FastLED.show();
}

void readPotentiometers() {
    potValue1 = 1023 - analogRead(potPin1);
    potValue2 = analogRead(potPin2);
}

void checkSensors() {
    for (int i = 0; i < numSensors; i++) {
        sensorStates[i] = digitalRead(sensorPins[i]);
        if (lastSensorStates[i] == HIGH && sensorStates[i] == LOW) {
            fietsOmwentelingen++;
            updateLedStrip();
            printStatus();
        }
        lastSensorStates[i] = sensorStates[i];
    }
}

void handleMainButton() {
    resultValue = map(potValue1, 0, 1023, 0, 100) + map(potValue2, 0, 1023, 0, 100);
    doel = map(resultValue, 0, 200, minimumDoel, maximumDoel);
    fietsOmwentelingen = 0;
    doelBereikt = false;
    updateLedStrip();
    printStatus();

unsigned long startTime = millis();
while (digitalRead(mainButtonPin) == HIGH) {
    if (millis() - startTime > 10) break;  // Pauze van 10ms zonder de hoofdloop volledig te blokkeren
}

}

void updateLedRings() {
    FastLED.clear();
    int actieveRingen = min(aantalRingen, NUM_RINGS);
    for (int ringnr = 1; ringnr <= actieveRingen; ringnr++) {
        int startLED = LEDS_PER_RING * (ringnr - 1);
        int endLED = startLED + LEDS_PER_RING;
        for (int i = startLED; i < endLED; i++) leds[i] = CRGB(0, 0, 255);
    }
}

void updateLedStrip() {
    int numLitLeds = constrain(map(fietsOmwentelingen, 0, doel, 0, NUM_LEDS), 0, NUM_LEDS);
    int redValue, greenValue;

    if (fietsOmwentelingen >= doel) {
        redValue = 0;
        greenValue = 255;
        if (!doelBereikt) {
            aantalRingen++;
            doelBereikt = true;
        }
    } else {
        float ratio = log(1 + (float)fietsOmwentelingen / doel * 0.8);
        ratio = constrain(ratio, 0.0, 1.0);
        redValue = 255 * (1 - ratio);
        greenValue = 255 * ratio;
    }

    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, i < numLitLeds ? strip.Color(redValue, greenValue, 0) : strip.Color(0, 0, 0));
    }
    strip.show();
}

void printStatus() {
    Serial.print("Fiets Omwentelingen: ");
    Serial.print(fietsOmwentelingen);
    Serial.print(" / Doel: ");
    Serial.println(doel);
}