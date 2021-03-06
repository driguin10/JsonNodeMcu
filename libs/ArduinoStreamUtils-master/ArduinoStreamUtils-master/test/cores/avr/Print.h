// StreamUtils - github.com/bblanchon/ArduinoStreamUtils
// Copyright Benoit Blanchon 2019
// MIT License

#pragma once

#include <stdint.h>
#include <cstring>
#include <string>

using String = std::string;

struct Print {
  virtual size_t write(const uint8_t *buffer, size_t size) = 0;
  virtual size_t write(uint8_t data) = 0;

  size_t write(const char *buffer, size_t size) {
    return write((const uint8_t *)buffer, size);
  }

  size_t print(const String &s) {
    return write(s.c_str(), s.size());
  }

  size_t print(const char *s) {
    return write(s, std::strlen(s));
  }

  template <typename T>
  size_t print(const T &value) {
    return print(std::to_string(value));
  }

  template <typename T>
  size_t println(const T &value) {
    return print(value);
  }
};
