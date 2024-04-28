#include <cmath>
#include <array>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <compare>
#include <numeric>

using std::cin;
using std::cerr;
using std::cout;
using std::vector;
using std::string;
using std::strong_ordering;
using std::istream;
using std::ostream;
using std::array;

struct BigInteger {
  static const int64_t kBase = 1e18;
  vector<int64_t> number;
  int signum;

  string toString() const {
    string string_value;
    if (signum == -1) {
      string_value = "-";
    }
    if (signum == 0 || number.empty()) {
      string_value = "0";
      return string_value;
    }
    string_value += std::to_string(number[number.size() - 1]);
    for (int i = number.size() - 2; i >= 0; i--) {
      int64_t value = number[i];
      int64_t max_d = kBase / 10;
      while (max_d > value) {
        string_value += "0";
        max_d /= 10;
      }
      if (value != 0) {
        string_value += std::to_string(value);
      }
    }
    return string_value;
  }

  BigInteger() {
    signum = 0;
    number.clear();
  }

  BigInteger(int input_number) {
    signum = 1;
    if (input_number == 0) {
      signum = 0;
    }
    if (input_number < 0) {
      signum = -1;
      input_number *= -1;
    }
    number.clear();
    number.push_back(input_number);
  }

  BigInteger(string str) {
    ToNull();
    signum = 1;
    if (!str.empty() && str[0] == '-') {
      signum = -1;
      str = str.substr(1, str.size() - 1);
    }
    for (unsigned i = 0; i < str.size(); i++) {
      if (str[i] > '9' || str[i] < '0') {
        str = str.substr(0, i);
      }
    }
    number.push_back(0);
    std::reverse(str.begin(), str.end());
    int ind = 0;
    int64_t degree = 1;
    for (unsigned i = 0; i < str.size(); i++) {
      number[ind] += degree * (str[i] - '0');
      degree *= 10;
      if (degree == BigInteger::kBase) {
        degree = 1;
        ind += 1;
        number.push_back(0);
      }
    }
    ClearNulls();
  }

  explicit operator bool() {
    return (signum != 0);
  }

  void ToNull() {
    number.clear();
    signum = 0;
  }

  void ClearNulls() {
    for (int i = number.size() - 1; i >= 0; i--) {
      if (number[i] == 0) {
        number.pop_back();
      } else {
        break;
      }
    }
    if (number.empty()) {
      signum = 0;
    }
  }

  bool operator<(const BigInteger& num) const {
    if (signum > num.signum) {
      return false;
    }
    if (signum < num.signum) {
      return true;
    }
    if (signum == 0) {
      return false;
    }
    if (number.size() > num.number.size()) {
      if (signum == -1) {
        return true;
      }
      return false;
    }
    if (number.size() < num.number.size()) {
      if (signum == -1) {
        return false;
      }
      return true;
    }
    int ans = 0;
    for (int i = number.size() - 1; i >= 0; i--) {
      if (number[i] < num.number[i]) {
        ans = 1;
        break;
      }
      if (number[i] > num.number[i]) {
        ans = -1;
        break;
      }
    }
    if (signum == -1) {
      return (ans == -1);
    }
    return (ans == 1);
  }

  bool operator>(const BigInteger& rhs) const {
    return rhs < *this;
  }

  bool operator<=(const BigInteger& rhs) const {
    return !(rhs < *this);
  }

  bool operator>=(const BigInteger& rhs) const {
    return !(*this < rhs);
  }

  bool operator==(const BigInteger& num) const { return (*this <= num && num <= *this); }

  bool operator!=(const BigInteger& num) const { return (!(*this == num)); }

  void swap(BigInteger& integer) {
    std::swap(number, integer.number);
    std::swap(signum, integer.signum);
  }

