#ifndef SEXPR_PARSER_HPP_
#define SEXPR_PARSER_HPP_

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace sexpr_parser {

class TreeNode {
public:
  TreeNode(const std::string& value);
  TreeNode(const std::vector<TreeNode>& children);
  bool IsLeaf() const;
  std::string GetValue() const;
  std::vector<TreeNode> GetChildren() const;
  std::string ToString() const;
  std::string ToSexpr() const;
  std::string ToPrologClause(const bool quotes_atoms) const;
  std::string ToPrologTerm(const bool quotes_atoms) const;
  std::unordered_set<std::string> CollectAtoms() const;
  std::unordered_set<std::string> CollectNonFunctorAtoms() const;
  std::unordered_map<std::string, int> CollectFunctorAtoms() const;
  TreeNode ReplaceAtoms(const std::string& before, const std::string& after) const;
  bool operator==(const TreeNode& another) const;
private:
  const bool is_leaf_;
  const std::string value_;
  const std::vector<TreeNode> children_;
};

std::string RemoveComments(const std::string& sexpr);
std::vector<TreeNode> Parse(const std::string& sexpr);
std::string ToProlog(const std::vector<TreeNode>& nodes, const bool quotes_atoms);
std::unordered_set<std::string> CollectAtoms(const std::vector<TreeNode>& nodes);
std::unordered_set<std::string> CollectNonFunctorAtoms(const std::vector<TreeNode>& nodes);
std::unordered_map<std::string, int> CollectFunctorAtoms(const std::vector<TreeNode>& nodes);
std::vector<TreeNode> ReplaceAtoms(const std::vector<TreeNode>& nodes, const std::string& before, const std::string& after);

}

#endif /* SEXPR_PARSER_HPP_ */
