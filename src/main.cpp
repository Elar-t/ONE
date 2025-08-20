/*
 * Base Project for Kode Dot (ESP32-S3)
 * ------------------------------------------------------------
 * Purpose
 *  - Provide a clean, production-ready starting point that wires up
 *    all the on-board peripherals with clear, English documentation.
 *  - Keep implementations in this file for approachability; advanced
 *    board abstractions live under `lib/kodedot_bsp/`.
 *
 * Features covered
 *  - Display + LVGL v9 UI
 *  - Touch + IO Expander buttons
 *  - Addressable LED (WS2812B) with robust RMT init
 *  - SD card over SD_MMC (1-bit)
 *  - IMU (LSM6DSOX) + Magnetometer (LIS2MDL)
 *  - RTC (MAX31329)
 *  - PMIC / Charger (BQ25896)
 *  - Fuel Gauge (BQ27220)
 *
 * Guidance
 *  - All sections are documented with why/what they do and how to change
 *    common parameters.
 *  - Prefer short, frequent updates (1–60s) and avoid long blocking calls.
 *  - Serial prints are informative but minimal; adjust verbosity as needed.
 */


/**
 * Detecta pulsaciones de botones conectados a un expansor I/O TCA95XX_16BIT y a un pin directo del ESP32-S3.
 * Usa interrupciones para responder a eventos de pulsación sin necesidad de sondeo constante.
 * Muestra en el monitor serie el botón detectado.
 */
/* ───────── KODE | docs.kode.diy ───────── */
#include <Arduino.h>
#include <esp_io_expander.hpp> /* Librería para controlar expansores de I/O en ESP32 */

/* Configuración del expansor */
#define CHIP_NAME         TCA95XX_16BIT
#define I2C_SCL_PIN       (47)  /* Pin SCL del bus I2C */
#define I2C_SDA_PIN       (48)  /* Pin SDA del bus I2C */
#define EXP_INT_PIN       (18)  /* Pin de interrupción del expansor */
#define I2C_ADDR          (0x20)/* Dirección I2C del expansor */

/* Conexiones de botones en el expansor */
#define PAD_UP            6
#define PAD_LEFT          7
#define PAD_DOWN          8
#define PAD_RIGHT         11
#define BUTTON_BOTTOM     9

/* Botón conectado directamente al ESP32-S3 */
#define BUTTON_UP_PIN     0    /* GPIO0 */

/* Instancia del expansor */
esp_expander::Base *expander = nullptr;

/* Banderas para interrupciones pendientes */
volatile bool expanderInterrupted = false;
volatile bool buttonUpInterrupted = false;

/*configuracion led*/

#include <Adafruit_NeoPixel.h> /* Librería para controlar tiras y LEDs NeoPixel */

#define NEOPIXEL_PIN   4                  /* Pin GPIO donde está conectado el NeoPixel */
#define NUMPIXELS      1                  /* Número de NeoPixels conectados */
#define PIXEL_FORMAT   (NEO_GRB + NEO_KHZ800) /* Formato de color y velocidad de datos */

Adafruit_NeoPixel *pixels;                /* Puntero al objeto NeoPixel */

#define DELAYVAL       500                /* Tiempo de espera entre cambios (ms) */


/* ISR para interrupción del expansor */
void IRAM_ATTR handleExpanderIRQ() {
  expanderInterrupted = true;
}

/* ISR para el botón en GPIO0 */
void IRAM_ATTR handleButtonUpIRQ() {
  buttonUpInterrupted = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Button interrupt test start");

    /* Crea el objeto NeoPixel con los parámetros definidos */
  pixels = new Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, PIXEL_FORMAT);

  pixels->begin();                        /* Inicializa el NeoPixel */
  pixels->clear();                        /* Asegura que el LED empiece apagado */
  pixels->show();                         /* Aplica el cambio */

  /* Inicializa el expansor */
  expander = new esp_expander::TCA95XX_16BIT(I2C_SCL_PIN, I2C_SDA_PIN, I2C_ADDR);
  expander->init();
  expander->begin();

  /* Configura pines del expansor como entradas */
  expander->pinMode(PAD_UP,        INPUT);
  expander->pinMode(PAD_LEFT,      INPUT);
  expander->pinMode(PAD_DOWN,      INPUT);
  expander->pinMode(PAD_RIGHT,     INPUT);
  expander->pinMode(BUTTON_BOTTOM, INPUT);

  /* Configura pin de interrupción del expansor */
  pinMode(EXP_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(EXP_INT_PIN),
                  handleExpanderIRQ, FALLING);

  /* Configura botón directo en GPIO0 */
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_UP_PIN),
                  handleButtonUpIRQ, FALLING);

  Serial.println("Setup complete. Waiting for button presses...");
}

void loop() {
  /* Si no hay interrupciones pendientes, espera */
  if (!expanderInterrupted && !buttonUpInterrupted) {
    delay(10);
    return;
  }

  /* Gestiona botón directo */
  if (buttonUpInterrupted) {
    buttonUpInterrupted = false;
    Serial.println("→ BUTTON_UP (GPIO0) pressed");
    delay(50);

      /* Enciende en blanco */
  pixels->setPixelColor(0, pixels->Color(150, 0, 150));
  pixels->show();
  
  }

  /* Gestiona botones del expansor */
  if (expanderInterrupted) {
    expanderInterrupted = false;
    if (expander->digitalRead(PAD_UP) == LOW) {
      Serial.println("→ PAD_UP pressed");
    }
    if (expander->digitalRead(PAD_LEFT) == LOW) {
      Serial.println("→ PAD_LEFT pressed");
    }
    if (expander->digitalRead(PAD_DOWN) == LOW) {
      Serial.println("→ PAD_DOWN pressed");
    }
    if (expander->digitalRead(BUTTON_BOTTOM) == LOW) {
      Serial.println("→ BUTTON_BOTTOM pressed");
           /* Apaga led */
      pixels->setPixelColor(0, pixels->Color(0, 0, 0));
      pixels->show();
    }
    if (expander->digitalRead(PAD_RIGHT) == LOW) {
      Serial.println("→ PAD_RIGHT pressed");
    }
    delay(50);
  }
}

 