  BigInteger& operator-=(BigInteger num) {
    if (num.signum == 0) {
      return *this;
    }
    if (signum == 0) {
      swap(num);
      signum *= -1;
      return *this;
    }
    if (signum == num.signum) {
      if (signum == 1) {
        if (num > *this) {
          swap(num);
          signum *= -1;
          num.signum *= -1;
        }
      } else {
        if (num < *this) {
          swap(num);
          signum *= -1;
          num.signum *= -1;
        }
      }
      int64_t buffer = 0;
      int length = num.number.size();
      for (int i = 0; i < length; i++) {
        int64_t diff = number[i] - buffer - num.number[i];
        if (diff < 0) {
          number[i] = kBase + diff;
          buffer = 1;
        } else {
          number[i] = diff;
          buffer = 0;
        }
      }
      number[length] -= buffer;
      ClearNulls();
      if (number.empty()) {
        signum = 0;
      }
      return *this;
    } else {
      num.signum *= -1;
      *this += num;
      return *this;
    }
  }

  BigInteger& operator+=(BigInteger num) {
    if (num.signum == 0) {
      return *this;
    }
    if (signum == 0) {
      swap(num);
      return *this;
    }
    if (signum == num.signum) {
      int64_t buffer = 0;
      unsigned length = std::min(num.number.size(), number.size());
      for (unsigned i = 0; i < length; i++) {
        int64_t sum = buffer + num.number[i] + number[i];
        if (sum >= kBase) {
          number[i] = sum - kBase;
          buffer = 1;
        } else {
          number[i] = sum;
          buffer = 0;
        }
      }
      if (length == number.size()) {
        for (unsigned i = length; i < num.number.size(); i++) {
          int64_t sum = buffer + num.number[i];
          if (sum >= kBase) {
            number.push_back(sum - kBase);
            buffer = 1;
          } else {
            number.push_back(sum);
            buffer = 0;
          }
        }
        if (buffer != 0) {
          number.push_back(buffer);
        }
      } else {
        unsigned ind = length;
        while (buffer == 1 && ind < number.size()) {
          int64_t sum = buffer + number[ind];
          if (sum >= kBase) {
            number[ind] = (sum - kBase);
            buffer = 1;
          } else {
            number[ind] = (sum);
            buffer = 0;
          }
          ind += 1;
        }
        if (buffer == 1) {
          number.push_back(buffer);
        }
      }
      return *this;
    } else {
      num.signum *= -1;
      *this -= num;
      return *this;
    }
  }

  BigInteger& operator*=(int64_t num) {
    if (num == 0) {
      ToNull();
      return *this;
    }
    if (signum == 0) {
      return *this;
    }
    if (num < 0) {
      signum *= -1;
    }
    __int128 buffer = 0;
    for (unsigned i = 0; i < number.size(); i++) {
      __int128 mult = buffer + static_cast<__int128> (num) * number[i];
      buffer = mult / kBase;
      number[i] = static_cast<int64_t>(mult % kBase);
    }
    if (buffer != 0) {
      number.push_back(buffer);
    }
    return *this;
  }

  BigInteger& operator*=(BigInteger num) {
    if (num.signum == 0) {
      ToNull();
      return *this;
    }
    if (signum == 0) {
      return *this;
    }
    if (num.signum < 0) {
      signum *= -1;
    }
    BigInteger copy = *this;
    ToNull();
    for (int i = num.number.size() - 1; i >= 0; i--) {
      *this *= kBase;
      BigInteger mult = copy;
      mult *= num.number[i];
      *this += mult;
    }
    return *this;
  }

  BigInteger subINt(int len, int pos) {
    BigInteger ans(0);
    ans.signum = this->signum;
    ans.number.resize(len);
    for (int i = pos; i < pos + len; i++) {
      ans.number[i - pos] = this->number[i];
    }
    return ans;
  }

  BigInteger upper(int len) {
    std::reverse(number.begin(), number.end());
    for (int i = 0; i < len; i++) {
      number.push_back(0);
    }
    std::reverse(number.begin(), number.end());
    return *this;
  }

