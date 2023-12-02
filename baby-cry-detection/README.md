# Baby Crying Detection System

## Overview

This Arduino-based system detects a baby crying using a sound sensor and communicates the detection status to AWS IoT Core. It utilizes specific sound pattern analysis and frequency analysis for accurate detection.

## AWS IoT Library

The system uses the `AWS_IOT` library from the `arduino-esp32/AWS_IOT` directory in the [ExploreEmbedded Hornbill examples](https://github.com/ExploreEmbedded/Hornbill-Examples) for AWS IoT Core integration.

## Core Logic

- **Sound Detection**: Analyzes audio signals for crying patterns. `detectSoundPattern()` identifies spikes in sound intensity, crucial for detecting a baby's cry.
- **FFT Analysis**: The `isBabyCrying()` function uses FFT to analyze sound frequencies, checking if they fall within the typical baby crying range (`BABY_CRY_FREQ_MIN` to `BABY_CRY_FREQ_MAX`) and exceed a set intensity threshold (`INTENSITY_THRESHOLD`).

## AWS IoT Publishing

Upon detection, the system publishes "True" to the `BABY_CRYING_TOPIC` on AWS IoT Core if a baby's cry is detected, and "False" otherwise.

## Configuration

Set your WiFi (`ssid`, `password`), AWS IoT endpoint (`HOST_ADDRESS`), and client ID (`CLIENT_ID`) as per your setup.

## Usage

The system operates continuously, analyzing ambient sounds and updating AWS IoT Core with the baby crying status for remote monitoring and integration with other systems.
