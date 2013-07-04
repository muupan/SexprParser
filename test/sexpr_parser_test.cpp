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
  const auto& trees = sp::Parse("a");
  ASSERT_TRUE(trees.size() == 1);
  ASSERT_TRUE(trees[0].IsLeaf());
  ASSERT_TRUE(trees[0].GetValue() == "a");
}

TEST(Parse, EmptyParen) {
  const auto& trees = sp::Parse("()");
  ASSERT_TRUE(trees.size() == 1);
  ASSERT_TRUE(!trees[0].IsLeaf());
  ASSERT_TRUE(trees[0].GetChildren().empty());
}

TEST(Parse, Reparse) {
  const auto& trees = sp::Parse("(a (b (c) d) e)");
  ASSERT_TRUE(trees.size() == 1);
  const auto& sexpr = trees[0].ToSexpr();
  const auto& another_trees = sp::Parse(sexpr);
  ASSERT_TRUE(std::equal(trees.begin(), trees.end(), another_trees.begin()));
}

TEST(Parse, ToPrologClause) {
  const auto& trees = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  ASSERT_TRUE(trees.size() == 5);
  ASSERT_TRUE(trees[0].ToPrologClause(false) == "role(player).");
  ASSERT_TRUE(trees[1].ToPrologClause(false) == "fact1.");
  ASSERT_TRUE(trees[2].ToPrologClause(false) == "fact2(1).");
  ASSERT_TRUE(trees[3].ToPrologClause(false) == "rule1 :- fact1.");
  ASSERT_TRUE(trees[4].ToPrologClause(false) == "rule2(_x) :- fact1, fact2(_x).");
}

TEST(Parse, ToProlog) {
  const auto& trees = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const std::string answer = "role(player).\n""fact1.\n""fact2(1).\n""rule1 :- fact1.\n""rule2(_x) :- fact1, fact2(_x).\n";
  const std::string answer_quoted = "'role'('player').\n""'fact1'.\n""'fact2'('1').\n""'rule1' :- 'fact1'.\n""'rule2'(_x) :- 'fact1', 'fact2'(_x).\n";
  ASSERT_TRUE(sp::ToProlog(trees, false) == answer);
  ASSERT_TRUE(sp::ToProlog(trees, true) == answer_quoted);
}

TEST(CollectAtoms, Test) {
  const auto& trees = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto& atoms = sp::CollectAtoms(trees);
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
  const auto& trees = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto& atoms = sp::CollectNonFunctorAtoms(trees);
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
  const auto& trees = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto& atoms = sp::CollectFunctorAtoms(trees);
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
  const auto& trees = sp::Parse("(role player) fact1 (fact2 1) (<= rule1 fact1) (<= (rule2 ?x) fact1 (fact2 ?x))");
  const auto& trees_replaced = sp::ReplaceAtoms(trees, "fact1", "fact3");
  ASSERT_TRUE(trees_replaced.size() == 5);
  ASSERT_TRUE(trees_replaced[0].ToPrologClause(false) == "role(player).");
  ASSERT_TRUE(trees_replaced[1].ToPrologClause(false) == "fact3.");
  ASSERT_TRUE(trees_replaced[2].ToPrologClause(false) == "fact2(1).");
  ASSERT_TRUE(trees_replaced[3].ToPrologClause(false) == "rule1 :- fact3.");
  ASSERT_TRUE(trees_replaced[4].ToPrologClause(false) == "rule2(_x) :- fact3, fact2(_x).");
}