  BigInteger& operator/=(BigInteger num) {
    if (signum == 0) {
      return *this;
    }
    BigInteger ans(0);
    ans.number.clear();
    ans.signum = signum * num.signum;
    num.signum = 1;
    signum = 1;
    while (*this >= num) {
      int len = num.number.size();
      int64_t lef = 0;
      int64_t rig = kBase;
      BigInteger sub_b_int = subINt(len, number.size() - len);
      if (sub_b_int < num) {
        len += 1;
        sub_b_int = subINt(len, number.size() - len);
      }
      while (lef != rig - 1) {
        int64_t mid = (lef + rig) / 2;
        if (num * mid <= sub_b_int) {
          lef = mid;
        } else {
          rig = mid;
        }
      }
      ans.number.push_back(lef);
      int div_ans_degree_len = number.size() - len;
      BigInteger div_value = (num * lef).upper(div_ans_degree_len);
      *this -= div_value;
      if (*this == 0) {
        for (int i = 0; i < div_ans_degree_len; i++) {
          ans.number.push_back(0);
        }
        break;
      }
    }
    std::reverse(ans.number.begin(), ans.number.end());
    ans.ClearNulls();
    swap(ans);
    return *this;
  }

  BigInteger& operator%=(const BigInteger& num) {
    BigInteger copy = *this;
    copy /= num;
    copy *= num;
    *this -= copy;
    return *this;
  }

  BigInteger operator-() {
    BigInteger copy = *this;
    copy.signum *= -1;
    return copy;
  }

  BigInteger& operator++() {
    *this += 1;
    return *this;
  }

  BigInteger operator++(int) {
    BigInteger copy = *this;
    *this += 1;
    return copy;
  }

  BigInteger operator--() {
    *this -= 1;
    return *this;
  }

  BigInteger operator--(int) {
    BigInteger copy = *this;
    *this -= 1;
    return copy;
  }

  BigInteger operator*(const BigInteger& num) const {
    BigInteger ans = *this;
    ans *= num;
    return ans;
  }

  BigInteger operator*(int64_t num) const {
    BigInteger ans = *this;
    ans *= num;
    return ans;
  }

  BigInteger operator/(const BigInteger& num) const {
    BigInteger ans = *this;
    ans /= num;
    return ans;
  }

  BigInteger operator/(int64_t num) const {
    BigInteger ans = *this;
    ans /= num;
    return ans;
  }

  BigInteger operator%(const BigInteger& num) const {
    BigInteger ans = *this;
    ans %= num;
    return ans;
  }

  BigInteger operator%(int64_t num) const {
    BigInteger ans = *this;
    ans %= num;
    return ans;
  }

  BigInteger operator+(const BigInteger& num) const {
    BigInteger ans = *this;
    ans += num;
    return ans;
  }

  BigInteger operator+(int num) const {
    BigInteger ans = *this;
    ans += num;
    return ans;
  }

  BigInteger operator-(const BigInteger& num) const {
    BigInteger ans = *this;
    ans -= num;
    return ans;
  }

  BigInteger operator-(int num) const {
    BigInteger ans = *this;
    ans -= num;
    return ans;
  }
};

BigInteger operator*(int int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer * int2;
}

BigInteger operator/(int int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer / int2;
}

BigInteger operator+(int int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer + int2;
}

BigInteger operator-(int int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer - int2;
}

BigInteger operator%(int int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer % int2;
}

bool operator<(const int& int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer < int2;
}

bool operator>(const int& int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer > int2;
}

bool operator<=(const int& int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer <= int2;
}

bool operator>=(const int& int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer >= int2;
}

bool operator==(const int& int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer == int2;
}

bool operator!=(const int& int1, const BigInteger& int2) {
  BigInteger big_integer(int1);
  return big_integer != int2;
}

BigInteger operator ""_bi(unsigned long long num) {
  BigInteger ans(num);
  return ans;
}

istream& operator>>(istream& input, BigInteger& big_integer) {
  string str;
  input >> str;
  big_integer = str;
  return input;
}

ostream& operator<<(ostream& output, const BigInteger& big_integer) {
  output << big_integer.toString();
  return output;
}

BigInteger gcd(BigInteger int1, BigInteger int2) {
  if (int1 == 0) {
    return int2;
  }
  if (int2 == 0) {
    return int1;
  }
  int1.signum = 1;
  int2.signum = 1;
  if (int1 < int2) { int1.swap(int2); }
  while (int2 > 0) {
    int1 %= int2;
    int1.swap(int2);
  }
  return int1;
}

void Normalize(BigInteger& numerator, BigInteger& denumerator) {
  BigInteger common_part = gcd(numerator, denumerator);
  numerator /= common_part;
  denumerator /= common_part;
  numerator.signum *= denumerator.signum;
  denumerator.signum = 1;
}

