#ifndef AVERAGE_VALUE_H
#define AVERAGE_VALUE_H

#include <Arduino.h>

template <typename T>
class AverageValue {
public:
  AverageValue(uint32_t valueNum)
      : maxValuesNum(valueNum > 0 ? valueNum : 1),
        valuesCount(0),
        currentPosition(0) {
    values = (T*)calloc(this->maxValuesNum, sizeof(T));
  }

  ~AverageValue() {
    if (values) {
      free(values);
    }
  }

  void push(T value) {
    if (!values) return;
    values[currentPosition] = value;
    currentPosition = (currentPosition + 1) % maxValuesNum;
    if (valuesCount < maxValuesNum) {
      valuesCount++;
    }
  }

  float average() {
    if (!values || valuesCount == 0) {
      return 0.0f;
    }
    float sum = 0.0f;
    for (uint32_t i = 0; i < valuesCount; ++i) {
      sum += (float)values[i];
    }
    return sum / (float)valuesCount;
  }

private:
  T* values;
  uint32_t maxValuesNum;
  uint32_t valuesCount;
  uint32_t currentPosition;
};

#endif

