#include <gtest/gtest.h>

#include "project.h"
 //1
TEST(TemplateVersionTest, BasicAssertions) {
  struct ProjectInfo info = {};
  EXPECT_GT(info.revision, 0);
}
