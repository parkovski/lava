#ifndef ASH_SYMBOL_H_
#define ASH_SYMBOL_H_

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <functional>
#include <limits>

namespace ash {

/**
 * \brief Numeric type wrapper representing a unique interned string.
 *
 * A symbol is an index into a list of interned strings. Each constant string
 * used in the program is given a unique symbol ID. A symbol may be used as
 * a string constant, but it may also optionally reference a data block
 * stored in the program's symbol table. The default symbol with an ID of
 * \c std::numeric_limits<symbol_t::id_type>::max() is invalid.
 */
struct symbol_t final {
  /// Underlying numeric type used as the symbol table index.
  using id_type = uint32_t;

  /// Default constructor for an invalid symbol ID.
  constexpr explicit symbol_t() noexcept
    : _id{std::numeric_limits<id_type>::max()}
  {}

  /// Constructor from an index of `id_type`.
  constexpr explicit symbol_t(id_type id) noexcept
    : _id{id}
  {
    assert(id != std::numeric_limits<id_type>::max());
  }

  /// Convenience conversion from \c size_t.
  constexpr static symbol_t fromSize(size_t size) noexcept {
    assert(size < (size_t)std::numeric_limits<id_type>::max());
    return symbol_t(static_cast<id_type>(size));
  }

  /// Default copy constructor.
  symbol_t(const symbol_t &) = default;

  /// Default copy assignment.
  symbol_t &operator=(const symbol_t &) = default;

  /// Equality operator over the \e ID.
  constexpr bool operator==(const symbol_t &other) const {
    return _id == other._id;
  }

  /// Inequality operator over the \e ID.
  constexpr bool operator!=(const symbol_t &other) const {
    return !(*this == other);
  }

  /// Less than operator over the \e ID.
  constexpr bool operator<(const symbol_t &other) const {
    return _id < other._id;
  }

  /// Less than or equal to operator over the \e ID.
  constexpr bool operator<=(const symbol_t &other) const {
    return *this < other || *this == other;
  }

  /// Greater than operator over the \e ID.
  constexpr bool operator>(const symbol_t &other) const {
    return other < *this;
  }

  /// Greater than or equal to operator over the \e ID.
  constexpr bool operator>=(const symbol_t &other) const {
    return other < *this || other == *this;
  }

  /// Accessor for the underlying \e ID.
  constexpr id_type id() const {
    return _id;
  }

private:
  id_type _id;
};

} // namespace ash

namespace std {
  /// Hash specialization for \c symbol_t equivalent to the \c id_type hash.
  template<> struct hash<ash::symbol_t> {
    size_t operator()(ash::symbol_t &s) const noexcept {
      return s.id();
    }
  };
} // namespace std

#endif /* ASH_SYMBOL_H_ */