struct Rational {
  BigInteger numerator;
  BigInteger denumerator;

  Rational() : numerator(0), denumerator(1) { Normalize(numerator, denumerator); }

  Rational(const BigInteger& numerator, const BigInteger& denumerator)
      : numerator(numerator), denumerator(denumerator) {}

  Rational(int numer, int denumer) : numerator(numer), denumerator(denumer) {
    Normalize(numerator, denumerator);
  }

  Rational(int numerator) : numerator(numerator), denumerator(1_bi) {}

  Rational(const BigInteger& numerator) : numerator(numerator), denumerator(1_bi) {}

  string toString() const {
    string ans = numerator.toString();
    if (denumerator != 1) {
      ans += "/" + denumerator.toString();
    }
    return ans;
  }

  explicit operator double() {
    double num = std::stod(numerator.toString());
    double denum = std::stod(denumerator.toString());
    return num / denum;
  }

  Rational& operator+=(Rational rational) {
    BigInteger denum_gcd = gcd(denumerator, rational.denumerator);
    denumerator /= denum_gcd;
    numerator = (numerator * rational.denumerator) / denum_gcd + (rational.numerator * denumerator);
    denumerator *= rational.denumerator;
    Normalize(numerator, denumerator);
    return *this;
  }

  Rational& operator-=(Rational rational) {
    BigInteger denum_gcd = gcd(denumerator, rational.denumerator);
    denumerator /= denum_gcd;
    numerator = (numerator * rational.denumerator) / denum_gcd - (rational.numerator * denumerator);
    denumerator *= rational.denumerator;
    Normalize(numerator, denumerator);
    return *this;
  }

  Rational& operator*=(Rational rational) {
    Normalize(numerator, rational.denumerator);
    Normalize(denumerator, rational.numerator);
    numerator *= rational.numerator;
    denumerator *= rational.denumerator;
    Normalize(numerator, denumerator);
    return *this;
  }

  Rational& operator/=(Rational rational) {
    Normalize(numerator, rational.numerator);
    Normalize(denumerator, rational.denumerator);
    denumerator *= rational.numerator;
    numerator *= rational.denumerator;
    Normalize(numerator, denumerator);
    return *this;
  }

  Rational operator+(const Rational& rational) const {
    Rational copy = *this;
    return copy += rational;
  }

  Rational operator-(const Rational& rational) const {
    Rational copy = *this;
    return copy -= rational;
  }

  Rational operator*(const Rational& rational) const {
    Rational copy = *this;
    return copy *= rational;
  }

  Rational operator/(const Rational& rational) const {
    Rational copy = *this;
    return copy /= rational;
  }

  Rational operator-() {
    Rational copy = *this;
    copy.numerator.signum *= -1;
    return copy;
  }

  bool operator<(const Rational& rhs) const {
    return numerator * rhs.denumerator < denumerator * rhs.numerator;
  }

  bool operator>(const Rational& rhs) const { return rhs < *this; }

  bool operator<=(const Rational& rhs) const { return !(rhs < *this); }

  bool operator>=(const Rational& rhs) const { return !(*this < rhs); }

  bool operator==(const Rational& rhs) const { return *this <= rhs && *this >= rhs; }

  bool operator!=(const Rational& rhs) const { return !(rhs == *this); }

  string asDecimal(size_t precision = 0) {
    Rational copy = *this;
    string ans;
    if (numerator.signum == -1) {
      ans += "-";
      numerator.signum = 1;
    }
    if (precision == 0) {
      if (numerator >= denumerator) {
        ans += "1";
        return ans;
      } else {
        return "0";
      }
    }
    if (numerator > denumerator) {
      ans += (numerator / denumerator).toString();
      numerator %= denumerator;
      ans += ".";
    } else {
      ans += "0.";
    }
    for (unsigned i = 0; i < precision; i++) {
      numerator *= 10;
      ans += (numerator / denumerator).toString();
      numerator %= denumerator;
    }
    *this = copy;
    return ans;
  }
};

Rational operator+(int num_1, const Rational& num_2) {
  Rational rational(num_1);
  return rational + num_2;
}

