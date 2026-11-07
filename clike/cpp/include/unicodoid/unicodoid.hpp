#ifndef UNICODOID_HPP
#define UNICODOID_HPP

#include "unicodoid/unicodoid.h"
#include "unicodoid/uax/uax29_segmentation.hpp"
#include "unicodoid/ConvertUTF.hpp"
#include "unicodoid/uax/uax15_normalization.hpp"
#include "unicodoid/uax/uax34_named_sequences.hpp"
#include "unicodoid/uax/uax44_properties.hpp"
#include "unicodoid/uts/uts10_collation.hpp"
#include "unicodoid/uts/uts51_emoji_sequences.hpp"
#include "unicodoid/uts/uts58_link_detection.hpp"
#include "unicodoid/utr/utr59_east_asian_spacing.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <stdexcept>
#include <memory>

namespace unicodoid {

/**
 * @brief Exception thrown on Unicodoid SDK errors.
 */
class Exception : public std::runtime_error {
public:
    unicodoid_status_t status;
    Exception(unicodoid_status_t st, const std::string& msg)
        : std::runtime_error(msg), status(st) {}
};

/**
 * @brief Forward declarations.
 */
class EncodingSystem;
class TextString;

/**
 * @brief Unicode Profile wrapper.
 */
class UnicodeProfile {
private:
    const unicodoid_unicode_profile_t* m_profile;

public:
    explicit UnicodeProfile(const unicodoid_unicode_profile_t* p) : m_profile(p) {}

    bool is_valid() const noexcept { return m_profile != nullptr; }

    uint32_t major_version() const noexcept { return m_profile ? m_profile->unicode_major : 0; }
    uint32_t minor_version() const noexcept { return m_profile ? m_profile->unicode_minor : 0; }
    uint32_t patch_version() const noexcept { return m_profile ? m_profile->unicode_patch : 0; }

    bool supports_uax(unicodoid_uax_flags_t flag) const noexcept {
        return m_profile && ((m_profile->supported_uax & flag) != 0);
    }

    bool supports_uts(unicodoid_uts_flags_t flag) const noexcept {
        return m_profile && ((m_profile->supported_uts & flag) != 0);
    }

    std::optional<uint32_t> to_unicode(unicodoid_char64_t native_val) const {
        if (!m_profile || !m_profile->to_unicode) return std::nullopt;
        uint32_t u = 0;
        if (m_profile->to_unicode(m_profile, native_val, &u) == UNICODOID_OK) {
            return u;
        }
        return std::nullopt;
    }

    std::optional<unicodoid_char64_t> from_unicode(uint32_t unicode_val) const {
        if (!m_profile || !m_profile->from_unicode) return std::nullopt;
        unicodoid_char64_t n = 0;
        if (m_profile->from_unicode(m_profile, unicode_val, &n) == UNICODOID_OK) {
            return n;
        }
        return std::nullopt;
    }
};

/**
 * @brief Wrapper for an encoding system.
 */
class EncodingSystem {
private:
    const unicodoid_system_t* m_sys;

public:
    explicit EncodingSystem(const unicodoid_system_t* sys) : m_sys(sys) {
        if (!sys) throw Exception(UNICODOID_ERR_INVALID_PARAM, "Null encoding system pointer");
    }

    const unicodoid_system_t* raw() const noexcept { return m_sys; }

    std::string_view id() const noexcept { return m_sys->meta.id ? m_sys->meta.id : ""; }
    std::string_view name() const noexcept { return m_sys->meta.name ? m_sys->meta.name : ""; }
    std::string_view version() const noexcept { return m_sys->meta.version ? m_sys->meta.version : ""; }
    std::string_view description() const noexcept { return m_sys->meta.description ? m_sys->meta.description : ""; }

    uint32_t code_unit_bits() const noexcept { return m_sys->meta.code_unit_bits; }
    unicodoid_char_model_t model() const noexcept { return m_sys->meta.model; }
    unicodoid_compat_level_t compat_level() const noexcept { return m_sys->meta.compat_level; }
    unicodoid_char64_t max_char_val() const noexcept { return m_sys->meta.max_char_val; }
    unicodoid_char64_t replacement_char() const noexcept { return m_sys->meta.replacement_char; }

    bool has_unicode_profile() const noexcept {
        return unicodoid_system_has_unicode_profile(m_sys);
    }

    UnicodeProfile unicode_profile() const noexcept {
        return UnicodeProfile(m_sys->unicode_profile);
    }

