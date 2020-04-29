#include <M5StickC.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h" //install oxullo/Arduino-MAX30100
#include "xbm.h" //my bitmap

#define REPORTING_PERIOD_MS     1000
// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;
unsigned int lastBeat = 0;
bool isHeatBeatAvailable = false;
uint16_t mainColor;
uint16_t goodColor;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("B:1");
    M5.Lcd.drawXBitmap(10, 5, hb2_bmp, hb2_bmp_width, hb2_bmp_height, mainColor);
    isHeatBeatAvailable = true;
}

void printHeartRate(float x){
    M5.Lcd.fillRect(70, 15, 100, 100,  BLACK);
    if(60 <= x && x <= 100){
        M5.Lcd.setTextColor(goodColor);
    }else{
        M5.Lcd.setTextColor(mainColor);
    }
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(70,15);
    M5.Lcd.printf("%03.0f", x);
}
void printSpO2(int x){
    M5.Lcd.fillRect(12, 70, 100, 100,  BLACK);
    M5.Lcd.setTextColor(mainColor);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(12,45);
    M5.Lcd.print("O");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(32,52);
    M5.Lcd.print("2");

    if(96 <= x){
        M5.Lcd.setTextColor(goodColor);
    }else{
        M5.Lcd.setTextColor(mainColor);
    }
    M5.Lcd.setTextSize(3);
    M5.Lcd.setCursor(70,45);
    M5.Lcd.printf("%3d", x);
}

void setup()
{
    Serial.begin(115200);
    M5.begin();
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(WHITE);
    delay(1000);
    M5.Lcd.fillScreen(BLACK);
    mainColor = M5.Lcd.color565(245, 245, 245); // white smoke
    goodColor = M5.Lcd.color565(0, 250, 154); // medium spring green
    M5.Lcd.drawXBitmap(10, 5, hb1_bmp, hb1_bmp_width, hb1_bmp_height, mainColor);
    printHeartRate(0.0);
    printSpO2(0);
    
    // Initialize the PulseOximeter instance and register a beat-detected callback
    // The parameter passed to the begin() method changes the samples flow that
    // the library spews to the serial.
    // Options:
    //  * PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT : filtered samples and beat detection threshold
    //  * PULSEOXIMETER_DEBUGGINGMODE_RAW_VALUES : sampled values coming from the sensor, with no processing
    //  * PULSEOXIMETER_DEBUGGINGMODE_AC_VALUES : sampled values after the DC removal filter

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin(PULSEOXIMETER_DEBUGGINGMODE_PULSEDETECT)) {
        Serial.println("ERROR: Failed to initialize pulse oximeter");
        for(;;);
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    // Make sure to call update as fast as possible
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("H:");
        Serial.println(pox.getHeartRate());

        Serial.print("O:");
        Serial.println(pox.getSpO2());

        tsLastReport = millis();
        if(isHeatBeatAvailable){
            lastBeat++;
            if(lastBeat > 3){
                isHeatBeatAvailable = false;
                lastBeat = 0;
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.drawXBitmap(10, 5, hb1_bmp, hb1_bmp_width, hb1_bmp_height, mainColor);
            }
        }
        printHeartRate(pox.getHeartRate());
        printSpO2(pox.getSpO2());
    }
}
