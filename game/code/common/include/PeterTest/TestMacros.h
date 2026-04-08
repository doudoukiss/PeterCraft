#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Peter::Test
{
  class Failure : public std::runtime_error
  {
  public:
    explicit Failure(const std::string& message)
      : std::runtime_error(message)
    {
    }
  };
} // namespace Peter::Test

#define PETER_ASSERT_TRUE(condition)                                                         \
  do                                                                                         \
  {                                                                                          \
    if (!(condition))                                                                        \
    {                                                                                        \
      std::ostringstream peterAssertStream;                                                  \
      peterAssertStream << "Assertion failed: " #condition << " at " << __FILE__ << ':'     \
                        << __LINE__;                                                         \
      throw Peter::Test::Failure(peterAssertStream.str());                                   \
    }                                                                                        \
  } while (false)

#define PETER_ASSERT_EQ(expected, actual)                                                    \
  do                                                                                         \
  {                                                                                          \
    const auto& peterExpected = (expected);                                                  \
    const auto& peterActual = (actual);                                                      \
    if (!(peterExpected == peterActual))                                                     \
    {                                                                                        \
      std::ostringstream peterAssertStream;                                                  \
      peterAssertStream << "Assertion failed: expected [" << peterExpected << "] got ["      \
                        << peterActual << "] at " << __FILE__ << ':' << __LINE__;           \
      throw Peter::Test::Failure(peterAssertStream.str());                                   \
    }                                                                                        \
  } while (false)

#define PETER_TEST_MAIN(...)                                                                 \
  int main()                                                                                 \
  {                                                                                          \
    try                                                                                      \
    {                                                                                        \
      __VA_ARGS__                                                                            \
      std::cout << "PASS\n";                                                                 \
      return 0;                                                                              \
    }                                                                                        \
    catch (const std::exception& exception)                                                  \
    {                                                                                        \
      std::cerr << exception.what() << '\n';                                                 \
      return 1;                                                                              \
    }                                                                                        \
  }
