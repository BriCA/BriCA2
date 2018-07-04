#include "brica/assocvec.hpp"
#include "gtest/gtest.h"

TEST(assocvec, default) {
  brica::AssocVec<int, int> map;

  int i;
  for (i = 0; i < 10; ++i) {
    map[i] = i;
  }

  for (i = 0; i < 10; ++i) {
    ASSERT_EQ(i, map[i]);
    ASSERT_EQ(i, map.index(i));
  }
}

TEST(assocvec, greater) {
  brica::AssocVec<int, int, std::greater<int>> map;

  int i;
  for (i = 0; i < 10; ++i) {
    map[i] = i;
  }

  for (i = 0; i < 10; ++i) {
    ASSERT_EQ(i, map[i]);
    ASSERT_EQ(9 - i, map.index(i));
  }
}
