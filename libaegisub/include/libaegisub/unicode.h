// Copyright (c) 2022, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#pragma once

#include <memory>
#include <string_view>
#include <unicode/brkiter.h>
#include <unicode/umachine.h>
#include <vector>

namespace agi {

class BreakIterator {
	std::unique_ptr<icu::BreakIterator> bi;
	std::string_view str;
	int32_t begin = 0, end = 0;

public:
	BreakIterator();

	void set_text(std::string_view new_str);
	bool done() const noexcept { return begin == UBRK_DONE || begin >= std::ssize(str); }
	bool is_last() const noexcept { return end == UBRK_DONE || end >= std::ssize(str); }

	void next() noexcept {
		begin = end;
		end = bi->next();
	}

	std::string_view current() const noexcept {
		if (end == UBRK_DONE) return str.substr(begin);
		return str.substr(begin, end - begin);
	}

	std::string_view current_to_end() const noexcept {
		return str.substr(begin);
	}
};

struct UTextDeleter {
	void operator()(UText *ut) { if (ut) utext_close(ut); }
};
using UTextPtr = std::unique_ptr<UText, UTextDeleter>;

/// Normalize a UTF-8 string to NFC
std::string NormalizeUnicode(std::string_view text);

/// Convert a UTF-8 string to a sequence of Unicode codepoints
std::vector<UChar32> Utf8ToCodepoints(std::string_view text);

/// Detect if a codepoint is an Arabic punctuation character
bool IsArabicPunctuation(UChar32 codepoint) noexcept;

/// Check whether a UTF-8 string contains Arabic punctuation
bool ContainsArabicPunctuation(std::string_view text);

} // namespace agi
