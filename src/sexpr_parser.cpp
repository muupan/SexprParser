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
    for (auto i = children_.begin(); i != children_.end(); ++i) {
      if (i != children_.begin()) {
        o << ' ';
      }
      o << i->ToSexpr();
    }
    o << " )";
    return o.str();
  }
}

std::string TreeNode::ToPrologAtom(const bool quotes_atoms, const std::string& atom_prefix) const {
  assert(is_leaf_);
  auto atom = value_;
  if (atom[0] == '?') {
    atom[0] = '_';
  } else {
    atom = atom_prefix + atom;
    if (quotes_atoms) {
      atom = '\'' + atom + '\'';
    }
  }
  return atom;
}

std::string TreeNode::ToPrologFunctor(const bool quotes_atoms, const std::string& functor_prefix) const {
  assert(is_leaf_);
  assert(value_[0] != '?');
  auto functor = functor_prefix + value_;
  if (quotes_atoms) {
    functor = '\'' + functor + '\'';
  }
  return functor;
}

std::string TreeNode::ToPrologTerm(const bool quotes_atoms, const std::string& functor_prefix, const std::string& atom_prefix) const {
  if (is_leaf_) {
    // Non-functor atom term
    return ToPrologAtom(quotes_atoms, atom_prefix);
  } else {
    // Compound term
    assert(children_.size() >= 2  && "Compound term must have a functor and one or more arguments.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    std::ostringstream o;
    // Functor
    o << children_.front().ToPrologFunctor(quotes_atoms, functor_prefix);
    o << '(';
    // Arguments
    for (auto i = children_.begin() + 1; i != children_.end(); ++i) {
      if (i != children_.begin() + 1) {
        o << ", ";
      }
      o << i->ToPrologTerm(quotes_atoms, functor_prefix, atom_prefix);
    }
    o << ')';
    return o.str();
  }
}

std::string TreeNode::ToPrologClause(const bool quotes_atoms, const std::string& functor_prefix, const std::string& atom_prefix) const {
  if (is_leaf_) {
    // Fact clause of atom term
    return ToPrologTerm(quotes_atoms, functor_prefix, atom_prefix) + '.';
  } else {
    assert(!children_.empty() && "Empty clause is not allowed.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    if (children_.front().GetValue() == "<=") {
      // Rule clause
      assert(children_.size() >= 3 && "Rule clause must have head and body.");
      std::ostringstream o;
      // Head
      o << children_.at(1).ToPrologTerm(quotes_atoms, functor_prefix, atom_prefix);
      o << " :- ";
      // Body
      for (auto i = children_.begin() + 2; i != children_.end(); ++i) {
        if (i != children_.begin() + 2) {
          o << ", ";
        }
        o << i->ToPrologTerm(quotes_atoms, functor_prefix, atom_prefix);
      }
      o << '.';
      return o.str();
    } else {
      // Fact clause of compound term
      return ToPrologTerm(quotes_atoms, functor_prefix, atom_prefix) + '.';
    }
  }
}

std::unordered_set<std::string> TreeNode::CollectAtoms() const {
  if (is_leaf_) {
    if (value_ == "<=" || value_.front() == '?') {
      // Not atom
      return std::unordered_set<std::string>();
    } else {
      // Atom
      return std::unordered_set<std::string>({value_});
    }
  } else {
    // Compound term
    assert(children_.size() >= 2  && "Compound term must have a functor and one or more arguments.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    std::unordered_set<std::string> values;
    for (const auto& child : children_) {
      const auto& child_atoms = child.CollectAtoms();
      values.insert(child_atoms.begin(), child_atoms.end());
    }
    return values;
  }
}

std::unordered_set<std::string> TreeNode::CollectNonFunctorAtoms() const {
  if (is_leaf_) {
    if (value_ == "<=" || value_.front() == '?') {
      // Not atom
      return std::unordered_set<std::string>();
    } else {
      // Non-functor atom
      return std::unordered_set<std::string>({value_});
    }
  } else {
    // Compound term
    assert(children_.size() >= 2  && "Compound term must have a functor and one or more arguments.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    std::unordered_set<std::string> values;
    // Ignore functor and search non-functor arguments
    for (auto i = children_.begin() + 1; i != children_.end(); ++i) {
      const auto& child_atoms = i->CollectNonFunctorAtoms();
      values.insert(child_atoms.begin(), child_atoms.end());
    }
    return values;
  }
}

std::unordered_map<std::string, int> TreeNode::CollectFunctorAtoms() const {
  if (is_leaf_) {
    // Not compound term
    return std::unordered_map<std::string, int>();
  } else {
    // Compound term
    assert(children_.size() >= 2  && "Compound term must have a functor and one or more arguments.");
    assert(children_.front().IsLeaf() && "Compound term must start with functor.");
    std::unordered_map<std::string, int> values;
    // Functor
    if (children_.front().GetValue() != "<=") {
      values.emplace(children_.front().GetValue(), children_.size() - 1);
    }
    // Search compound term arguments
    for (auto i = children_.begin() + 1; i != children_.end(); ++i) {
      if (!i->IsLeaf()) {
        const auto& child_atoms = i->CollectFunctorAtoms();
        values.insert(child_atoms.begin(), child_atoms.end());
      }
    }
    return values;
  }
}

TreeNode TreeNode::ReplaceAtoms(const std::string& before, const std::string& after) const {
  if (is_leaf_) {
    if (value_ == before) {
      return TreeNode(after);
    } else {
      return TreeNode(value_);
    }
  } else {
    std::vector<TreeNode> new_children;
    for (const auto& child : children_) {
      new_children.push_back(child.ReplaceAtoms(before, after));
    }
    return TreeNode(new_children);
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
TreeNode ParseTillRightParen(Tokenizer::iterator& i, const bool flatten_tuple_with_one_child) {
  std::vector<TreeNode> children;
  for (; *i != ")"; ++i) {
    if (*i == "(") {
      // Skip left paren and parse inside parens
      ++i;
      children.push_back(ParseTillRightParen(i, flatten_tuple_with_one_child));
    } else {
      children.push_back(TreeNode(*i));
    }
  }
  if (flatten_tuple_with_one_child && children.size() == 1) {
    return TreeNode(children.front());
  }
  // When this function returns, the iterator is always pointing to ")"
  return TreeNode(children);
}

std::vector<TreeNode> Parse(const std::string& sexpr, bool flatten_tuple_with_one_child) {
  Separator sep(" \n\t\r", "()");
  const auto sexpr_no_comments = RemoveComments(sexpr);
  Tokenizer tokens(sexpr_no_comments, sep);
  std::vector<TreeNode> results;
  for (auto i = tokens.begin(); i != tokens.end(); ++i) {
    if (*i == "(") {
      // Skip left paren and parse inside parens
      ++i;
      results.push_back(ParseTillRightParen(i, flatten_tuple_with_one_child));
    } else {
      results.push_back(TreeNode(*i));
    }
  }
  return results;
}

std::vector<TreeNode> ParseKIF(const std::string& kif) {
  return Parse(kif, true);
}

std::string ToProlog(const std::vector<TreeNode>& nodes, const bool quotes_atoms, const std::string& functor_prefix, const std::string& atom_prefix) {
  std::ostringstream o;
  for (const auto& node : nodes) {
    o << node.ToPrologClause(quotes_atoms, functor_prefix, atom_prefix) << std::endl;
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

std::unordered_set<std::string> CollectNonFunctorAtoms(const std::vector<TreeNode>& nodes) {
  std::unordered_set<std::string> values;
  for (const auto& node : nodes) {
    const auto& node_values = node.CollectNonFunctorAtoms();
    values.insert(node_values.begin(), node_values.end());
  }
  return values;
}

std::unordered_map<std::string, int> CollectFunctorAtoms(const std::vector<TreeNode>& nodes) {
  std::unordered_map<std::string, int> values;
  for (const auto& node : nodes) {
    const auto& node_values = node.CollectFunctorAtoms();
    values.insert(node_values.begin(), node_values.end());
  }
  return values;
}

std::vector<TreeNode> ReplaceAtoms(const std::vector<TreeNode>& nodes, const std::string& before, const std::string& after) {
  std::vector<TreeNode> new_nodes;
  for (const auto& node : nodes) {
    new_nodes.push_back(node.ReplaceAtoms(before, after));
  }
  return new_nodes;
}

}
