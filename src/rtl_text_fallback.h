// Copyright (c) 2024
// All rights reserved.

#pragma once

#include <string>

/// Apply a bidi- and Arabic-shaping pass suitable for RTL UI fallbacks when
/// the underlying text control cannot handle bidirectional rendering.
///
/// @param text Logical text from the edit buffer
/// @param rtl_layout Whether the surrounding UI is in RTL mode
/// @return A visually ordered string for display, or the original text if the
///         transform fails
std::string ApplyBidirectionalFallback(std::string const& text, bool rtl_layout);
