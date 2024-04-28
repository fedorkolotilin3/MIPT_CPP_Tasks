#include <algorithm>
#include <iostream>
#include <cstring>

using std::cin;
using std::cout;
using std::memcpy;
using std::memset;
class String {
 private:
  size_t _size = 0;
  size_t _capacity;
  char* _arr;

  String(size_t size) : _size(size), _capacity(size + 1), _arr(new char[size + 1]) {
    _arr[_size] = '\0';
  }

  void reallocate(size_t new_cp) {
    char* new_arr = new char[new_cp];
    memcpy(new_arr, _arr, _size + 1);
    _capacity = new_cp;
    delete[] _arr;
    _arr = new_arr;
  }

 public:

  String() : String(static_cast<size_t>(0)) {
  }

  String(size_t count, char ch) : String(count) {
    memset(_arr, ch, count);
  }

  String(const char* string) : String(strlen(string)) {
    memcpy(_arr, string, _size);
  }

  String(const String &string) : String(string._size) {
    memcpy(_arr, string._arr, _size);
  }

  String &operator=(String other) {
    swap(other);
    return *this;
  }

  char operator[](size_t pos) const {
    return _arr[pos];
  }

  char &operator[](size_t pos) {
    return _arr[pos];
  }

  String &operator+=(const String &rhs) {
    if (_capacity <= _size + rhs._size) {
      reallocate(_size + rhs._size + 1);
    }
    memcpy(_arr + _size, rhs._arr, rhs._size);
    _size += rhs._size;
    _arr[_size] = '\0';
    return *this;
  }

  String &operator+=(const char ch) {
    push_back(ch);
    return *this;
  }

  void swap(String &other) {
    std::swap(_arr, other._arr);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  size_t length() const {
    return _size;
  }

  size_t size() const {
    return _size;
  }

  size_t capacity() const {
    return _capacity - 1;
  }

  char front() const {
    return _arr[0];
  }

  char back() const {
    return _arr[_size - 1];
  }

  char &front() {
    return _arr[0];
  }

  char &back() {
    return _arr[_size - 1];
  }

  void push_back(char ch) {
    if (_capacity == _size + 1) {
      reallocate(2 * _capacity);
    }
    _arr[_size] = ch;
    _arr[++_size] = '\0';
  }

  void pop_back() {
    _arr[--_size] = '\0';
  }

  String substr(size_t start, size_t count) const {
    String substring(count);
    memcpy(substring._arr, _arr + start, count);
    return substring;
  }

  bool equals_to_substring(size_t ind, const String &sub) const {
    if (_size - ind < sub.size()) {
      return false;
    }
    for(size_t i = 0; i < sub.size(); i++) {
      if (_arr[i + ind] != sub[i]) {
        return false;
      }
    }
    return true;
  }

  size_t find(const String &sub) const {
    for (size_t i = 0; i + sub._size <= _size; i++) {
      if (equals_to_substring(i, sub)) {
        return (i);
      }
    }
    return _size;
  }

  size_t rfind(const String &sub) const {
    for (size_t i = (_size - sub._size + 1); i > 0; i--) {
      if (equals_to_substring(i - 1, sub)) {
        return (i - 1);
      }
    }
    return _size;
  }

  bool empty() const {
    return _size == 0;
  }

  void clear() {
    _size = 0;
    _arr[0] = '\0';
  }

  void shrink_to_fit() {
    reallocate(_size + 1);
  }

  ~String() {
    delete[] _arr;
  }

  char* data() {
    return _arr;
  }

  const char* data() const {
    return _arr;
  }
};

bool is_end(char ch) {
  switch (ch) {
    case ' ':return true;
    case '\n':return true;
    case '\t':return true;
    case '\0':return true;
    case -1:return true;
    default:return false;
  }
}

std::istream &operator>>(std::istream &is, String &string) {
  string.clear();
  char ch = is.get();
  while (is && is_end(ch)) {
    ch = is.get();
  }
  while (is && !is_end(ch)) {
    string.push_back(ch);
    ch = is.get();
  }
  return is;
}

std::ostream &operator<<(std::ostream &os, const String &string) {
  for (size_t i = 0; i < string.size(); i++) {
    os << string[i];
  }
  return os;
}

String operator+(const String &str1, const String &str2) {
  String sum_string;
  sum_string += str1;
  sum_string += str2;
  return sum_string;
}

String operator+(const char str1, const String &str2) {
  String sum_string;
  sum_string += str1;
  sum_string += str2;
  return sum_string;
}

String operator+(const String str1, const char str2) {
  String sum_string;
  sum_string += str1;
  sum_string += str2;
  return sum_string;
}

bool operator<(const String &lhs, const String &rhs) {
  for (size_t i = 0; i < std::min(lhs.size(), rhs.size()) + 1; i++) {
    if (rhs[i] != lhs[i]) {
      return lhs[i] < rhs[i];
    }
  }
  return false;
}

bool operator>(const String &lhs, const String &rhs) {
  return rhs < lhs;
}

bool operator<=(const String &lhs, const String &rhs) {
  return !(rhs < lhs);
}

bool operator>=(const String &lhs, const String &rhs) {
  return !(lhs < rhs);
}

bool operator==(const String &lhs, const String &rhs) {
  if (lhs.size() == rhs.size()) {
    for (size_t i = 0; i < lhs.size(); i++) {
      if (lhs[i] != rhs[i]) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool operator!=(const String &lhs, const String &rhs) {
  return !(rhs == lhs);
}