/*
  ESP32 Real-Time Audio Frequency Visualizer

  Features:
  - FFT Analysis
  - Top 2 Dominant Frequencies
  - 16 Band Spectrum Analyzer
  - ILI9341 TFT Display

  Author: RashDriving
*/

#include <TFT_eSPI.h>
#include <arduinoFFT.h>

#define AUDIO_PIN 34

#define SAMPLES 512
#define SAMPLING_FREQUENCY 8000

double vReal[SAMPLES];
double vImag[SAMPLES];

ArduinoFFT<double> FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

TFT_eSPI tft = TFT_eSPI();

float bands[16];

void captureAudio();
void calculateBands();
void drawSpectrum();
void findDominantPeaks(float &peak1Freq,
                       float &peak2Freq,
                       float &peak1Mag,
                       float &peak2Mag);

void setup()
{
    Serial.begin(115200);

    analogReadResolution(12);

    tft.init();
    tft.setRotation(1);

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);

    delay(1000);
}

void loop()
{
    captureAudio();

    FFT.dcRemoval();

    FFT.windowing(
        FFTWindow::Hamming,
        FFTDirection::Forward);

    FFT.compute(FFTDirection::Forward);

    FFT.complexToMagnitude();

    float peak1Freq = 0;
    float peak2Freq = 0;

    float peak1Mag = 0;
    float peak2Mag = 0;

    findDominantPeaks(
        peak1Freq,
        peak2Freq,
        peak1Mag,
        peak2Mag);

    calculateBands();

    tft.fillScreen(TFT_BLACK);

    tft.setCursor(10, 10);
    tft.printf("Peak1: %.0f Hz", peak1Freq);

    tft.setCursor(10, 35);
    tft.printf("Peak2: %.0f Hz", peak2Freq);

    String type1;
    String type2;

    if (peak1Freq < 250)
        type1 = "BASS";
    else if (peak1Freq < 2000)
        type1 = "MID";
    else
        type1 = "TREBLE";

    if (peak2Freq < 250)
        type2 = "BASS";
    else if (peak2Freq < 2000)
        type2 = "MID";
    else
        type2 = "TREBLE";

    tft.setCursor(10, 60);
    tft.print(type1);

    tft.setCursor(160, 60);
    tft.print(type2);

    drawSpectrum();

    Serial.print("Peak1: ");
    Serial.print(peak1Freq);
    Serial.print(" Hz  ");

    Serial.print("Peak2: ");
    Serial.print(peak2Freq);
    Serial.println(" Hz");

    delay(50);
}

void captureAudio()
{
    const unsigned long period_us =
        1000000UL / SAMPLING_FREQUENCY;

    for (int i = 0; i < SAMPLES; i++)
    {
        unsigned long start = micros();

        vReal[i] = analogRead(AUDIO_PIN);
        vImag[i] = 0;

        while ((micros() - start) < period_us)
        {
        }
    }
}

void findDominantPeaks(
    float &peak1Freq,
    float &peak2Freq,
    float &peak1Mag,
    float &peak2Mag)
{
    for (int i = 5; i < SAMPLES / 2; i++)
    {
        float frequency =
            ((float)i * SAMPLING_FREQUENCY) /
            SAMPLES;

        float magnitude = vReal[i];

        if (magnitude > peak1Mag)
        {
            peak2Mag = peak1Mag;
            peak2Freq = peak1Freq;

            peak1Mag = magnitude;
            peak1Freq = frequency;
        }
        else if (magnitude > peak2Mag)
        {
            peak2Mag = magnitude;
            peak2Freq = frequency;
        }
    }
}

void calculateBands()
{
    int binsPerBand =
        (SAMPLES / 2) / 16;

    for (int b = 0; b < 16; b++)
    {
        float sum = 0;

        for (int i = b * binsPerBand;
             i < (b + 1) * binsPerBand;
             i++)
        {
            sum += vReal[i];
        }

        bands[b] =
            sum / binsPerBand;
    }
}

void drawSpectrum()
{
    int x = 6;
    int width = 14;

    for (int i = 0; i < 16; i++)
    {
        int height =
            constrain(
                bands[i] / 25,
                0,
                140);

        tft.fillRect(
            x,
            230 - height,
            width,
            height,
            TFT_CYAN);

        x += 19;
    }
}
