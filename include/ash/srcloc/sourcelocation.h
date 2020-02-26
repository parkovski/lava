#ifndef ASH_SRCLOC_SOURCELOCATION_H_
#define ASH_SRCLOC_SOURCELOCATION_H_

#include <cstdlib>

namespace ash::srcloc {

struct FileId {
  constexpr FileId() noexcept : id{(unsigned)(-1)} {}
  FileId(const FileId &) = default;
  FileId &operator=(const FileId &) = default;

  constexpr bool operator==(const FileId &other) const noexcept
  { return id == other.id; }
  constexpr bool operator!=(const FileId &other) const noexcept
  { return !(*this == other); }
  constexpr bool operator<(const FileId &other) const noexcept
  { return id < other.id; }
  constexpr bool operator<=(const FileId &other) const noexcept
  { return !(other < *this); }
  constexpr bool operator>(const FileId &other) const noexcept
  { return other < *this; }
  constexpr bool operator>=(const FileId &other) const noexcept
  { return !(*this < other); }

  constexpr bool isValid() const noexcept {
    return id != (unsigned)(-1);
  }

private:
  constexpr explicit FileId(unsigned id) noexcept : id{id} {}

  friend class SourceLocator;
  unsigned id;
};

struct LocId {
  constexpr LocId() noexcept : id{(unsigned)(-1)} {}
  LocId(const LocId &) = default;
  LocId &operator=(const LocId &) = default;

  constexpr bool operator==(const LocId &other) const noexcept
  { return id == other.id; }
  constexpr bool operator!=(const LocId &other) const noexcept
  { return !(*this == other); }
  constexpr bool operator<(const LocId &other) const noexcept
  { return id < other.id; }
  constexpr bool operator<=(const LocId &other) const noexcept
  { return !(other < *this); }
  constexpr bool operator>(const LocId &other) const noexcept
  { return other < *this; }
  constexpr bool operator>=(const LocId &other) const noexcept
  { return !(*this < other); }

  constexpr bool isValid() const noexcept {
    return id != (unsigned)(-1);
  }

private:
  constexpr explicit LocId(unsigned id) noexcept : id{id} {}

  friend class SourceLocator;
  unsigned id;
};

struct SourceLocation {
  FileId file;
  size_t index;
  unsigned line;
  unsigned column;

  constexpr bool isValid() const noexcept {
    return file.isValid();
  }
};

} // namespace ash::srcloc

#endif // ASH_SRCLOC_SOURCELOCATION_H_
