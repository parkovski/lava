#ifndef ASH_SOURCE_LOCATOR_H_
#define ASH_SOURCE_LOCATOR_H_

#include "location.h"

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace ash::source {

class SourceLocator {
public:
  explicit SourceLocator() {}

  // TEMPORARY
  FileId addFile(std::string_view path, std::string_view text);
  /// Add a file for \c path. Inserts a location record for the file
  /// at position 0 (line 1, column 1).
  FileId addFile(std::string_view path);
  FileId findFile(std::string_view path) const;
  std::string_view fileName(FileId fileId) const;
  // TEMPORARY
  std::string_view fileText(FileId fileId) const;

  // Append only!
  LocId mark(const SourceLocation &loc);
  LocId first(FileId fileId) const;
  LocId last(FileId fileId) const;
  SourceLocation find(LocId locId) const;
  SourceLocation findNext(LocId locId) const;
  SourceLocation findPrev(LocId locId) const;

private:
  bool isValidFileId(FileId fileId) const;
  bool isValidLocId(LocId locId) const;

  struct LocationRecord {
    size_t index;
    unsigned line;
    unsigned column;
  };

  struct FileRecord {
    std::string path;
    std::string text;
    std::vector<LocationRecord> locations;
  };

  std::unordered_map<std::string_view, FileId> _fileIds;
  std::vector<FileRecord> _files;
};

} // namespace ash::source

#endif // ASH__SOURCE_LOCATOR_H_
