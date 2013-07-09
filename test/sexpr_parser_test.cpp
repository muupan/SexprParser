#include "gtest/gtest.h"
#include "sexpr_parser.hpp"

#include <algorithm>

namespace sp = sexpr_parser;

TEST(RemoveComments, Test) {
  ASSERT_TRUE(sp::RemoveComments("; comment\n a ; comment") == "\n a ");
}

TEST(Parse, Empty) {
  ASSERT_TRUE(sp::Parse("").empty());
  ASSERT_TRUE(sp::Parse(" \n\t").empty());
  ASSERT_TRUE(sp::Parse("  \n\n\t\t").empty());
  ASSERT_TRUE(sp::Parse(" \n\t \n\t").empty());
}

TEST(Parse, SingleLiteral) {
  const auto nodes = sp::Parse("a");
  ASSERT_TRUE(nodes.size() == 1);
  const auto node = nodes.front();
  ASSERT_TRUE(node.IsLeaf());
  ASSERT_TRUE(node.GetValue() == "a");
}

TEST(Parse, EmptyParen) {
  const auto nodes = sp::Parse("()");
  ASSERT_TRUE(nodes.size() == 1);
  const auto node = nodes.front();
  ASSERT_TRUE(!node.IsLeaf());
  ASSERT_TRUE(node.GetChildren().empty());
}

TEST(Parse, LowerReservedWords) {
  const auto str = std::string("(ROLE INIT TRUE DOES LEGAL NEXT TERMINAL GOAL BASE INPUT OR NOT DISTINCT NOT_RESERVED)");
  const auto answer = std::string("(role init true does legal next terminal goal base input or not distinct NOT_RESERVED)");
  const auto nodes = sp::Parse(str);
  ASSERT_TRUE(nodes.size() == 1);
  const auto node = nodes.front();
  ASSERT_TRUE(node.ToSexpr() == answer);
}

TEST(Parse, Reparse) {
  const auto nodes = sp::Parse("(a (b (c) d) e)");
  ASSERT_TRUE(nodes.size() == 1);
  const auto node = nodes.front();
  const auto sexpr = node.ToSexpr();
  const auto another_nodes = sp::Parse(sexpr);
  ASSERT_TRUE(std::equal(nodes.begin(), nodes.end(), another_nodes.begin()));
}

TEST(Parse, FlattenTupleWithOneChild) {
  const auto kif = std::string("(((a)) (b (c) d) e)");
  const auto kif_flattened = std::string("(a (b c d) e)");
  const auto nodes = sp::Parse(kif, true);
  const auto nodes_flattened = sp::Parse(kif_flattened, true);
  ASSERT_TRUE(nodes.size() == 1);
  ASSERT_TRUE(nodes_flattened.size() == 1);
  ASSERT_TRUE(std::equal(nodes.begin(), nodes.end(), nodes_flattened.begin()));
}

TEST(Parse, ToPrologClause) {
  const auto nodes = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  ASSERT_TRUE(nodes.size() == 5);
  ASSERT_TRUE(nodes[0].ToPrologClause(false, "", "") == "role(player).");
  ASSERT_TRUE(nodes[1].ToPrologClause(false, "", "") == "fact1.");
  ASSERT_TRUE(nodes[2].ToPrologClause(false, "", "") == "fact2(1).");
  ASSERT_TRUE(nodes[3].ToPrologClause(false, "", "") == "rule1 :- fact1.");
  ASSERT_TRUE(nodes[4].ToPrologClause(false, "", "") == "rule2(_x) :- fact1, fact2(_x).");
}

TEST(Parse, ToProlog) {
  const auto nodes = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const std::string answer =
      "role(player).\n"
      "fact1.\n"
      "fact2(1).\n"
      "rule1 :- fact1.\n"
      "rule2(_x) :- fact1, fact2(_x).\n";
  const std::string answer_quoted =
      "'role'('player').\n"
      "'fact1'.\n"
      "'fact2'('1').\n"
      "'rule1' :- 'fact1'.\n"
      "'rule2'(_x) :- 'fact1', 'fact2'(_x).\n";
  ASSERT_TRUE(sp::ToProlog(nodes, false) == answer);
  ASSERT_TRUE(sp::ToProlog(nodes, true) == answer_quoted);
}

