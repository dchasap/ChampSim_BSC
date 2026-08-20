#ifndef ENV_VAR_H
#define ENV_VAR_H

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <optional>
#include <string>

namespace champsim
{

template <typename T>
class EnvVar {
public:
  static std::optional<T> get(const char* name);

  static T get_or(const char* name, const T& def)
  {
    auto v = get(name);
    return v ? *v : def;
  }
};

template <>
inline std::optional<std::string> EnvVar<std::string>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  return std::string(v);
}

inline std::string to_lower_copy(const std::string& s)
{
  std::string r = s;
  std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return r;
}

template <>
inline std::optional<bool> EnvVar<bool>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  std::string s = to_lower_copy(std::string(v));
  if (s == "1" || s == "true" || s == "yes" || s == "on") {
    return true;
  }
  if (s == "0" || s == "false" || s == "no" || s == "off") {
    return false;
  }
  std::exit(1);
}

template <>
inline std::optional<unsigned long long> EnvVar<unsigned long long>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  try {
    return std::stoull(std::string(v));
  } catch (...) {
    std::exit(1);
  }
}

template <>
inline std::optional<long long> EnvVar<long long>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  try {
    return std::stoll(std::string(v));
  } catch (...) {
    std::exit(1);
  }
}

template <>
inline std::optional<double> EnvVar<double>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  try {
    return std::stod(std::string(v));
  } catch (...) {
    std::exit(1);
  }
}

template <>
inline std::optional<unsigned int> EnvVar<unsigned int>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  try {
    return static_cast<unsigned int>(std::stoull(std::string(v)));
  } catch (...) {
    std::exit(1);
  }
}

template <>
inline std::optional<int> EnvVar<int>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  try {
    return std::stoi(std::string(v));
  } catch (...) {
    std::exit(1);
  }
}

template <>
inline std::optional<unsigned long> EnvVar<unsigned long>::get(const char* name)
{
  char* v = std::getenv(name);
  if (!v) {
    return std::nullopt;
  }
  try {
    return std::stoul(std::string(v));
  } catch (...) {
    std::exit(1);
  }
}

} // namespace champsim

#endif