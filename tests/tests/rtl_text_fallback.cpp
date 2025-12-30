// Copyright (c) 2024
// All rights reserved.

#include <main.h>

#include "rtl_text_fallback.h"

TEST(rtl_text_fallback, reshapes_arabic_and_punctuation) {
        std::string logical = "مرحبا, كيف حالك?";
        std::string visual = ApplyBidirectionalFallback(logical, true);

        EXPECT_NE(logical, visual);
        EXPECT_NE(visual.find("\u060C"), std::string::npos); // Arabic comma
        EXPECT_NE(visual.find("\u061F"), std::string::npos); // Arabic question mark
        EXPECT_EQ(std::string::npos, visual.find(","));
        EXPECT_EQ(std::string::npos, visual.find("?"));
}

TEST(rtl_text_fallback, leaves_ltr_text_untouched_when_not_rtl) {
        std::string logical = "hello, world!";
        EXPECT_EQ(logical, ApplyBidirectionalFallback(logical, false));
}
