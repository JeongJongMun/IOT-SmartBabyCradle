#include <arduinoFFT.h>

const int SOUND_SENSOR_PIN = A0;
const int SAMPLES = 512; // Must be a power of 2
const double SAMPLING_FREQUENCY = 1000; // Hz, must be less than 10000 due to ADC
const int SOUND_THRESHOLD = 45; // Example threshold in decibel

// Constants for detectSoundPattern
const int MIN_SPIKES = 40; // Minimum number of spikes to consider for a crying pattern
const int MAX_SPIKES = 160; // Maximum number to avoid continuous noise
const double SPIKE_THRESHOLD = 20.0; // Threshold for detecting a spike in intensity

// Constants for isBabyCrying
const double BABY_CRY_FREQ_MIN = 250.0; // Minimum frequency for baby cry (in Hz)
const double BABY_CRY_FREQ_MAX = 600.0; // Maximum frequency for baby cry (in Hz)
const double INTENSITY_THRESHOLD = 35.0; // Minimum intensity to consider
const int MATCH_THRESHOLD = 115; // Number of matches within the frequency range to consider as baby crying

double vReal[SAMPLES];
double vImag[SAMPLES];
arduinoFFT FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

double readDecibelLevel();
bool detectSoundPattern();
bool isBabyCrying(double frequencies[], double intensities[], int count);

void setup() {
    Serial.begin(115200);
    pinMode(SOUND_SENSOR_PIN, INPUT);
}

void loop() {
    double decibelLevel = readDecibelLevel();
    Serial.print("Decibel Level: ");
    Serial.println(decibelLevel);

    if (decibelLevel > SOUND_THRESHOLD) {
        if (detectSoundPattern()) {
            // Perform FFT and intensity analysis
            FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.Compute(FFT_FORWARD);
            FFT.ComplexToMagnitude();

            double frequencies[SAMPLES / 2];
            double intensities[SAMPLES / 2];
            for (int i = 0; i < SAMPLES / 2; i++) {
                frequencies[i] = (i * SAMPLING_FREQUENCY) / SAMPLES;
                intensities[i] = vReal[i];
            }
            if (isBabyCrying(frequencies, intensities, SAMPLES / 2)) {
                Serial.println("Baby is crying!");
            } else {
                Serial.println("Detected sound is not a baby crying.");
            }
        } else {
            Serial.println("No significant crying pattern detected.");
        }
    } else {
        Serial.println("Sound level is below threshold.");
    }
    delay(1000); // Delay before the next measurement
}

double readDecibelLevel() {
    int analogValue = analogRead(SOUND_SENSOR_PIN);
    // double decibelLevel = (analogValue / 1023.0) * 120.0; // Placeholder for actual conversion
    return analogValue;
}

bool detectSoundPattern() {
    double lastIntensity = 0.0;
    int spikeCount = 0;
    bool isSpike = false;

    for (int i = 0; i < SAMPLES; i++) {
        vReal[i] = analogRead(SOUND_SENSOR_PIN); // Read and store the sound data
        double currentIntensity = vReal[i];

        if (!isSpike && currentIntensity > lastIntensity + SPIKE_THRESHOLD) {
            isSpike = true;
        } else if (isSpike && currentIntensity < lastIntensity) {
            spikeCount++;
            isSpike = false;
        }
        lastIntensity = currentIntensity;
        delay(20); // Delay for 10 milliseconds to space out the readings
    }
    Serial.print("Spike Count: ");
    Serial.println(spikeCount);
    return spikeCount >= MIN_SPIKES && spikeCount <= MAX_SPIKES;
}

bool isBabyCrying(double frequencies[], double intensities[], int count) {
    int targetFrequencyMatches = 0;
    for (int i = 0; i < count; i++) {
        if (frequencies[i] >= BABY_CRY_FREQ_MIN && frequencies[i] <= BABY_CRY_FREQ_MAX &&
            intensities[i] > INTENSITY_THRESHOLD) {
            Serial.print("Frequency : ");
            Serial.print(frequencies[i]);
            Serial.print("   intensity : ");
            Serial.println(intensities[i]);
            targetFrequencyMatches++;
        }
    }
    Serial.print("Target Frequency Matches: ");
    Serial.println(targetFrequencyMatches);
    return targetFrequencyMatches > MATCH_THRESHOLD;
}
