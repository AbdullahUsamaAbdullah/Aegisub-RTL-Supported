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

#include "libaegisub/unicode.h"

#include "libaegisub/exception.h"

#include <unicode/normalizer2.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/utf16.h>

using namespace agi;

BreakIterator::BreakIterator() {
	UErrorCode err = U_ZERO_ERROR;
	bi.reset(icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), err));
	if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));
}

void BreakIterator::set_text(std::string_view new_str) {
        UErrorCode err = U_ZERO_ERROR;
        UTextPtr ut(utext_openUTF8(nullptr, new_str.data(), new_str.size(), &err));
        bi->setText(ut.get(), err);
        if (U_FAILURE(err)) throw agi::InternalError(u_errorName(err));

	str = new_str;
        begin = 0;
        end = bi->next();
}

std::string agi::NormalizeUnicode(std::string_view text) {
        UErrorCode err = U_ZERO_ERROR;
        auto const *normalizer = icu::Normalizer2::getNFCInstance(err);
        if (U_FAILURE(err) || !normalizer)
                return std::string(text);

        icu::UnicodeString src = icu::UnicodeString::fromUTF8(text);
        icu::UnicodeString normalized;
        normalized = normalizer->normalize(src, err);

        if (U_FAILURE(err))
                return std::string(text);

        std::string out;
        normalized.toUTF8String(out);
        return out;
}

std::vector<UChar32> agi::Utf8ToCodepoints(std::string_view text) {
        icu::UnicodeString unicode = icu::UnicodeString::fromUTF8(text);
        std::vector<UChar32> codepoints;
        codepoints.reserve(unicode.countChar32());

        for (int32_t i = 0; i < unicode.length(); ) {
                UChar32 c = unicode.char32At(i);
                codepoints.push_back(c);
                i += U16_LENGTH(c);
        }

        return codepoints;
}

bool agi::IsArabicPunctuation(UChar32 codepoint) noexcept {
        UBlockCode block = ublock_getCode(codepoint);
        bool in_arabic_block = block == UBLOCK_ARABIC
                || block == UBLOCK_ARABIC_SUPPLEMENT
                || block == UBLOCK_ARABIC_EXTENDED_A
                || block == UBLOCK_ARABIC_EXTENDED_B;

        if (in_arabic_block) {
                switch (u_charType(codepoint)) {
                        case U_OTHER_PUNCTUATION:
                        case U_DASH_PUNCTUATION:
                        case U_CONNECTOR_PUNCTUATION:
                        case U_INITIAL_PUNCTUATION:
                        case U_FINAL_PUNCTUATION:
                                return true;
                        default:
                                break;
                }
        }

        UCharDirection dir = u_charDirection(codepoint);
        if (dir == U_RIGHT_TO_LEFT || dir == U_RIGHT_TO_LEFT_ARABIC) {
                switch (u_charType(codepoint)) {
                        case U_OTHER_PUNCTUATION:
                        case U_INITIAL_PUNCTUATION:
                        case U_FINAL_PUNCTUATION:
                                return true;
                        default:
                                break;
                }
        }

        return false;
}

bool agi::ContainsArabicPunctuation(std::string_view text) {
        for (auto codepoint : Utf8ToCodepoints(text)) {
                if (IsArabicPunctuation(codepoint))
                        return true;
        }

        return false;
}
