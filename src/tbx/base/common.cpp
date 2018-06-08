#include "common.h"

#include <zlib.h>

void debugprintf(const char* str, ...)
{
  static char buffer[512];
  va_list args;
  va_start (args, str);
  vsnprintf (buffer, 512, str, args);
  printf("%s\n", buffer);
}

void debugnnprintf(const char* str, ...)
{
  static char buffer[512];
  va_list args;
  va_start (args, str);
  vsnprintf (buffer, 512, str, args);
  printf("%s", buffer);
}


std::string strings::humanReadableSize(size_t bytes, bool si, u32 p) {
  static constexpr char pre[][7] = { "kMGTPE", "KMGTPE"};
  
  
  int unit = si ? 1000 : 1024;
  if (bytes < unit) return std::to_string(bytes) + "B";
  int exp = std::log(bytes) / std::log(unit);
  
  return fmt::sprintf("%.*f%c%sB", p, bytes / std::pow(unit, exp), pre[si ? 1 : 0][exp-1], si ? "" : "i");
}

bool strings::isPrefixOf(const std::string& string, const std::string& prefix)
{
  return std::mismatch(prefix.begin(), prefix.end(), string.begin()).first == prefix.end();
}

std::string strings::tolower(const std::string &text)
{
  std::string lname;
  lname.resize(text.size());
  transform(text.begin(), text.end(), lname.begin(), ::tolower);
  return lname;
}

std::vector<byte> strings::toByteArray(const std::string& string)
{
  const size_t length = string.length();
  
  assert(length % 2 == 0);
  
  std::vector<byte> array = std::vector<byte>(length/2, 0);
  
  for (size_t i = 0; i < length; ++i)
  {
    u32 shift = i % 2 == 0 ? 4 : 0;
    size_t position = i / 2;
    
    char c = string[i];
    u8 v = 0;
    
    if (c >= '0' && c <= '9') v = c - '0';
    else if (c >= 'a' && c <= 'f') v = c - 'a' + 0xa;
    else if (c >= 'A' && c <= 'F') v = c - 'A' + 0xa;
    else assert(false);
      
    array[position] |= v << shift;
  }
  
  return array;
}

std::string strings::fromByteArray(const byte* data, size_t length)
{
  constexpr bool uppercase = false;
  
  char buf[length*2+1];
  for (size_t i = 0; i < length; i++)
    sprintf(buf+i*2, uppercase ? "%02X" : "%02x", data[i]);
  
  buf[length*2] = '\0';
  return buf;
}

std::string strings::fileNameFromPath(const std::string& path)
{
  size_t lastSeparator = path.find_last_of('/');
  if (lastSeparator != std::string::npos)
    return path.substr(lastSeparator+1);
  else
    return path;
}



enum class ZlibResult : int
{
  OK = Z_OK,
  DATA_ERROR = Z_DATA_ERROR,
  MEM_ERROR = Z_MEM_ERROR,
  NEED_DICT = Z_NEED_DICT,
  STREAM_END = Z_STREAM_END
};

int inflate(byte* src, size_t length, byte* dest, size_t destLength)
{
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.total_out = 0;
  strm.avail_in = static_cast<unsigned>(length);
  strm.total_in = strm.avail_in;
  strm.next_in = src;
  
  int r = inflateInit2(&strm, -15);
  
  if (r != Z_OK)
    return r;
  
  strm.next_out = dest;
  strm.avail_out = static_cast<uint32_t>(destLength);
  
  do
  {
    r = inflate(&strm, Z_FINISH);
    
    switch (r)
    {
      case Z_NEED_DICT: r = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        inflateEnd(&strm);
        return r;
    }
  } while (r != Z_STREAM_END);
  
  inflateEnd(&strm);
  return r == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

#include <random>
  
static std::random_device device;
static std::default_random_engine engine(device());
  
u64 utils::random64(u64 first, u64 last)
{
  return (engine() % (last - first + 1)) + first;
}

