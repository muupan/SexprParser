#include <sstream>
#include <boost/tokenizer.hpp>
#include "sexprparser.hpp"

namespace sexprparser {

typedef boost::char_separator<char> Separator;
typedef boost::tokenizer<Separator> Tokenizer;

TreeElement::TreeElement(const std::string& value) :
    is_leaf_(true), value_(value), children_(std::vector<TreeElement>()) {
}

TreeElement::TreeElement(const std::vector<TreeElement>& children) :
    is_leaf_(false), value_(std::string()), children_(children) {
}

bool TreeElement::IsLeaf() const {
  return is_leaf_;
}

std::string TreeElement::GetValue() const {
  return value_;
}

std::vector<TreeElement> TreeElement::GetChildren() const {
  return children_;
}

std::string TreeElement::ToString() const {
  if (is_leaf_) {
    return "leaf:" + value_;
  } else {
    std::ostringstream o;
    o << "non-leaf[" << children_.size() << "](";
    for (const auto& child : children_) {
      o << ' ' << child.ToString();
    }
    o << " )";
    return o.str();
  }
}

// Argument iterator must point to the token next to "("
// When this function returns, iterator will point to corresponding ")"
TreeElement ParseTillRightParen(Tokenizer::iterator& i) {
  std::vector<TreeElement> children;
  for (; *i != ")"; ++i) {
    if (*i == "(") {
      // Skip left paren and parse inside parens
      ++i;
      children.push_back(ParseTillRightParen(i));
    } else {
      children.push_back(TreeElement(*i));
    }
  }
  // When this function returns, the iterator is always pointing to ")"
  return TreeElement(children);
}

std::vector<TreeElement> Parse(const std::string& sexpr) {
  Separator sep(" ", "()");
  Tokenizer tokens(sexpr, sep);
  std::vector<TreeElement> results;
  for (auto i = tokens.begin(); i != tokens.end(); ++i) {
    if (*i == "(") {
      // Skip left paren and parse inside parens
      ++i;
      results.push_back(ParseTillRightParen(i));
    } else {
      results.push_back(TreeElement(*i));
    }
  }
  return results;
}

}


