#ifndef LAVA_SYN_LOCATION_H_
#define LAVA_SYN_LOCATION_H_

#include <cstddef>
#include <cstdint>
#include <compare>
#include <cassert>

namespace lava::syn {

struct Location {
  size_t index;
  uint32_t line;
  uint32_t column;

  constexpr Location() noexcept
    : index{size_t(-1)}
  {}

  constexpr Location(size_t index, uint32_t line, uint32_t column) noexcept
    : index{index}
    , line{line}
    , column{column}
  {}

  friend constexpr bool
  operator==(const Location &a, const Location &b) = default;

  friend constexpr auto
  operator<=>(const Location &a, const Location &b) noexcept
  { return a.index <=> b.index; }

  explicit constexpr operator bool() const noexcept
  { return index != size_t(-1); }
};

struct Span {
  constexpr Span() noexcept
    : _start{}
    , _end{}
  {}

  constexpr Span(Location start, Location end) noexcept
    : _start{start}
    , _end{end}
  {
    assert(end.index >= start.index
        && (end.line > start.line || (end.line == start.line
                                   && end.column >= start.column)));
  }

  constexpr const Location &start() const noexcept
  { return _start; }

  constexpr const Location &end() const noexcept
  { return _end; }

  friend constexpr bool
  operator==(const Span &a, const Span &b) = default;

  friend constexpr auto
  operator<=>(const Span &a, const Span &b) = default;

  explicit constexpr operator bool() const noexcept
  { return _start && _end; }

  constexpr size_t length() const noexcept
  { return _end.index - _start.index; }

  constexpr uint32_t lines() const noexcept
  { return _end.line - _start.line + 1; }

  constexpr uint32_t columns() const noexcept {
    assert(lines() == 1 && "Columns is only defined for single-line spans.");
    return _end.column - _start.column;
  }

private:
  Location _start;
  Location _end;
};

struct RefSpan {
  const Location *_start;
  const Location *_end;

  constexpr RefSpan(const Span &span)
    : _start{&span.start()}
    , _end{&span.end()}
  {}

  constexpr RefSpan(const Location &start, const Location &end)
    : _start{&start}
    , _end{&end}
  {}

  constexpr RefSpan(const RefSpan &) = default;
  constexpr RefSpan &operator=(const RefSpan &) = default;

  constexpr const Location &start() const noexcept
  { return *_start; }

  constexpr const Location &end() const noexcept
  { return *_end; }

  friend constexpr bool
  operator==(const RefSpan &a, const RefSpan &b)
  { return a.start() == b.start() && a.end() == b.end(); }

  friend constexpr auto
  operator<=>(const RefSpan &a, const RefSpan &b) {
    auto res = a.start() <=> b.start();
    if (res != std::strong_ordering::equal) {
      return res;
    }
    return a.end() <=> b.end();
  }

  explicit constexpr operator bool() const noexcept
  { return start() && end(); }

  operator Span() const noexcept
  { return Span{*_start, *_end}; }

  constexpr size_t length() const noexcept
  { return end().index - start().index; }

  constexpr uint32_t lines() const noexcept
  { return end().line - start().line; }

  constexpr uint32_t columns() const noexcept {
    assert(lines() == 1 && "Columns is only defined for single-line spans.");
    return end().column - start().column;
  }
};

} // namespace lava::syn

#endif /* LAVA_SYN_LOCATION_H_ */
