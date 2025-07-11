#include <gtest/gtest.h>

#include "val/values.h"

TEST(Values, QuantifierBoolean) { 

  std::vector<bool> arr = {true,false,false};
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<bool>>(arr);
  EXPECT_EQ(val->all_of(), false);
  EXPECT_EQ(val->any_of(), true);
  EXPECT_EQ(val->none_of(), false);
  
}

TEST(Values, QuantifierCharacter) { 

  std::vector<char> arr = {0,1,2};
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<char>>(arr);
  EXPECT_EQ(val->all_of(), false);
  EXPECT_EQ(val->any_of(), true);
  EXPECT_EQ(val->none_of(), false);
   
}

TEST(Values, QuantifierInteger) { 

  std::vector<int> arr = {0,1,2};
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<int>>(arr);
  EXPECT_EQ(val->all_of(), false);
  EXPECT_EQ(val->any_of(), true);
  EXPECT_EQ(val->none_of(), false);
   
}

TEST(Values, QuantifierFloat) { 

  std::vector<double> arr = {0, 1.2, 2.34e5};
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<double>>(arr);
  EXPECT_EQ(val->all_of(), false);
  EXPECT_EQ(val->any_of(), true);
  EXPECT_EQ(val->none_of(), false);
   
}

TEST(Values, QuantifierString) { 

  std::vector<std::string> arr = {"", "a", "b"};
  val::BaseValue::PointerType val = std::make_unique<val::ArrayValue<std::string>>(arr);
  EXPECT_EQ(val->all_of(), false);
  EXPECT_EQ(val->any_of(), true);
  EXPECT_EQ(val->none_of(), false);
   
}
