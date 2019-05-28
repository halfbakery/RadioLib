#include "Morse.h"

// structure to save data about character Morse code
/*!
  \cond RADIOLIB_DOXYGEN_HIDDEN
*/
struct Morse_t {
  char c;                                             // ASCII character
  const char* m;                                      // Morse code representation
};
/*!
  \endcond
*/

// array of all Morse code characters
static const Morse_t MorseTable[MORSE_LENGTH] = {
  {.c = 'A', .m = ".-"},
  {.c = 'B', .m = "-..."},
  {.c = 'C', .m = "-.-."},
  {.c = 'D', .m = "-.."},
  {.c = 'E', .m = "."},
  {.c = 'F', .m = "..-."},
  {.c = 'G', .m = "--."},
  {.c = 'H', .m = "...."},
  {.c = 'I', .m = ".."},
  {.c = 'J', .m = ".---"},
  {.c = 'K', .m = "-.-"},
  {.c = 'L', .m = ".-.."},
  {.c = 'M', .m = "--"},
  {.c = 'N', .m = "-."},
  {.c = 'O', .m = "---"},
  {.c = 'P', .m = ".--."},
  {.c = 'Q', .m = "--.-"},
  {.c = 'R', .m = ".-."},
  {.c = 'S', .m = "..."},
  {.c = 'T', .m = "-"},
  {.c = 'U', .m = "..-"},
  {.c = 'V', .m = "...-"},
  {.c = 'W', .m = ".--"},
  {.c = 'X', .m = "-..-"},
  {.c = 'Y', .m = "-.--"},
  {.c = 'Z', .m = "--.."},
  {.c = '1', .m = ".----"},
  {.c = '2', .m = "..---"},
  {.c = '3', .m = "...--"},
  {.c = '4', .m = "....-"},
  {.c = '5', .m = "....."},
  {.c = '6', .m = "-...."},
  {.c = '7', .m = "--..."},
  {.c = '8', .m = "---.."},
  {.c = '9', .m = "----."},
  {.c = '0', .m = "-----"},
  {.c = '.', .m = ".-.-.-"},
  {.c = ',', .m = "--..--"},
  {.c = ':', .m = "---..."},
  {.c = '?', .m = "..--.."},
  {.c = '\'', .m = ".----."},
  {.c = '-', .m = "-....-"},
  {.c = '/', .m = "-..-."},
  {.c = '(', .m = "-.--."},
  {.c = ')', .m = "-.--.-"},
  {.c = '\"', .m = ".-..-."},
  {.c = '=', .m = "-...-"},
  {.c = '+', .m = ".-.-."},
  {.c = '@', .m = ".--.-."},
  {.c = ' ', .m = "_"},                               // space is used to separate words
  {.c = 0x01, .m = "-.-.-"},                          // ASCII SOH (start of heading) is used as alias for start signal
  {.c = 0x02, .m = ".-.-."}                           // ASCII EOT (end of transmission) is used as alias for stop signal
};

MorseClient::MorseClient(PhysicalLayer* phy) {
  _phy = phy;
}

int16_t MorseClient::begin(float base, uint8_t speed) {
  // calculate 24-bit frequency
  _base = (base * (uint32_t(1) << _phy->getDivExponent())) / _phy->getCrystalFreq();

  // calculate dot length (assumes PARIS as typical word)
  _dotLength = 1200 / speed;

  // set module frequency deviation to 0
  int16_t state = _phy->setFrequencyDeviation(0);

  return(state);
}

size_t MorseClient::startSignal() {
  return(MorseClient::write(0x01));
}

size_t MorseClient::write(const char* str) {
  if(str == NULL) {
    return(0);
  }

  return(MorseClient::write((uint8_t*)str, strlen(str)));
}

size_t MorseClient::write(uint8_t* buff, size_t len) {
  size_t n = 0;
  for(size_t i = 0; i < len; i++) {
    n += MorseClient::write(buff[i]);
  }
  return(n);
}

size_t MorseClient::write(uint8_t b) {
  // find the correct Morse code in array
  uint8_t pos;
  bool found = false;
  for(pos = 0; pos < MORSE_LENGTH; pos++) {
    if(MorseTable[pos].c == toupper(b)) {
      found = true;
      break;
    }
  }
  Serial.print((char)b);
  Serial.print('\t');
  // check if the requested code was found in the array
  if(found) {
    // iterate over Morse code representation and output appropriate tones
    for(uint8_t i = 0; i < strlen(MorseTable[pos].m); i++) {
      Serial.print(MorseTable[pos].m[i]);
      switch(MorseTable[pos].m[i]) {
        case '.':
          _phy->transmitDirect(_base);
          delay(_dotLength);
          break;
        case '-':
          _phy->transmitDirect(_base);
          delay(_dotLength * 3);
          break;
        case '_':
          // do nothing (word space)
          break;
      }

      // symbol space
      _phy->standby();
      delay(_dotLength);
    }
    Serial.println();

    // letter space
    delay(_dotLength * 3);

    return(1);
  }

  return(0);
}

