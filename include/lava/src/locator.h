#ifndef LAVA_SRC_LOCATOR_H_
#define LAVA_SRC_LOCATOR_H_

#include "source.h"
#include <unordered_map>
#include <filesystem>
#include <compare>
#include <iosfwd>

namespace lava::src {

// Source locator file ID.
struct FileId {
  friend struct Locator;

  constexpr FileId() noexcept : id{(uint32_t)(-1)} {}
  constexpr FileId(const FileId &) = default;
  constexpr FileId &operator=(const FileId &) = default;

  constexpr auto operator<=>(const FileId &other) const noexcept = default;

  constexpr explicit operator bool() const noexcept {
    return id != (uint32_t)(-1);
  }

private:
  constexpr explicit FileId(uint32_t id) noexcept : id{id} {}

  uint32_t id;
};

// Manages a set of source files for a program.
struct Locator {
  /// Insert a source file if it does not exist, or get the ID of the
  /// preexisting file at `path`.
  /// @param path File system path for the new source file.
  /// @return A pair `{id, inserted}` where `id` is the ID of the file at
  ///         `path` and `inserted` tells whether the file was newly inserted.
  std::pair<FileId, bool> insert(const std::filesystem::path &path);

  /// Find the source file at the given path.
  /// @param path File system path for the file.
  /// @return The ID of the file, or invalid (`FileId{}`) if the file is not
  ///         tracked.
  FileId find(const std::filesystem::path &path);

  /// Get a reference to the `Source` object for the given ID.
  /// @param id File ID. Must be valid.
  /// @return Reference to `Source` object at `id`.
  const SourceFile &operator[](FileId id) const {
    return _files[id.id];
  }

  /// Get a reference to the `Source` object for the given ID.
  /// @param id File ID. Must be valid.
  /// @return Reference to `Source` object at `id`.
  SourceFile &operator[](FileId id) {
    return _files[id.id];
  }

private:
  std::unordered_map<std::string_view, FileId> _paths;
  std::vector<SourceFile> _files;
  FileId _next_id = FileId{0};
};

} // namespace lava::src

#endif /* LAVA_SRC_LOCATOR_H_ */