    /**
     * @brief Encode one character using a fresh codec state.
     *
     * For stateful systems, use a Codec or a TextString to keep state across
     * calls; a single isolated call always starts from the initial state.
     */
    std::vector<uint8_t> encode(unicodoid_char64_t ch) const {
        std::vector<uint8_t> buf(m_sys->meta.max_bytes_per_char ? m_sys->meta.max_bytes_per_char : 8);
        size_t written = 0;

        unicodoid_codec_t codec;
        unicodoid_status_t st = unicodoid_codec_init(&codec, m_sys);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to initialize codec");
        st = unicodoid_codec_encode_char(&codec, ch, buf.data(), buf.size(), &written);
        unicodoid_codec_free(&codec);
        if (st != UNICODOID_OK) throw Exception(st, "Encoding failed");
        buf.resize(written);
        return buf;
    }

    std::pair<unicodoid_char64_t, size_t> decode(const uint8_t* data, size_t len) const {
        unicodoid_char64_t ch = 0;
        size_t read_bytes = 0;

        unicodoid_codec_t codec;
        unicodoid_status_t st = unicodoid_codec_init(&codec, m_sys);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to initialize codec");
        st = unicodoid_codec_decode_char(&codec, data, len, &ch, &read_bytes);
        unicodoid_codec_free(&codec);
        if (st != UNICODOID_OK) throw Exception(st, "Decoding failed");
        return { ch, read_bytes };
    }

    bool validate(const uint8_t* data, size_t len, size_t* out_invalid_offset = nullptr) const noexcept {
        if (!m_sys->ops.validate) return true;
        unicodoid_codec_t codec;
        if (unicodoid_codec_init(&codec, m_sys) != UNICODOID_OK) return false;
        bool ok = unicodoid_codec_validate(&codec, data, len, out_invalid_offset) == UNICODOID_OK;
        unicodoid_codec_free(&codec);
        return ok;
    }

    size_t char_count(const uint8_t* data, size_t len) const {
        size_t count = 0;
        unicodoid_codec_t codec;
        unicodoid_status_t st = unicodoid_codec_init(&codec, m_sys);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to initialize codec");
        st = unicodoid_codec_char_count(&codec, data, len, &count);
        unicodoid_codec_free(&codec);
        if (st != UNICODOID_OK) throw Exception(st, "Failed counting characters");
        return count;
    }
};

/**
 * @brief Stateful codec wrapper.
 *
 * Keeps one instance of a system's state, which is required for modal systems
 * and convenient for streaming otherwise.
 */
class Codec {
private:
    unicodoid_codec_t m_codec;
    bool m_valid;

public:
    explicit Codec(const EncodingSystem& sys) : m_valid(false) {
        unicodoid_status_t st = unicodoid_codec_init(&m_codec, sys.raw());
        if (st != UNICODOID_OK) throw Exception(st, "Failed to initialize codec");
        m_valid = true;
    }

    ~Codec() {
        if (m_valid) unicodoid_codec_free(&m_codec);
    }

    Codec(const Codec&) = delete;
    Codec& operator=(const Codec&) = delete;

    Codec(Codec&& other) noexcept : m_codec(other.m_codec), m_valid(other.m_valid) {
        other.m_valid = false;
        other.m_codec.system = nullptr;
        other.m_codec.state = nullptr;
        other.m_codec.state_size = 0;
    }

    Codec& operator=(Codec&& other) noexcept {
        if (this != &other) {
            if (m_valid) unicodoid_codec_free(&m_codec);
            m_codec = other.m_codec;
            m_valid = other.m_valid;
            other.m_valid = false;
            other.m_codec.system = nullptr;
            other.m_codec.state = nullptr;
            other.m_codec.state_size = 0;
        }
        return *this;
    }

    void reset() {
        unicodoid_status_t st = unicodoid_codec_reset(&m_codec);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to reset codec");
    }

    std::vector<uint8_t> encode_char(unicodoid_char64_t ch) {
        std::vector<uint8_t> buf(m_codec.system->meta.max_bytes_per_char
                                     ? m_codec.system->meta.max_bytes_per_char
                                     : 8);
        size_t written = 0;
        unicodoid_status_t st = unicodoid_codec_encode_char(
            &m_codec, ch, buf.data(), buf.size(), &written
        );
        if (st != UNICODOID_OK) throw Exception(st, "Encoding failed");
        buf.resize(written);
        return buf;
    }

    std::pair<unicodoid_char64_t, size_t> decode_char(const uint8_t* data, size_t len) {
        unicodoid_char64_t ch = 0;
        size_t read_bytes = 0;
        unicodoid_status_t st = unicodoid_codec_decode_char(
            &m_codec, data, len, &ch, &read_bytes
        );
        if (st != UNICODOID_OK) throw Exception(st, "Decoding failed");
        return { ch, read_bytes };
    }

