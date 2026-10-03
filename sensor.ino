float readZMCT103C(int sensorPin) {

  const int samples = 1000;

  // Find DC offset
  long offsetSum = 0;

  for (int i = 0; i < 500; i++) {
    offsetSum += analogRead(sensorPin);
    delayMicroseconds(100);
  }

  float offset = offsetSum / 500.0;

  // Calculate RMS
  double sum = 0;

  for (int i = 0; i < samples; i++) {

    float value = analogRead(sensorPin) - offset;

    sum += value * value;

    delayMicroseconds(200);
  }

  float rmsADC = sqrt(sum / samples);

  // Your calibration factor
  float current = rmsADC * 0.00465;

  // Noise suppression
  if (current < 0.015) {
    current = 0.0;
  }

  return current;
}