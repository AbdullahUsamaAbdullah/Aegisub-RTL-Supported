// Copyright (c) 2024
// All rights reserved.

#include "rtl_text_fallback.h"

#include <unicode/ubidi.h>
#include <unicode/ubiditransform.h>
#include <unicode/localpointer.h>
#include <unicode/unistr.h>
#include <unicode/ushape.h>
#include <unicode/utypes.h>

#include <string>
#include <string_view>

namespace {
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
} // namespace

std::string ApplyBidirectionalFallback(std::string const& text, bool rtl_layout) {
        if (text.empty()) return text;

        icu::ErrorCode status;
        icu::LocalUBiDiTransformPointer transform(ubiditransform_open(status));
        if (status.isFailure() || transform.isNull())
                return text;

        auto utf16 = ToUtf16(text);
        // Add slack space so we can grow when Arabic shaping expands characters.
        std::u16string buffer(utf16.size() + 32, 0);

        const UBiDiLevel para_level = rtl_layout ? UBIDI_DEFAULT_RTL : UBIDI_DEFAULT_LTR;
        const uint32_t shaping_options = U_SHAPE_LETTERS_SHAPE |
                U_SHAPE_TEXT_DIRECTION_LOGICAL |
                U_SHAPE_LENGTH_GROW_SHRINK |
                U_SHAPE_PRESERVE_PRESENTATION;

        int32_t written = ubiditransform_transform(
                transform.getAlias(),
                reinterpret_cast<const UChar*>(utf16.data()), static_cast<int32_t>(utf16.size()),
                reinterpret_cast<UChar*>(buffer.data()), static_cast<int32_t>(buffer.size()),
                para_level, UBIDI_LOGICAL,
                para_level, UBIDI_VISUAL,
                UBIDI_MIRRORING_ON, shaping_options, status);

        if (status == U_BUFFER_OVERFLOW_ERROR) {
                status.reset();
                buffer.resize(written);
                written = ubiditransform_transform(
                        transform.getAlias(),
                        reinterpret_cast<const UChar*>(utf16.data()), static_cast<int32_t>(utf16.size()),
                        reinterpret_cast<UChar*>(buffer.data()), static_cast<int32_t>(buffer.size()),
                        para_level, UBIDI_LOGICAL,
                        para_level, UBIDI_VISUAL,
                        UBIDI_MIRRORING_ON, shaping_options, status);
        }

        if (status.isFailure() || written <= 0)
                return text;

        buffer.resize(written);
        return FromUtf16(buffer);
}