    std::vector<unicodoid_char64_t> decode(const uint8_t* data, size_t len) {
        std::vector<unicodoid_char64_t> out;
        size_t offset = 0;
        while (offset < len) {
            unicodoid_char64_t ch = 0;
            size_t read_bytes = 0;
            unicodoid_status_t st = unicodoid_codec_decode_char(
                &m_codec, data + offset, len - offset, &ch, &read_bytes
            );
            if (st == UNICODOID_ERR_NO_CHARACTER) {
                if (read_bytes == 0) {
                    throw Exception(UNICODOID_ERR_INTERNAL, "Decoder made no progress");
                }
                offset += read_bytes;
                continue;
            }
            if (st != UNICODOID_OK) throw Exception(st, "Decoding failed");
            if (read_bytes == 0) throw Exception(UNICODOID_ERR_INTERNAL, "Decoder made no progress");
            out.push_back(ch);
            offset += read_bytes;
        }
        return out;
    }

    std::vector<uint8_t> encode(const std::vector<unicodoid_char64_t>& chars) {
        size_t needed = 0;
        unicodoid_status_t st = m_codec.system->ops.byte_count(
            m_codec.system, chars.data(), chars.size(), &needed
        );
        if (st != UNICODOID_OK) throw Exception(st, "Failed to size encoding");

        std::vector<uint8_t> buf(needed);
        size_t written = 0;
        st = unicodoid_codec_encode(
            &m_codec, chars.data(), chars.size(), buf.data(), buf.size(), &written
        );
        if (st != UNICODOID_OK) throw Exception(st, "Encoding failed");
        buf.resize(written);
        return buf;
    }
};

/**
 * @brief Memory-safe, length-bounded string.
 */
class TextString {
private:
    struct AdoptTag {};

    unicodoid_string_t m_str;

    TextString(AdoptTag, unicodoid_string_t raw) noexcept : m_str(raw) {}

    static void clear_moved_from(TextString& other) noexcept {
        other.m_str.system = nullptr;
        other.m_str.data = nullptr;
        other.m_str.length = 0;
        other.m_str.capacity = 0;
        other.m_str.is_owned = false;
        other.m_str.state = nullptr;
        other.m_str.state_size = 0;
    }

public:
    TextString(const EncodingSystem& sys, size_t initial_capacity = 16) {
        unicodoid_status_t st = unicodoid_string_init(&m_str, sys.raw(), initial_capacity);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to initialize string");
    }

    TextString(const EncodingSystem& sys, const void* data, size_t length_bytes) {
        unicodoid_status_t st = unicodoid_string_from_slice(&m_str, sys.raw(), data, length_bytes);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to wrap slice");
    }

    TextString(const EncodingSystem& sys, const std::vector<unicodoid_char64_t>& chars) {
        unicodoid_status_t st = unicodoid_string_from_chars(&m_str, sys.raw(), chars.data(), chars.size());
        if (st != UNICODOID_OK) throw Exception(st, "Failed to create string from chars");
    }

    ~TextString() {
        unicodoid_string_free(&m_str);
    }

    TextString(const TextString& other) {
        unicodoid_status_t st = unicodoid_string_clone(&other.m_str, &m_str);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to clone string");
    }

    TextString& operator=(const TextString& other) {
        if (this != &other) {
            unicodoid_string_free(&m_str);
            unicodoid_status_t st = unicodoid_string_clone(&other.m_str, &m_str);
            if (st != UNICODOID_OK) throw Exception(st, "Failed to clone string");
        }
        return *this;
    }

    TextString(TextString&& other) noexcept : m_str(other.m_str) {
        clear_moved_from(other);
    }

    TextString& operator=(TextString&& other) noexcept {
        if (this != &other) {
            unicodoid_string_free(&m_str);
            m_str = other.m_str;
            clear_moved_from(other);
        }
        return *this;
    }

    /**
     * @brief Take ownership of a C string created by the SDK.
     *
     * The source struct is zeroed so its destructor is a no-op if it is a
     * local C object.
     */
    static TextString adopt(unicodoid_string_t raw) noexcept {
        TextString result(AdoptTag{}, raw);
        return result;
    }

    const unicodoid_string_t* raw() const noexcept { return &m_str; }
    unicodoid_string_t* raw() noexcept { return &m_str; }

    EncodingSystem system() const noexcept { return EncodingSystem(m_str.system); }
    const uint8_t* data() const noexcept { return m_str.data; }
    size_t byte_length() const noexcept { return m_str.length; }

    size_t char_length() const {
        size_t count = 0;
        unicodoid_status_t st = unicodoid_string_char_length(&m_str, &count);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to calculate char length");
        return count;
    }

    unicodoid_char64_t char_at(size_t index) const {
        unicodoid_char64_t ch = 0;
        unicodoid_status_t st = unicodoid_string_get_char_at(&m_str, index, &ch);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to get character at index");
        return ch;
    }

