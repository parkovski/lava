#include "ash/source/locator.h"

#include <cassert>

using namespace ash::source;

namespace {
  constexpr unsigned FileIndexMax = 0x3FFu;
  constexpr int FileIndexBitIndex = 22;
  constexpr unsigned RecordIndexMax = 0x003FFFFFu;

  constexpr std::pair<size_t, size_t> locIdToIndexPair(unsigned locId) {
    size_t fileIndex = locId >> FileIndexBitIndex;
    size_t recordIndex = locId & RecordIndexMax;
    return std::make_pair(fileIndex, recordIndex);
  }

  constexpr unsigned indexPairToLocId(size_t fileIndex, size_t recordIndex) {
    assert(fileIndex <= FileIndexMax && "File index too large.");
    assert(recordIndex <= RecordIndexMax && "Record index too large.");
    return ((unsigned)fileIndex << FileIndexBitIndex)
         | (unsigned)recordIndex;
  }
}

FileId SourceLocator::addFile(std::string_view path, std::string_view text) {
  FileId nextFileId = FileId{(unsigned)_files.size()};
  auto [it, inserted] = _fileIds.emplace(path, nextFileId);
  if (inserted) {
    _files.emplace_back(FileRecord {
        std::string(path),
        std::string(text),
        std::vector<LocationRecord>{
          LocationRecord { 0, 1, 1 }
        }
    });
  }
  return it->second;
}

FileId SourceLocator::addFile(std::string_view path) {
  return addFile(path, {});
}

FileId SourceLocator::findFile(std::string_view path) const {
  if (auto it = _fileIds.find(path); it != _fileIds.end()) {
    return it->second;
  }
  return FileId();
}

std::string_view SourceLocator::fileName(FileId fileId) const {
  if (!isValidFileId(fileId)) {
    return {};
  }
  return _files[fileId.id].path;
}

std::string_view SourceLocator::fileText(FileId fileId) const {
  if (!isValidFileId(fileId)) {
    return {};
  }
  return _files[fileId.id].text;
}

LocId SourceLocator::mark(const SourceLocation &loc) {
  if (!isValidFileId(loc.file)) {
    return LocId();
  }
  size_t fileIndex = loc.file.id;
  auto &records = _files[fileIndex].locations;
  assert(!records.empty()
      && "Records should be initialized with location index 0.");
  assert(
    (loc.index > records.back().index && loc.line >= records.back().line)
    && "SourceLocator is currently append only!"
  );
  records.emplace_back(LocationRecord{loc.index, loc.line, loc.column});
  return LocId(indexPairToLocId(fileIndex, records.size() - 1));
}

LocId SourceLocator::first(FileId fileId) const {
  if (!isValidFileId(fileId)) {
    return LocId();
  }
  return LocId(indexPairToLocId(fileId.id, 0));
}

LocId SourceLocator::last(FileId fileId) const {
  if (!isValidFileId(fileId)) {
    return LocId();
  }
  size_t index = fileId.id;
  return LocId(indexPairToLocId(index, _files[index].locations.size() - 1));
}

SourceLocation SourceLocator::find(LocId locId) const {
  if (!isValidLocId(locId)) {
    return SourceLocation{};
  }

  auto [fileIndex, recordIndex] = locIdToIndexPair(locId.id);
  auto const &record = _files[fileIndex].locations[recordIndex];
  return SourceLocation {
    FileId((unsigned)fileIndex),
    record.index,
    record.line,
    record.column
  };
}

SourceLocation SourceLocator::findNext(LocId locId) const {
  auto [fileIndex, recordIndex] = locIdToIndexPair(locId.id);
  if (fileIndex > _files.size()
      || recordIndex > _files[fileIndex].locations.size() - 1) {
    return SourceLocation{};
  }
  auto const &record = _files[fileIndex].locations[recordIndex + 1];
  return SourceLocation {
    FileId((unsigned)fileIndex),
    record.index,
    record.line,
    record.column
  };
}

SourceLocation SourceLocator::findPrev(LocId locId) const {
  auto [fileIndex, recordIndex] = locIdToIndexPair(locId.id);
  if (fileIndex > _files.size() || recordIndex == 0) {
    return SourceLocation{};
  }
  auto const &record = _files[fileIndex].locations[recordIndex - 1];
  return SourceLocation {
    FileId((unsigned)fileIndex),
    record.index,
    record.line,
    record.column
  };
}

bool SourceLocator::isValidFileId(FileId fileId) const {
  return fileId.id < _files.size();
}

bool SourceLocator::isValidLocId(LocId locId) const {
  auto [fileIndex, recordIndex] = locIdToIndexPair(locId.id);
  return fileIndex < _files.size()
      && recordIndex < _files[fileIndex].locations.size();
}
