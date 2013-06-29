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
