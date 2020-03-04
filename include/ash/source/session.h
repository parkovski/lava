#ifndef ASH_SOURCE_SESSION_H_
#define ASH_SOURCE_SESSION_H_

#include "locator.h"

#include <tuple>

namespace ash::source {

class Session {
public:
  explicit Session(SourceLocator *locator, FileId file)
    : _locator{locator},
      _file{file}
  {}

  SourceLocator &locator() { return *_locator; }
  const SourceLocator &locator() const { return *_locator; }

  FileId file() const { return _file; }

protected:
  SourceLocator *_locator;
  FileId _file;
};

} // namespace ash::source

#endif // ASH_SOURCE_SESSION_H_
