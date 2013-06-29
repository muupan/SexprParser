#include <iostream>
#include <string>
#include "sexpr_parser.hpp"

namespace sp = sexpr_parser;

int main() {
  const auto& sample = std::string("a (b) (c   d)\n\t(e (f (g () h) i) j)");
  std::cout << "Input S-expressions:" << std::endl;
  std::cout << sample << std::endl;
  const auto& parsed = sp::Parse(sample);
  std::cout << "Output tree structures:" << std::endl;
  for (const auto& root : parsed) {
    std::cout << root.ToString() << std::endl;
  }
  std::cout << "Output S-expressions:" << std::endl;
  for (const auto& root : parsed) {
    std::cout << root.ToSexpr() << std::endl;
  }
  return 0;
}
