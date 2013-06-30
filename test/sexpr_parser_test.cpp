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
  const auto& trees = sp::Parse("(role player) fact (fact2 1) (<= rule1 fact) (<= (rule2 ?x) fact (fact2 ?x))");
  ASSERT_TRUE(trees.size() == 5);
  ASSERT_TRUE(trees[0].ToPrologClause() == "role(player).");
  ASSERT_TRUE(trees[1].ToPrologClause() == "fact.");
  ASSERT_TRUE(trees[2].ToPrologClause() == "fact2(1).");
  ASSERT_TRUE(trees[3].ToPrologClause() == "rule1 :- fact.");
  ASSERT_TRUE(trees[4].ToPrologClause() == "rule2(_x) :- fact, fact2(_x).");
}

TEST(Parse, ToProlog) {
  const auto& trees = sp::Parse("(role player) fact (fact2 1) (<= rule1 fact) (<= (rule2 ?x) fact (fact2 ?x))");
  const std::string answer = "role(player).\n""fact.\n""fact2(1).\n""rule1 :- fact.\n""rule2(_x) :- fact, fact2(_x).\n";
  ASSERT_TRUE(sp::ToProlog(trees) == answer);
}

TEST(CollectAtoms, Test) {
  const auto& trees = sp::Parse("(role player) fact (fact2 1) (<= rule1 fact) (<= (rule2 ?x) fact (fact2 ?x))");
  const auto& atoms = sp::CollectAtoms(trees);
  ASSERT_TRUE(atoms.size() == 7); // role, player, fact, fact2, 1, rule1, rule2
  ASSERT_TRUE(atoms.count("role"));
  ASSERT_TRUE(atoms.count("player"));
  ASSERT_TRUE(atoms.count("fact"));
  ASSERT_TRUE(atoms.count("fact2"));
  ASSERT_TRUE(atoms.count("1"));
  ASSERT_TRUE(atoms.count("rule1"));
  ASSERT_TRUE(atoms.count("rule2"));
  ASSERT_TRUE(!atoms.count("?x"));
  ASSERT_TRUE(!atoms.count("<="));
}
