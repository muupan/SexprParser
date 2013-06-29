#include "sexpr_parser.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

namespace sexpr_parser {

typedef boost::char_separator<char> Separator;
typedef boost::tokenizer<Separator> Tokenizer;

TreeNode::TreeNode(const std::string& value) :
    is_leaf_(true), value_(value), children_() {
}

TreeNode::TreeNode(const std::vector<TreeNode>& children) :
    is_leaf_(false), value_(), children_(children) {
}

bool TreeNode::IsLeaf() const {
  return is_leaf_;
}

std::string TreeNode::GetValue() const {
  return value_;
}

std::vector<TreeNode> TreeNode::GetChildren() const {
  return children_;
}

std::string TreeNode::ToString() const {
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

std::string TreeNode::ToSexpr() const {
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

bool TreeNode::operator==(const TreeNode& another) const {
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

std::string RemoveComments(const std::string& sexpr) {
  boost::regex comment(";.*?$");
  return boost::regex_replace(sexpr, comment, std::string());
}

// Argument iterator must point to the token next to "("
// When this function returns, iterator will point to corresponding ")"
TreeNode ParseTillRightParen(Tokenizer::iterator& i) {
  std::vector<TreeNode> children;
  for (; *i != ")"; ++i) {
    if (*i == "(") {
      // Skip left paren and parse inside parens
      ++i;
      children.push_back(ParseTillRightParen(i));
    } else {
      children.push_back(TreeNode(*i));
    }
  }
  // When this function returns, the iterator is always pointing to ")"
  return TreeNode(children);
}

std::vector<TreeNode> Parse(const std::string& sexpr) {
  Separator sep(" \n\t", "()");
  const auto sexpr_no_comments = RemoveComments(sexpr);
  Tokenizer tokens(sexpr_no_comments, sep);
  std::vector<TreeNode> results;
  for (auto i = tokens.begin(); i != tokens.end(); ++i) {
    if (*i == "(") {
      // Skip left paren and parse inside parens
      ++i;
      results.push_back(ParseTillRightParen(i));
    } else {
      results.push_back(TreeNode(*i));
    }
  }
  return results;
}

}


