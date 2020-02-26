#include "ash/srcloc/sourcelocator.h"

#include <cassert>

using namespace ash::srcloc;

namespace {
  std::pair<size_t, size_t> locIdToIndexPair(unsigned locId) {
    size_t fileIndex = locId >> 22;
    size_t recordIndex = locId & 0x003FFFFF;
    return std::make_pair(fileIndex, recordIndex);
  }

  unsigned indexPairToLocId(size_t fileIndex, size_t recordIndex) {
    assert(fileIndex <= 0x3FF && "File index too large.");
    assert(recordIndex <= 0x003FFFFF && "Record index too large.");
    return ((unsigned)fileIndex << 22) | ((unsigned)recordIndex);
  }
}

FileId SourceLocator::addFile(std::string_view path) {
  FileId nextFileId = FileId{(unsigned)_files.size()};
  auto [it, inserted] = _fileIds.emplace(path, nextFileId);
  if (inserted) {
    _files.emplace_back(std::make_pair(std::string(path),
                                       std::vector<LocationRecord>()));
  }
  return it->second;
}

FileId SourceLocator::findFile(std::string_view path) const {
  if (auto it = _fileIds.find(path); it != _fileIds.end()) {
    return it->second;
  }
  return FileId();
}

std::string_view SourceLocator::fileName(FileId fileId) const {
  size_t index = fileId.id;
  if (index >= _files.size()) {
    return {};
  }
  return _files[index].first;
}

LocId SourceLocator::mark(const SourceLocation &loc) {
  size_t fileIndex = loc.file.id;
  if (fileIndex >= _fileIds.size()) {
    return LocId();
  }
  auto &records = _files[fileIndex].second;
  assert(
    (
      !records.empty() ||
      (loc.index == 0 && loc.line == 1 && loc.column == 1)
    ) &&
    "First record should point to first character"
  );
  assert(
    (
      records.empty() ||
      (loc.index > records.back().index && loc.line >= records.back().line)
    ) &&
    "SourceLocator is currently append only!"
  );
  records.emplace_back(LocationRecord{loc.index, loc.line, loc.column});
  return LocId(indexPairToLocId(fileIndex, records.size() - 1));
}

SourceLocation SourceLocator::find(LocId locId) const {
  auto [fileIndex, recordIndex] = locIdToIndexPair(locId.id);

  if (fileIndex >= _files.size()) {
    return SourceLocation{FileId(), 0, 0, 0};
  }
  auto const &records = _files[fileIndex].second;
  if (recordIndex >= records.size()) {
    return SourceLocation{FileId(), 0, 0, 0};
  }
  auto const &record = records[recordIndex];
  return SourceLocation {
    FileId((unsigned)fileIndex),
    record.index,
    record.line,
    record.column
  };
}
