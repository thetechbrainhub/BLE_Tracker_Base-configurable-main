#include "Filters.h"

// KalmanFilter implementation
KalmanFilter::KalmanFilter(float initialValue, float processNoise, float measurementNoise) {
  Q = processNoise;
  R = measurementNoise;
  P = 1.0;
  X = initialValue;
}

float KalmanFilter::update(float measurement) {
  // Prediction step
  P = P + Q;

  // Update step
  K = P / (P + R);
  X = X + K * (measurement - X);
  P = (1 - K) * P;

  return X;
}

float KalmanFilter::getValue() {
  return X;
}

// MovingAverageFilter implementation
MovingAverageFilter::MovingAverageFilter(int size) {
  windowSize = size;
  window.resize(windowSize, 0);
  currentIndex = 0;
  windowFilled = false;
  sum = 0;
}

float MovingAverageFilter::update(float newValue) {
  // Subtract the oldest value from the sum
  sum -= window[currentIndex];
  
  // Add the new value to the window and sum
  window[currentIndex] = newValue;
  sum += newValue;
  
  // Update index and filled flag
  currentIndex = (currentIndex + 1) % windowSize;
  if (currentIndex == 0) {
    windowFilled = true;
  }
  
  // Calculate average
  return sum / (windowFilled ? windowSize : currentIndex);
}

float MovingAverageFilter::getValue() {
  return sum / (windowFilled ? windowSize : (currentIndex == 0 ? 1 : currentIndex));
}