size_t MorseClient::print(__FlashStringHelper* fstr) {
  PGM_P p = reinterpret_cast<PGM_P>(fstr);
  size_t n = 0;
  while(true) {
    char c = pgm_read_byte(p++);
    if(c == '\0') {
      break;
    }
    n += MorseClient::write(c);
  }
  return n;
}

size_t MorseClient::print(const String& str) {
  return(MorseClient::write((uint8_t*)str.c_str(), str.length()));
}

size_t MorseClient::print(const char* str) {
  return(MorseClient::write((uint8_t*)str, strlen(str)));
}

size_t MorseClient::print(char c) {
  return(MorseClient::write(c));
}

size_t MorseClient::print(unsigned char b, int base) {
  return(MorseClient::print((unsigned long)b, base));
}

size_t MorseClient::print(int n, int base) {
  return(MorseClient::print((long)n, base));
}

size_t MorseClient::print(unsigned int n, int base) {
  return(MorseClient::print((unsigned long)n, base));
}

size_t MorseClient::print(long n, int base) {
  if(base == 0) {
    return(MorseClient::write(n));
  } else if(base == DEC) {
    if (n < 0) {
      int t = MorseClient::print('-');
      n = -n;
      return(MorseClient::printNumber(n, DEC) + t);
    }
    return(MorseClient::printNumber(n, DEC));
  } else {
    return(MorseClient::printNumber(n, base));
  }
}

size_t MorseClient::print(unsigned long n, int base) {
  if(base == 0) {
    return(MorseClient::write(n));
  } else {
    return(MorseClient::printNumber(n, base));
  }
}

size_t MorseClient::print(double n, int digits) {
  return(MorseClient::printFloat(n, digits));
}

size_t MorseClient::println(void) {
  return(MorseClient::write(0x02));
}

size_t MorseClient::println(__FlashStringHelper* fstr) {
  size_t n = MorseClient::print(fstr);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(const String& str) {
  size_t n = MorseClient::print(str);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(const char* str) {
  size_t n = MorseClient::print(str);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(char c) {
  size_t n = MorseClient::print(c);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(unsigned char b, int base) {
  size_t n = MorseClient::print(b, base);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(int num, int base) {
  size_t n = MorseClient::print(num, base);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(unsigned int num, int base) {
  size_t n = MorseClient::print(num, base);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(long num, int base) {
  size_t n = MorseClient::print(num, base);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(unsigned long num, int base) {
  size_t n = MorseClient::print(num, base);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::println(double d, int digits) {
  size_t n = MorseClient::print(d, digits);
  n += MorseClient::println();
  return(n);
}

size_t MorseClient::printNumber(unsigned long n, uint8_t base) {
  char buf[8 * sizeof(long) + 1];
  char *str = &buf[sizeof(buf) - 1];

  *str = '\0';

  if(base < 2) {
    base = 10;
  }

  do {
    char c = n % base;
    n /= base;

    *--str = c < 10 ? c + '0' : c + 'A' - 10;
  } while(n);

  return(MorseClient::write(str));
}

size_t MorseClient::printFloat(double number, uint8_t digits)  {
  size_t n = 0;

  char code[] = {0x00, 0x00, 0x00, 0x00};
  if (isnan(number)) strcpy(code, "nan");
  if (isinf(number)) strcpy(code, "inf");
  if (number > 4294967040.0) strcpy(code, "ovf");  // constant determined empirically
  if (number <-4294967040.0) strcpy(code, "ovf");  // constant determined empirically

  if(code[0] != 0x00) {
    return(MorseClient::write(code));
  }

  // Handle negative numbers
  if (number < 0.0) {
    n += MorseClient::print('-');
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for(uint8_t i = 0; i < digits; ++i) {
    rounding /= 10.0;
  }
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  n += MorseClient::print(int_part);

  // Print the decimal point, but only if there are digits beyond
  if(digits > 0) {
    n += MorseClient::print('.');
  }

  // Extract digits from the remainder one at a time
  while(digits-- > 0) {
    remainder *= 10.0;
    unsigned int toPrint = (unsigned int)(remainder);
    n += MorseClient::print(toPrint);
    remainder -= toPrint;
  }

  return n;
}
