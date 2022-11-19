#include "lava/src/source.h"
#include "lava/util/scope_exit.h"
#include <fstream>
#include <cstdio>

#ifdef _WIN32
# define ftell _ftelli64
# define fseek _fseeki64
#endif

using namespace lava::src;

SourceFile::SourceFile(const std::filesystem::path &path)
  : _path{path.native()}
{
  FILE *file = fopen(path.native().c_str(), "rb");
  if (!file) {
    throw std::ios_base::failure("fopen");
  }
  LAVA_SCOPE_EXIT { fclose(file); };

  if (fseek(file, 0, SEEK_END)) {
    throw std::ios_base::failure("fseek(SEEK_END)");
  }
  size_t size = static_cast<size_t>(ftell(file));
  if (size == (size_t)(-1)) {
    throw std::ios_base::failure("ftell");
  }
  _content.reserve(size);
  if (fseek(file, 0, SEEK_SET)) {
    throw std::ios_base::failure("fseek(0, SEEK_SET)");
  }

  do {
    auto readsize = fread(_content.data() + _content.size(), 1, size, file);
    if (readsize == 0 && ferror(file)) {
      throw std::ios_base::failure("fread");
    }
    _content.resize(_content.size() + readsize);
    size -= readsize;
  } while (size);

  while (!feof(file)) {
    size = _content.size();
    _content.resize(_content.size() * 2);
    auto readsize = fread(_content.data() + _content.size(), 1, size, file);
    if (readsize == 0 && ferror(file)) {
      throw std::ios_base::failure("fread");
    }
    _content.resize(_content.size() + readsize);
    size -= readsize;
  }
}
