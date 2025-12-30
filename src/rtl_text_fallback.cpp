// Copyright (c) 2024
// All rights reserved.

#include "rtl_text_fallback.h"

#include <unicode/errorcode.h>
#include <unicode/localpointer.h>
#include <unicode/unistr.h>
#include <unicode/ushape.h>
#include <unicode/utypes.h>
#include <unicode/ubidi.h>

#include <string>
#include <string_view>

namespace {
// Compatibility shim for older ICU builds that may not expose UBIDI_REMOVE_BIDI_CONTROLS
#ifndef UBIDI_REMOVE_BIDI_CONTROLS
constexpr uint32_t UBIDI_REMOVE_BIDI_CONTROLS = 0;
#endif

// Convert a UTF-8 string to a UTF-16 buffer suitable for ICU transform APIs.
std::u16string ToUtf16(std::string const& text) {
        icu::UnicodeString unicode = icu::UnicodeString::fromUTF8(text);
        return std::u16string(unicode.getBuffer(), unicode.length());
}

std::string FromUtf16(std::u16string const& text16) {
        icu::UnicodeString unicode(reinterpret_cast<const UChar*>(text16.data()), text16.size());
        std::string utf8;
        unicode.toUTF8String(utf8);
        return utf8;
}

class RtlFallbackLibrary {
        bool rtl_layout;
        icu::ErrorCode status;

        static bool NeedsRtlAnchor(char16_t ch) {
                switch (ch) {
                case u'!':
                case u'.':
                case u':':
                case u',':
                case u';':
                case u'?':
                        return true;
                default:
                        return false;
                }
        }

        static std::u16string NormalizeArabicPunctuation(std::u16string_view text, bool rtl_layout) {
                if (!rtl_layout)
                        return std::u16string{text.begin(), text.end()};

                std::u16string normalized;
                normalized.reserve(text.size());

                for (char16_t ch : text) {
                        switch (ch) {
                        case u',':
                                normalized.push_back(u'\u060C'); // Arabic comma
                                break;
                        case u';':
                                normalized.push_back(u'\u061B'); // Arabic semicolon
                                break;
                        case u'.':
                                normalized.push_back(u'\u06D4'); // Arabic full stop
                                break;
                        case u'!':
                                normalized.push_back(u'\u200F'); // Anchor punctuation inside RTL runs
                                normalized.push_back(u'!');
                                break;
                        case u'?':
                                normalized.push_back(u'\u061F'); // Arabic question mark
                                break;
                        case u':':
                                normalized.push_back(u'\u200F');
                                normalized.push_back(u':');
                                break;
                        default:
                                if (NeedsRtlAnchor(ch)) {
                                        normalized.push_back(u'\u200F');
                                }
                                normalized.push_back(ch);
                                break;
                        }
                }

                return normalized;
        }

        std::u16string ShapeArabic(std::u16string text) {
                if (!rtl_layout)
                        return text;

                const uint32_t shaping_options = U_SHAPE_LETTERS_SHAPE |
                        U_SHAPE_TEXT_DIRECTION_LOGICAL |
                        U_SHAPE_LENGTH_GROW_SHRINK |
                        U_SHAPE_PRESERVE_PRESENTATION;

                // Provide headroom for expansion during shaping.
                const auto source_length = static_cast<int32_t>(text.size());
                text.resize(text.size() + 32);
                int32_t written = u_shapeArabic(
                        reinterpret_cast<const UChar*>(text.data()),
                        source_length,
                        reinterpret_cast<UChar*>(text.data()),
                        static_cast<int32_t>(text.size()),
                        shaping_options,
                        status);

                if (status.isFailure() || written <= 0)
                        return {};

                text.resize(written);
                return text;
        }

        std::string ReorderToVisual(std::u16string const& text) {
                icu::LocalUBiDiPointer bidi(ubidi_openSized(static_cast<int32_t>(text.size()), 0, status));
                if (status.isFailure() || bidi.isNull())
                        return {};

                const UBiDiLevel base_level = rtl_layout ? UBIDI_RTL : UBIDI_LTR;
                ubidi_setPara(
                        bidi.getAlias(),
                        reinterpret_cast<const UChar*>(text.data()),
                        static_cast<int32_t>(text.size()),
                        base_level,
                        nullptr,
                        status);

                if (status.isFailure())
                        return {};

                std::u16string visual(text.size() + 32, 0);
                int32_t reorder_flags = UBIDI_DO_MIRRORING |
                        UBIDI_REMOVE_BIDI_CONTROLS;
#ifdef UBIDI_INSERT_LRM_FOR_NUMBERS
                reorder_flags |= UBIDI_INSERT_LRM_FOR_NUMBERS;
#endif

                int32_t written = ubidi_writeReordered(
                        bidi.getAlias(),
                        reinterpret_cast<UChar*>(visual.data()),
                        static_cast<int32_t>(visual.size()),
                        reorder_flags,
                        status);

                if (status == U_BUFFER_OVERFLOW_ERROR) {
                        status.reset();
                        visual.resize(written);
                        written = ubidi_writeReordered(
                                bidi.getAlias(),
                                reinterpret_cast<UChar*>(visual.data()),
                                static_cast<int32_t>(visual.size()),
                                reorder_flags,
                                status);
                }

                if (status.isFailure() || written <= 0)
                        return {};

                visual.resize(written);
                return FromUtf16(visual);
        }

public:
        explicit RtlFallbackLibrary(bool rtl_layout) : rtl_layout(rtl_layout) { }

        std::string Process(std::string const& logical_text) {
                std::u16string utf16 = ToUtf16(logical_text);
                if (utf16.empty())
                        return logical_text;

                std::u16string normalized = NormalizeArabicPunctuation(utf16, rtl_layout);
                std::u16string shaped = ShapeArabic(std::move(normalized));
                if (status.isFailure() || shaped.empty())
                        return logical_text;

                std::string visual = ReorderToVisual(shaped);
                if (status.isFailure() || visual.empty())
                        return logical_text;

                return visual;
        }
};
} // namespace

std::string ApplyBidirectionalFallback(std::string const& text, bool rtl_layout) {
        if (text.empty()) return text;

        RtlFallbackLibrary shaper(rtl_layout);
        return shaper.Process(text);
}
