#pragma once

#include <string>

namespace llmx {
namespace rtaco {

/**
 * @brief Enable bitwise operators for an enum class.
 *
 * Specialize this template for your enum to enable the bitwise operators.
 *
 * Example:
 * @code
 *     enum class MyFlags : uint8_t {
 *     FlagA = 0x01,
 *     FlagB = 0x02,
 *     FlagC = 0x04
 *   };
 *
 *   template <>
 *   struct enable_bitmask_operators<MyFlags> : std::true_type {};
 * @endcode
 */
template<typename _Tp>
struct enable_bitmask_operators : std::false_type {};

template<typename _Tp>
constexpr bool enable_bitmask_operators_v = enable_bitmask_operators<_Tp>::value;

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E operator|(E lhs, E rhs) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E operator&(E lhs, E rhs) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E operator^(E lhs, E rhs) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E operator~(E val) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<T>(val));
}

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E& operator|=(E& lhs, E rhs) {
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E& operator&=(E& lhs, E rhs) {
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
    requires enable_bitmask_operators_v<E>
constexpr E& operator^=(E& lhs, E rhs) {
    lhs = lhs ^ rhs;
    return lhs;
}

template<typename E, std::integral U>
    requires enable_bitmask_operators_v<E>
constexpr E operator<<(E lhs, U shift) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) << shift);
}

template<typename E, std::integral U>
    requires enable_bitmask_operators_v<E>
constexpr E operator>>(E lhs, U shift) {
    using T = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<T>(lhs) >> shift);
}

} // namespace rtaco
} // namespace llmx
