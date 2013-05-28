#ifndef SEXPRPARSER_HPP_
#define SEXPRPARSER_HPP_

#include <string>
#include <vector>

namespace sexprparser {

class TreeElement {
private:
  const bool is_leaf_;
  const std::string value_;
  const std::vector<TreeElement> children_;
public:
  TreeElement(const std::string& value);
  TreeElement(const std::vector<TreeElement>& children);
  bool IsLeaf() const;
  std::string GetValue() const;
  std::vector<TreeElement> GetChildren() const;
  std::string ToString() const;
};

std::vector<TreeElement> Parse(const std::string& sexpr);

}

#endif /* SEXPRPARSER_HPP_ */