Rational operator-(int num_1, const Rational& num_2) {
  Rational rational(num_1);
  return rational - num_2;
}

Rational operator*(int num_1, const Rational& num_2) {
  Rational rational(num_1);
  return rational * num_2;
}

Rational operator/(int num_1, const Rational& num_2) {
  Rational rational(num_1);
  return rational / num_2;
}

ostream& operator<<(ostream& output, const Rational& rational) {
  return output << rational.numerator << "/" << rational.denumerator;
}

istream& operator>>(istream& input, Rational& rational) {
  string inp;
  input >> inp;
  string numer = "";
  string denumer = "";
  bool fl = false;
  for (auto ch : inp) {
    if (ch == '/') {
      fl = true;
    } else {
      if (fl) {
        denumer += ch;
      } else {
        numer += ch;
      }
    }
  }
  rational.numerator = numer;
  if (fl) {
    rational.denumerator = denumer;
  } else {
    rational.denumerator = 1;
  }
  Normalize(rational.numerator, rational.denumerator);
  return input;
}

constexpr bool is_prime(size_t N) {
  for (size_t i = 2; i * i <= N; i++) {
    if (N % i == 0) {
      return false;
    }
  }
  return true;
}

long long FastPowM(int val, int pow, int mod) {
  if (pow == 0) {
    return 1;
  }
  if (pow == 1) {
    return val % mod;
  }
  if (pow % 2 == 0) {
    return (FastPowM(val, pow / 2, mod) * FastPowM(val, pow / 2, mod)) % mod;
  }
  return (FastPowM(val, pow - 1, mod) * val) % mod;
}

template <size_t N>
struct Residue {
  int val = 0;

  Residue() {
    val = 0;
  };

  explicit Residue(int val) : val((static_cast<int>(N) + (val % static_cast<int>(N))) % N) {};

  explicit operator int() {
    return val;
  }

  Residue& operator+=(const Residue& other) {
    val += other.val;
    val = val % N;
    return *this;
  }

  Residue& operator-=(const Residue& other) {
    val -= other.val;
    val = (N + val) % N;
    return *this;
  }

  Residue& operator*=(const Residue& other) {
    val *= other.val;
    val = val % N;
    return *this;
  }

  Residue& operator/=(const Residue& other) {
    static_assert(is_prime(N));
    int multiplyer = FastPowM(other.val, N - 2, N);
    return *this *= Residue(multiplyer);
  }

  Residue operator+(const Residue& residue) const {
    Residue copy = *this;
    return copy += residue;
  }

  Residue operator-(const Residue& residue) const {
    Residue copy = *this;
    return copy -= residue;
  }

  Residue operator*(const Residue& residue) const {
    Residue copy = *this;
    return copy *= residue;
  }

  Residue operator/(const Residue& residue) const {
    Residue copy = *this;
    return copy /= residue;
  }

  Residue operator+(int num) const {
    Residue copy = *this;
    return copy += num;
  }

  Residue operator-(int num) const {
    Residue copy = *this;
    return copy -= num;
  }

  Residue operator*(int num) const {
    Residue copy = *this;
    return copy *= num;
  }

  Residue operator/(int num) const {
    Residue copy = *this;
    return copy /= num;
  }

  Residue operator-() {
    int ans_val = (val ? N - val : val);
    return Residue(ans_val);
  }

  bool operator==(const Residue& other) const {
    return val == other.val;
  }

  bool operator!=(const Residue& other) const {
    return val != other.val;
  }

  bool operator==(int other) const {
    return *this == Residue(other);
  }

  bool operator!=(int other) const {
    return *this != Residue(other);
  }
};

template <size_t N>
bool operator==(int num1, const Residue<N>& num2) {
  return num2 == num1;
}

template <size_t N>
bool operator!=(int num1, const Residue<N>& num2) {
  return num2 != num1;
}

template <size_t N>
Residue<N> operator+(int num_1, const Residue<N>& num_2) {
  Residue<N> residue(num_1);
  return residue + num_2;
}

template <size_t N>
Residue<N> operator-(int num_1, const Residue<N>& num_2) {
  Residue<N> residue(num_1);
  return residue - num_2;
}

