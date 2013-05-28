#include <iostream>
#include <string>
#include "sexprparser.hpp"

int main() {
  const std::string sample = "a (b) (c d) (e (f (g h) i) j)";
  const auto& parsed = sexprparser::Parse(sample);
  for (const auto& root : parsed) {
    std::cout << root.ToString() << std::endl;
  }
  return 0;
}