TEST(Parse, FilterVariableCode) {
  const auto& nodes = sp::Parse("(<= head (body ?v+v))");
  const std::string answer = "head :- body(_v_c43_v).\n";
  ASSERT_TRUE(sp::ToProlog(nodes, false) == answer);
}

TEST(CollectAtoms, Test) {
  const auto nodes = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto atoms = sp::CollectAtoms(nodes);
  ASSERT_TRUE(atoms.size() == 7); // role, player, fact1, fact2, 1, rule1, rule2
  ASSERT_TRUE(atoms.count("role"));
  ASSERT_TRUE(atoms.count("player"));
  ASSERT_TRUE(atoms.count("fact1"));
  ASSERT_TRUE(atoms.count("fact2"));
  ASSERT_TRUE(atoms.count("1"));
  ASSERT_TRUE(atoms.count("rule1"));
  ASSERT_TRUE(atoms.count("rule2"));
  ASSERT_TRUE(!atoms.count("?x"));
  ASSERT_TRUE(!atoms.count("<="));
}

TEST(CollectNonFunctorAtoms, Test) {
  const auto nodes = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto atoms = sp::CollectNonFunctorAtoms(nodes);
  ASSERT_TRUE(atoms.size() == 4); // player, fact1, 1, rule1
  ASSERT_TRUE(!atoms.count("role"));
  ASSERT_TRUE(atoms.count("player"));
  ASSERT_TRUE(atoms.count("fact1"));
  ASSERT_TRUE(!atoms.count("fact2"));
  ASSERT_TRUE(atoms.count("1"));
  ASSERT_TRUE(atoms.count("rule1"));
  ASSERT_TRUE(!atoms.count("rule2"));
  ASSERT_TRUE(!atoms.count("?x"));
  ASSERT_TRUE(!atoms.count("<="));
}

TEST(CollectFunctorAtoms, Test) {
  const auto nodes = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto atoms = sp::CollectFunctorAtoms(nodes);
  ASSERT_TRUE(atoms.size() == 3); // role, fact2, rule2
  ASSERT_TRUE(atoms.count("role"));
  ASSERT_TRUE(atoms.at("role") == 1);
  ASSERT_TRUE(!atoms.count("player"));
  ASSERT_TRUE(!atoms.count("fact1"));
  ASSERT_TRUE(atoms.count("fact2"));
  ASSERT_TRUE(atoms.at("fact2") == 1);
  ASSERT_TRUE(!atoms.count("1"));
  ASSERT_TRUE(!atoms.count("rule1"));
  ASSERT_TRUE(atoms.count("rule2"));
  ASSERT_TRUE(atoms.at("rule2") == 1);
  ASSERT_TRUE(!atoms.count("?x"));
  ASSERT_TRUE(!atoms.count("<="));
}

TEST(ReplaceAtoms, Test) {
  const auto nodes = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto nodes_replaced = sp::ReplaceAtoms(nodes, "fact1", "fact3");
  ASSERT_TRUE(nodes_replaced.size() == 5);
  ASSERT_TRUE(nodes_replaced[0].ToPrologClause(false, "", "") == "role(player).");
  ASSERT_TRUE(nodes_replaced[1].ToPrologClause(false, "", "") == "fact3.");
  ASSERT_TRUE(nodes_replaced[2].ToPrologClause(false, "", "") == "fact2(1).");
  ASSERT_TRUE(nodes_replaced[3].ToPrologClause(false, "", "") == "rule1 :- fact3.");
  ASSERT_TRUE(nodes_replaced[4].ToPrologClause(false, "", "") == "rule2(_x) :- fact3, fact2(_x).");
}
