#include "ash/ash.h"
#include "ash/document.h"

#include <iostream>

bool readpair(const std::string &s, size_t &first, size_t &second) {
# define SKIPWS while (s[index] == ' ') { ++index; }
  size_t index = 0;
  first = 0;
  second = 0;
  SKIPWS
  if (s[index] < '0' || s[index] > '9') {
    return false;
  }
  while (s[index] >= '0' && s[index] <= '9') {
    first = first * 10 + s[index] - '0';
    ++index;
  }
  SKIPWS
  if (s[index] == ',') {
    ++index;
    SKIPWS
  }
  if (s[index] < '0' || s[index] > '9') {
    return false;
  }
  while (s[index] >= '0' && s[index] <= '9') {
    second = second * 10 + s[index] - '0';
    ++index;
  }
  return true;
# undef SKIPWS
}

int main() {
  ash::doc::CoolDocument<int> doc;

  doc.append("Hello world\nHello world\nHello world\n");

  for (size_t l = 1; l <= doc.lines(); ++l) {
    auto span = doc.spanForLine(l);
    std::cout << l << ": " << span.first << ", " << span.second << "\n";
  }

  char text[256];
  size_t bufsize = 256;
  doc.c_substr(text, &bufsize, 0, 255);
  std::cout << ">>>\n" << text << "<<<\n";

  doc.insert(17, "\nyou are a dawg\n");

  for (size_t l = 1; l <= doc.lines(); ++l) {
    auto span = doc.spanForLine(l);
    std::cout << l << ": " << span.first << ", " << span.second << "\n";
  }

  bufsize = 256;
  doc.c_substr(text, &bufsize, 0, 255);
  std::cout << ">>>\n" << text << "<<<\n";

  doc.erase(0, 12);

  for (size_t l = 1; l <= doc.lines(); ++l) {
    auto span = doc.spanForLine(l);
    std::cout << l << ": " << span.first << ", " << span.second << "\n";
  }

  bufsize = 256;
  doc.c_substr(text, &bufsize, 0, 255);
  std::cout << ">>>\n" << text << "<<<\n";

  return 0;
}
