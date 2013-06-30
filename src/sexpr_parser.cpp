#include "sexpr_parser.hpp"

#include <algorithm>
#include <cassert>
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

std::string TreeNode::ToPrologTerm() const {
  if (is_leaf_) {
    // Atom term
    auto atom = value_;
    if (atom[0] == '?') {
      atom[0] = '_';
    }
    return atom;
  } else {
    // Compound term
    assert(children_.size() >= 2  && "Compound term must have a functor and one or more arguments.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    std::ostringstream o;
    // Functor
    o << children_.front().GetValue();
    o << '(';
    // Arguments
    for (auto i = children_.begin() + 1; i != children_.end(); ++i) {
      if (i != children_.begin() + 1) {
        o << ", ";
      }
      o << i->ToPrologTerm();
    }
    o << ')';
    return o.str();
  }
}

std::string TreeNode::ToPrologClause() const {
  if (is_leaf_) {
    // Fact clause of atom term
    return ToPrologTerm() + '.';
  } else {
    assert(!children_.empty() && "Empty clause is not allowed.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    if (children_.front().GetValue() == "<=") {
      // Rule clause
      assert(children_.size() >= 3 && "Rule clause must have head and body.");
      std::ostringstream o;
      // Head
      o << children_.at(1).ToPrologTerm();
      o << " :- ";
      // Body
      for (auto i = children_.begin() + 2; i != children_.end(); ++i) {
        if (i != children_.begin() + 2) {
          o << ", ";
        }
        o << i->ToPrologTerm();
      }
      o << '.';
      return o.str();
    } else {
      // Fact clause of compound term
      return ToPrologTerm() + '.';
    }
  }
}

std::unordered_set<std::string> TreeNode::CollectAtoms() const {
  if (is_leaf_) {
    if (value_ == "<=" || value_.front() == '?') {
      // Not atom
      return std::unordered_set<std::string>();
    } else {
      // atom
      return std::unordered_set<std::string>({value_});
    }
  } else {
    std::unordered_set<std::string> values;
    for (const auto& child : children_) {
      const auto& child_atoms = child.CollectAtoms();
      values.insert(child_atoms.begin(), child_atoms.end());
    }
    return values;
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

std::string ToProlog(const std::vector<TreeNode>& nodes) {
  std::ostringstream o;
  for (const auto& node : nodes) {
    o << node.ToPrologClause() << std::endl;
  }
  return o.str();
}

std::unordered_set<std::string> CollectAtoms(const std::vector<TreeNode>& nodes) {
  std::unordered_set<std::string> values;
  for (const auto& node : nodes) {
    const auto& node_values = node.CollectAtoms();
    values.insert(node_values.begin(), node_values.end());
  }
  return values;
}

}