    void append(unicodoid_char64_t ch) {
        unicodoid_status_t st = unicodoid_string_append_char(&m_str, ch);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to append character");
    }

    void append_bytes(const void* bytes, size_t num_bytes) {
        unicodoid_status_t st = unicodoid_string_append_bytes(&m_str, bytes, num_bytes);
        if (st != UNICODOID_OK) throw Exception(st, "Failed to append bytes");
    }

    bool operator==(const TextString& other) const noexcept {
        return unicodoid_string_equals(&m_str, &other.m_str);
    }

    int compare(const TextString& other) const noexcept {
        return unicodoid_string_compare(&m_str, &other.m_str);
    }
};

/**
 * @brief System Registry in C++.
 */
class Registry {
public:
    static void init() {
        unicodoid_registry_init();
    }

    static void register_system(const EncodingSystem& sys) {
        unicodoid_status_t st = unicodoid_registry_register(sys.raw());
        if (st != UNICODOID_OK) throw Exception(st, "Failed to register system");
    }

    static std::optional<EncodingSystem> find_by_id(const std::string& id) {
        const unicodoid_system_t* sys = unicodoid_registry_find_by_id(id.c_str());
        if (sys) return EncodingSystem(sys);
        return std::nullopt;
    }

    static std::optional<EncodingSystem> find_by_name(const std::string& name) {
        const unicodoid_system_t* sys = unicodoid_registry_find_by_name(name.c_str());
        if (sys) return EncodingSystem(sys);
        return std::nullopt;
    }

    static size_t count() {
        return unicodoid_registry_count();
    }

    static EncodingSystem get_at(size_t idx) {
        const unicodoid_system_t* sys = unicodoid_registry_get_at(idx);
        if (!sys) throw Exception(UNICODOID_ERR_NOT_FOUND, "System index out of bounds");
        return EncodingSystem(sys);
    }

    static EncodingSystem utf8() { return EncodingSystem(unicodoid_system_get_utf8()); }
    static EncodingSystem utf16le() { return EncodingSystem(unicodoid_system_get_utf16le()); }
    static EncodingSystem utf16be() { return EncodingSystem(unicodoid_system_get_utf16be()); }
    static EncodingSystem utf32le() { return EncodingSystem(unicodoid_system_get_utf32le()); }
    static EncodingSystem utf32be() { return EncodingSystem(unicodoid_system_get_utf32be()); }
    static EncodingSystem ascii() { return EncodingSystem(unicodoid_system_get_ascii()); }
    static EncodingSystem custom_conlang() { return EncodingSystem(unicodoid_system_get_custom_conlang()); }
    static EncodingSystem shiftmodal16() { return EncodingSystem(unicodoid_system_get_shiftmodal16()); }
};

/**
 * @brief Transcoding API in C++.
 */
class Transcoder {
public:
    static TextString transcode(
        const TextString& src,
        const EncodingSystem& dst_sys,
        unicodoid_transcode_policy_t policy = UNICODOID_POLICY_STRICT
    ) {
        unicodoid_transcode_config_t cfg = unicodoid_transcode_config_default();
        cfg.policy = policy;

        unicodoid_string_t out_raw;
        unicodoid_status_t st = unicodoid_transcode_string(src.raw(), dst_sys.raw(), &cfg, &out_raw);
        if (st != UNICODOID_OK) throw Exception(st, "Transcoding string failed");

        return TextString::adopt(out_raw);
    }
};

/**
 * @brief Normalizer API in C++.
 */
class Normalizer {
public:
    static TextString normalize(const TextString& src, unicodoid_norm_form_t form) {
        unicodoid_string_t out_raw;
        unicodoid_status_t st = unicodoid_normalize_string(src.raw(), form, &out_raw);
        if (st != UNICODOID_OK) throw Exception(st, "Normalization failed");
        return TextString::adopt(out_raw);
    }
};

/**
 * @brief Collation API in C++.
 */
class Collator {
public:
    static int collate(
        const TextString& a,
        const TextString& b,
        unicodoid_collation_strength_t strength = UNICODOID_COLLATE_TERTIARY
    ) {
        unicodoid_collation_options_t opts = unicodoid_collation_options_default();
        opts.strength = strength;
        return unicodoid_collate(a.raw(), b.raw(), &opts);
    }
};

/**
 * @brief Text boundary type for the native segmentation API.
 *
 * Defined in <unicodoid/uax/uax29_segmentation.hpp> and brought in here for
 * the convenience of C++ callers through the include chain.
 */

/**
 * @brief Native Unicode text segmentation (UAX #29 and UAX #14).
 *
 * The algorithms are implemented in C++ directly on top of the generated
 * Unicode 18.0.0 data layer.  They do not forward to the C segmenter.
 */


} // namespace unicodoid

#endif /* UNICODOID_HPP */
