#include "sexpr_parser.hpp"
#include <algorithm>
#include <sstream>
#include <boost/tokenizer.hpp>

namespace sexpr_parser {

typedef boost::char_separator<char> Separator;
typedef boost::tokenizer<Separator> Tokenizer;

TreeElement::TreeElement(const std::string& value) :
    is_leaf_(true), value_(value), children_() {
}

TreeElement::TreeElement(const std::vector<TreeElement>& children) :
    is_leaf_(false), value_(), children_(children) {
}

const bool& TreeElement::IsLeaf() const {
  return is_leaf_;
}

const std::string& TreeElement::GetValue() const {
  return value_;
}

const std::vector<TreeElement>& TreeElement::GetChildren() const {
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

std::string TreeElement::ToSexpr() const {
  if (is_leaf_) {
    return value_;
  } else {
    std::ostringstream o;
    o << '(';
    for (const auto& child : children_) {
      o << ' ' << child.ToSexpr();
    }
    o << " )";
    return o.str();
  }
}

bool TreeElement::operator==(const TreeElement& another) const {
  if (is_leaf_) {
    if (another.IsLeaf()) {
      return value_ == another.GetValue();
    } else {
      return false;
    }
  } else {
    if (!another.IsLeaf()) {
      return children_ == another.GetChildren();
    } else {
      return false;
    }
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
  Separator sep(" \n\t", "()");
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


