/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef CASELESS_STRING_COMPARE_HPP_4491238
#define CASELESS_STRING_COMPARE_HPP_4491238

#include <cctype>
#include <iterator>
#include <functional>

namespace ts
{
  template <typename Cmp>
  struct CaselessElemComparator
  {
    bool operator()(char a, char b) const
    {
      Cmp cmp;
      return cmp(std::tolower(a), std::tolower(b));
    }
  };

  // The CaselessString class template keeps a reference to a string and defines all 
  // relational operators such that the stored string can be compared in a case-insensitive manner.
  // It currently only works for the basic ASCII character set.
  template <typename StringType>
  struct CaselessString
  {
    explicit CaselessString(const StringType& string_)
      : string(string_)
    {
    }

    auto begin() const
    {
      using std::begin;
      return begin(string);
    }

    auto end() const
    {
      using std::end;
      return end(string);
    }

  private:
    const StringType& string;
  };

  template <typename StringType>
  CaselessString<StringType> caseless_string(const StringType& string)
  {
    return CaselessString<StringType>(string);
  }

  template <typename T>
  auto begin(const CaselessString<T>& s)
  {
    return s.begin();
  }

  template <typename T>
  auto end(const CaselessString<T>& s)
  {
    return s.end();
  }

  template <typename T>
  bool operator==(const CaselessString<T>& a, boost::string_ref b)
  {
    using std::begin; using std::end;

    return std::equal(begin(a), end(a), begin(b), end(b), CaselessElemComparator<std::equal_to<>>());
  }

  template <typename T>
  bool operator==(boost::string_ref a, const CaselessString<T>& b)
  {
    return b == a;
  }

  template <typename T>
  bool operator!=(const CaselessString<T>& a, boost::string_ref b)
  {
    return !(a == b);
  }

  template <typename T>
  bool operator!=(boost::string_ref a, const CaselessString<T>& b)
  {
    return !(b == a);
  }

  template <typename T>
  bool operator<(const CaselessString<T>& a, boost::string_ref b)
  {
    using std::begin; using std::end;

    return std::lexicographical_compare(begin(a), end(a), begin(b), end(b), 
                                        CaselessElemComparator<std::less<>>());
  }

  template <typename T>
  bool operator<(boost::string_ref a, const CaselessString<T>& b)
  {
    using std::begin; using std::end;

    return std::lexicographical_compare(begin(a), end(a), begin(b), end(b),
                                        CaselessElemComparator<std::less<>>());
  }

  template <typename T>
  bool operator>(const CaselessString<T>& a, boost::string_ref b)
  {
    return b < a;
  }

  template <typename T>
  bool operator>(boost::string_ref a, const CaselessString<T>& b)
  {
    return b < a;
  }

  template <typename T>
  bool operator<=(const CaselessString<T>& a, boost::string_ref b)
  {
    return !(a > b);
  }

  template <typename T>
  bool operator<=(boost::string_ref a, const CaselessString<T>& b)
  {
    return !(a > b);
  }

  template <typename T>
  bool operator>=(const CaselessString<T>& a, boost::string_ref b)
  {
    return !(a < b);
  }

  template <typename T>
  bool operator>=(boost::string_ref a, const CaselessString<T>& b)
  {
    return !(a < b);
  }
}

#endif