template <size_t N>
Residue<N> operator*(int num_1, const Residue<N>& num_2) {
  Residue<N> residue(num_1);
  return residue * num_2;
}

template <size_t N>
Residue<N> operator/(int num_1, const Residue<N>& num_2) {
  Residue<N> residue(num_1);
  return residue / num_2;
}

template <size_t N>
ostream& operator<<(ostream& output, const Residue<N>& num_2) {
  output << num_2.val << "mod" << N;
  return output;
}

template <size_t M, size_t N, typename Field=Rational>
struct Matrix {
  array<array<Field, N>, M> table;
  Matrix() = default;

  template <typename T>
  Matrix(std::initializer_list<std::initializer_list<T>> list) {
    int i = 0;
    for (auto item : list) {
      int j = 0;
      for (T t : item) {
        table[i][j] = Field(t);
        j++;
      }
      i++;
    }
  }

  void PrintMatrix() const {
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        cerr << table[i][j] << " ";
      }
      cerr << std::endl;
    }
    cerr << std::endl;
  }

  static Matrix unityMatrix() {
    static_assert(N == M);
    Matrix ans;
    for (size_t i = 0; i < N; i++) {
      ans.table[i][i] = Field(1);
    }
    return ans;
  }

  array<Field, N>& operator[](size_t pos) {
    return table[pos];
  }

  const array<Field, N>& operator[](size_t pos) const {
    return table[pos];
  }

  template <size_t M1, size_t N1, typename Field1>
  Matrix& operator+=(const Matrix<M1, N1, Field1>& other) {
    static_assert(M1 == M && N1 == N);
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        table[i][j] += other.table[i][j];
      }
    }

    return *this;
  }

  template <size_t M1, size_t N1, typename Field1>
  Matrix operator+(const Matrix<M1, N1, Field1>& other) const {
    Matrix copy = *this;
    return copy += other;
  }

  template <size_t M1, size_t N1, typename Field1>
  Matrix& operator-=(const Matrix<M1, N1, Field1>& other) {
    static_assert(M1 == M && N1 == N);
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        table[i][j] -= other.table[i][j];
      }
    }

    return *this;
  }

  template <size_t M1, size_t N1, typename Field1>
  Matrix operator-(const Matrix<M1, N1, Field1>& other) const {
    Matrix copy = *this;
    return copy -= other;
  }

  template <size_t M1, size_t N1, typename Field1>
  Matrix& operator*=(const Matrix<M1, N1, Field1>& other) {
    *this = *this * other;
    return *this;
  }

  template <size_t M1, size_t N1, typename Field1>
  Matrix<M, N1, Field> operator*(const Matrix<M1, N1, Field1>& other) const {
    static_assert(M1 == N);
    Matrix<M, N1, Field> matrix_copy;
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N1; j++) {
        Field table_ij = Field(0);
        for (size_t w = 0; w < M1; w++) {
          table_ij += table[i][w] * other[w][j];
        }
        matrix_copy[i][j] = table_ij;
      }
    }
    return matrix_copy;
  }

  Matrix& operator*=(const Field& number) {
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        table[i][j] *= number;
      }
    }
    return *this;
  }

  Matrix operator*(const Field& number) const {
    Matrix copy = *this;
    copy *= number;
    return copy;
  }

  Matrix operator-() const {
    Matrix copy = *this;
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        copy.table[i][j] = -(copy.table[i][j]);
      }
    }
  }

  struct Modification {
    size_t first;
    size_t second;
    Field mult;
  };

  void DoModification(Modification modif) {
    for (size_t i = 0; i < N; i++) {
      table[modif.first][i] += table[modif.second][i] * modif.mult;
    }
  }

  vector<Modification> Gauss() {
    size_t num_next_notnull = M;
    vector<Modification> ans;
    for (size_t i = 0; i < M; i++) {
      if (table[i][0] != 0) {
        num_next_notnull = i;
      }
    }
    for (size_t i = 0; i < N; i++) {
      if (table[i][i] == Field(0)) {
        if (num_next_notnull != M) {
          Modification modification{i, num_next_notnull, Field(-1)};
          ans.push_back(modification);
          DoModification(modification);
        }
      }
      if (table[i][i] == Field(0)) {
        continue;
      }
      num_next_notnull = M;
      for (size_t j = i + 1; j < M; j++) {
        if (table[j][i] != Field(0)) {
          Modification modification{j, i, -(table[j][i] / table[i][i])};
          ans.push_back(modification);
          DoModification(modification);
        }
        if (i != N - 1 && table[j][i + 1] != Field(0)) {
          num_next_notnull = std::min(num_next_notnull, j);
        }
      }
    }
    return ans;
  }

  vector<Modification> GaussToSimplified() {
    size_t num_next_notnull = M;
    vector<Modification> ans;
    for (size_t i = 0; i < M; i++) {
      if (table[i][0] != 0) {
        num_next_notnull = i;
      }
    }
    for (size_t i = 0; i < N; i++) {
      if (table[i][i] == Field(0)) {
        if (num_next_notnull != M) {
          Modification modification{i, num_next_notnull, Field(-1)};
          ans.push_back(modification);
          DoModification(modification);
        }
      }
      if (table[i][i] == Field(0)) {
        continue;
      }
      num_next_notnull = M;
      for (size_t j = 0; j < M; j++) {
        if (table[j][i] != Field(0) && j != i) {
          Modification modification{j, i, -(table[j][i] / table[i][i])};
          ans.push_back(modification);
          DoModification(modification);
          if (j > i) {
            if (i != N - 1 && table[j][i] != Field(0)) {
              num_next_notnull = std::min(num_next_notnull, j);
            }
          }
        }
      }
    }
    return ans;
  }

  Field det() const {
    static_assert(N == M);
    Matrix copy = *this;
    copy.Gauss();
    Field ans = Field(1);
    for (size_t i = 0; i < N; i++) {
      ans *= copy[i][i];
    }
    return ans;
  }

  Matrix<N, M, Field> transposed() const {
    Matrix<N, M, Field> copy;
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        copy[j][i] = table[i][j];
      }
    }
    return copy;
  }

  size_t rank() const {
    Matrix copy = *this;
    copy.Gauss();
    size_t counter = 0;
    for (int i = M - 1; i >= 0; i--) {
      bool is_null = true;
      for (size_t j = 0; j < N; j++) {
        if (copy[i][j] != Field(0)) { is_null = false; }
      }
      if (is_null) {
        counter++;
      } else {
        break;
      }
    }
    return (M - counter);
  }

  Matrix inverted() const {
    Matrix copy = *this;
    static_assert(M == N);
    Matrix result = Matrix::unityMatrix();
    auto mods = copy.GaussToSimplified();
    for (auto& mod : mods) {
      result.DoModification(mod);
    }
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        result[i][j] /= copy[i][i];
      }
    }
    return result;
  }

  void invert() {
    *this = inverted();
  }

  Field trace() const {
    static_assert(M == N);
    Field ans = Field(0);
    for (size_t i = 0; i < N; i++) {
      ans += table[i][i];
    }
    return ans;
  }

  array<Field, N> getRow(size_t pos) const {
    return (*this)[pos];
  }

  array<Field, M> getColumn(size_t pos) const {
    array<Field, M> answer;
    for (size_t i = 0; i < M; i++) {
      answer[i] = table[i][pos];
    }
    return answer;
  }

  template <size_t M1, size_t N1, typename Field1 = Rational>
  bool operator==(const Matrix<M1, N1, Field1>& other) const {
    if (N1 != N || M1 != M) {
      return false;
    }
    bool answer = true;
    for (size_t i = 0; i < M; i++) {
      for (size_t j = 0; j < N; j++) {
        if (table[i][j] != other[i][j]) {
          answer = false;
        }
      }
    }
    return answer;
  }

  template <size_t M1, size_t N1, typename Field1 = Rational>
  bool operator!=(const Matrix<M1, N1, Field1>& other) const {
    return !(*this == other);
  }
};

template <size_t M, size_t N, typename Field = Rational>
Matrix<M, N, Field> operator*(const Field& number, const Matrix<M, N, Field>& matrix) {
  return matrix * number;
}

template <size_t N, typename Field = Rational>
using SquareMatrix = Matrix<N, N, Field>;