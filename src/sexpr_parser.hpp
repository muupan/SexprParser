#ifndef SEXPR_PARSER_HPP_
#define SEXPR_PARSER_HPP_

#include <string>
#include <vector>

namespace sexpr_parser {

class TreeElement {
public:
  TreeElement(const std::string& value);
  TreeElement(const std::vector<TreeElement>& children);
  const bool& IsLeaf() const;
  const std::string& GetValue() const;
  const std::vector<TreeElement>& GetChildren() const;
  std::string ToString() const;
  std::string ToSexpr() const;
private:
  const bool is_leaf_;
  const std::string value_;
  const std::vector<TreeElement> children_;
};

std::vector<TreeElement> Parse(const std::string& sexpr);

}

#endif /* SEXPR_PARSER_HPP_ */
