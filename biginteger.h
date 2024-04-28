#include <assert.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <compare>
#include <numeric>

using std::cin;
using std::cout;
using std::vector;
using std::string;
using std::istream;
using std::ostream;
using std::cerr;
using std::endl;

enum class Sign {
  Negative = -1,
  Null = 0,
  Positive = 1
};

void operator*=(Sign& sign, const Sign& sign2) {
  sign = static_cast<Sign>(static_cast<int>(sign) * static_cast<int>(sign2));
}

Sign operator*(const Sign& sign1, const Sign& sign2) {
  Sign ans = Sign::Positive;
  ans *= sign1;
  ans *= sign2;
  return ans;
}

struct BigInteger;

bool operator<(const BigInteger& num1, const BigInteger& num2);

bool operator>(const BigInteger& num1, const BigInteger& num2);

bool operator<=(const BigInteger& num1, const BigInteger& num2);

bool operator>=(const BigInteger& num1, const BigInteger& num2);

bool operator==(const BigInteger& num1, const BigInteger& num2);

bool operator!=(const BigInteger& num1, const BigInteger& num2);

struct BigInteger {
  static const int64_t kBase = 1e9;
  vector<int32_t> number;
  Sign signum;

  string toString() const {
    string string_value;
    if (signum == Sign::Negative) {
      string_value = "-";
    }
    if (signum == Sign::Null || number.empty()) {
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
    signum = Sign::Null;
    number.clear();
  }

  BigInteger(int input_number) {
    signum = Sign::Positive;
    if (input_number == 0) {
      signum = Sign::Null;
    }
    if (input_number < 0) {
      signum = Sign::Negative;
      input_number *= -1;
    }
    number.clear();
    if (input_number >= kBase) {
      number.push_back(input_number % kBase);
      number.push_back(input_number / kBase);
    } else {
      number.push_back(input_number);
    }
  }

  BigInteger(string str) {
    toNull();
    signum = Sign::Positive;
    if (!str.empty() && str[0] == '-') {
      signum = Sign::Negative;
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
    clearNulls();
  }

  explicit operator bool() {
    return (signum != Sign::Null);
  }

  void toNull() {
    number.clear();
    signum = Sign::Null;
  }

  void clearNulls() {
    for (size_t i = 0; i < number.size(); i++) {
      size_t ind = number.size() - 1 - i;
      if (number[ind] == 0) {
        number.pop_back();
      } else {
        break;
      }
    }
    if (number.empty()) {
      signum = Sign::Null;
    }
  }

  void swap(BigInteger& integer) {
    std::swap(number, integer.number);
    std::swap(signum, integer.signum);
  }

  BigInteger subInt(int len, int pos) {
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

  BigInteger& operator-=(const BigInteger& num) {
    if (num.signum == Sign::Null) {
      return *this;
    }
    if (signum == Sign::Null) {
      *this = num;
      signum *= Sign::Negative;
      return *this;
    }
    if (signum != num.signum) {
      signum *= Sign::Negative;
      *this += num;
      signum *= Sign::Negative;
      return *this;
    }
    if (num == *this) {
      toNull();
      return *this;
    }
    if ((signum == Sign::Positive) != (num < *this)) {
      *this = -(num - *this);
      return *this;
    }

    // после всех проверок выше мы работаем с двумя ненулевыми числами одного знака, притом *this по модулю больше чем num
    int64_t buffer = 0;
    size_t length = number.size();
    for (size_t i = 0; i < length; i++) {
      int64_t num_number_i = 0;
      if (num.number.size() > i) {
        num_number_i = num.number[i];
      }
      int64_t diff = number[i] - buffer - num_number_i;
      if (diff < 0) {
        number[i] = kBase + diff;
        buffer = 1;
      } else {
        number[i] = diff;
        buffer = 0;
      }
    }
    clearNulls();
    if (number.empty()) {
      signum = Sign::Null;
    }
    return *this;
  }

  BigInteger& operator+=(BigInteger num) {
    if (num.signum == Sign::Null) {
      return *this;
    }
    if (signum == Sign::Null) {
      swap(num);
      return *this;
    }
    if (signum != num.signum) {
      signum *= Sign::Negative;
      *this -= num;
      signum *= Sign::Negative;
      return *this;
    }
    // сложение двух чисел одного знака
    int64_t buffer = 0;
    size_t length = std::min(num.number.size(), number.size());
    //сложение до последней цифры кратчайшего
    for (size_t i = 0; i < length; i++) {
      int64_t sum = buffer + num.number[i] + number[i];
      if (sum >= kBase) {
        number[i] = sum - kBase;
        buffer = 1;
      } else {
        number[i] = sum;
        buffer = 0;
      }
    }
    if (length == number.size()) {      // прибавление оставшихся цифр если кратчайшее - this
      for (size_t ind = length; ind < num.number.size(); ind++) {
        int64_t sum = buffer + num.number[ind];
        number.push_back((sum >= kBase ? sum - kBase : sum));
        buffer = (sum >= kBase ? 1 : 0);
      }
      if (buffer != 0) {
        number.push_back(buffer);
      }
    } else {
      size_t ind = length;
      while (buffer == 1 && ind < number.size()) {
        int64_t sum = buffer + number[ind];
        number[ind] = (sum >= kBase ? sum - kBase : sum);
        buffer = (sum >= kBase ? 1 : 0);
        ind += 1;
      }
      if (buffer != 0) {
        number.push_back(buffer);
      }
    }
    return *this;
  }
  // умножение на цифру
  BigInteger& operator*=(int64_t num) {
    if (!number.empty() && num == kBase) {
      number.insert(number.begin(), 0);
      return *this;
    }
    if (num == 0) {
      toNull();
      return *this;
    }
    if (signum == Sign::Null) {
      return *this;
    }
    if (num < 0) {
      signum *= Sign::Negative;
    }
    // перемножение ненулевых чисел с уже учтёнными знаками
    int64_t buffer = 0;
    for (unsigned i = 0; i < number.size(); i++) {
      int64_t mult = buffer + num * number[i];
      buffer = mult / kBase;
      number[i] = (mult % kBase);
    }
    if (buffer != 0) {
      number.push_back(buffer);
    }
    return *this;
  }

  // умножение на число
  BigInteger& operator*=(const BigInteger& num) {
    if (num.signum == Sign::Null) {
      toNull();
      return *this;
    }
    if (signum == Sign::Null) {
      return *this;
    }
    if (num.signum == Sign::Negative) {
      signum *= Sign::Negative;
    }
    BigInteger copy = *this;
    toNull();
    for (int i = num.number.size() - 1; i >= 0; i--) {
      *this *= kBase;
      BigInteger mult = copy;
      mult *= num.number[i];
      *this += mult;
    }
    return *this;
  }

  BigInteger& operator/=(BigInteger num) {
    if (signum == Sign::Null) {
      return *this;
    }
    BigInteger ans(0);
    ans.number.clear();
    ans.signum = signum * num.signum;
    num.signum = Sign::Positive;
    signum = Sign::Positive;
    // деление столбиком
    while (*this >= num) {
      int len = num.number.size();
      int64_t lef = 0;
      int64_t rig = kBase;
      // взятие куска первых цифр нужной длинны
      BigInteger sub_b_int = subInt(len, number.size() - len);
      if (sub_b_int < num) {
        len += 1;
        sub_b_int = subInt(len, number.size() - len);
      }
      // бинпоиск результата деления этого куска на делитель
      while (lef != rig - 1) {
        int64_t mid = (lef + rig) / 2;
        (num * mid <= sub_b_int ? lef = mid : rig = mid);
      }
      ans.number.push_back(lef);
      // вычитание из делимого (частное * делитель) с нужным сдвигом
      int div_ans_degree_len = number.size() - len;
      BigInteger div_value = (num * lef).upper(div_ans_degree_len);
      *this -= div_value;
      if (*this != 0) {
        continue;
      }
      for (int i = 0; i < div_ans_degree_len; i++) {
        ans.number.push_back(0);
      }
      break;
    }
    std::reverse(ans.number.begin(), ans.number.end());
    ans.clearNulls();
    swap(ans);
    return *this;
  }

  BigInteger& operator%=(const BigInteger& num) {
    BigInteger copy = *this;
    copy /= num;
    copy *= num;
    *this -= copy;
    clearNulls();
    return *this;
  }

  BigInteger operator-() {
    BigInteger copy = *this;
    copy.signum *= Sign::Negative;
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

bool operator<(const BigInteger& num1, const BigInteger& num2) {
  if (num1.signum != num2.signum) {
    return num1.signum < num2.signum;
  }
  if (num1.signum == Sign::Null) {
    return false;
  }
  Sign ans = Sign::Null;
  if (num1.number.size() != num2.number.size()) {
    ans = (num1.number.size() < num2.number.size() ? Sign::Positive : Sign::Negative);
    return (num1.signum == Sign::Negative ? ans == Sign::Negative : ans == Sign::Positive );
  }

  for (int i = num1.number.size() - 1; i >= 0; i--) {
    if (num1.number[i] != num2.number[i]) {
      ans = (num1.number[i] < num2.number[i] ? Sign::Positive : Sign::Negative);
      break;
    }
  }
  return (num1.signum == Sign::Negative ? ans == Sign::Negative : ans == Sign::Positive);
}

bool operator>(const BigInteger& num1, const BigInteger& num2) {
  return num2 < num1;
}

bool operator<=(const BigInteger& num1, const BigInteger& num2) {
  return !(num2 < num1);
}

bool operator>=(const BigInteger& num1, const BigInteger& num2) {
  return !(num1 < num2);
}

bool operator==(const BigInteger& num1, const BigInteger& num2) { return (num1 <= num2 && num2 <= num1); }

bool operator!=(const BigInteger& num1, const BigInteger& num2) { return (!(num1 == num2)); }

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
  int1.signum = Sign::Positive;
  int2.signum = Sign::Positive;
  if (int1 < int2) { int1.swap(int2); }
  while (int2 > 0) {
    int1 %= int2;
    int1.swap(int2);
  }
  return int1;
}

void beautify(BigInteger& numerator, BigInteger& denumerator) {
  BigInteger common_part = gcd(numerator, denumerator);
  numerator /= common_part;
  denumerator /= common_part;
  numerator.signum *= denumerator.signum;
  denumerator.signum = Sign::Positive;
}

struct Rational {
  BigInteger numerator;
  BigInteger denumerator;

  Rational() : numerator(0), denumerator(1) {}

  Rational(const BigInteger& numerator, const BigInteger& denumerator)
      : numerator(numerator), denumerator(denumerator) {}

  Rational(int numer, int denumer) : numerator(numer), denumerator(denumer) {
    beautify(numerator, denumerator);
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

  Rational& operator+=(const Rational& rational) {
    numerator = (numerator * rational.denumerator) + (rational.numerator * denumerator);
    denumerator *= rational.denumerator;
    beautify(numerator, denumerator);
    return *this;
  }

  Rational& operator-=(const Rational& rational) {
    numerator = (numerator * rational.denumerator) - (rational.numerator * denumerator);
    denumerator *= rational.denumerator;
    beautify(numerator, denumerator);
    return *this;
  }

  Rational& operator*=(const Rational& rational) {
    numerator *= rational.numerator;
    denumerator *= rational.denumerator;
    beautify(numerator, denumerator);
    return *this;
  }

  Rational& operator/=(const Rational& rational) {
    denumerator *= rational.numerator;
    numerator *= rational.denumerator;
    beautify(numerator, denumerator);
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
    copy.numerator.signum *= Sign::Negative;
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
    if (numerator.signum == Sign::Negative) {
      ans += "-";
      numerator.signum = Sign::Positive;
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
    for (size_t i = 0; i < precision; i++) {
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
        denumer+=ch;
      } else {
        numer+=ch;
      }
    }
  }
  rational.numerator = numer;
  if (fl) {
    rational.denumerator = denumer;
  } else {
    rational.denumerator = 1;
  }
  beautify(rational.numerator, rational.denumerator);
  return input;
}

/*
В общем ок, оставшиеся замечания:
86: - отступ

138: - для того, чтобы не возиться с одинаковым определением operator< как
в 517: и далее - можно и нужно вынести сравнение из класса. В таком случае неявные касты (В твоём случае конструктор от int) сделают всё остальное. Ну и разумеется все операторы ниже удалить (тоже заменить на внешние на operator>= (const BigInteger&, const BigInteger&)

Нейминг - если используешь CamelCase c большой то и везде методы должны быть с большой буквы. Исправь опечатки в комментариях

Из-за того, что ты вызываешь -= из += и наоборот, у тебя происходит много лишних копий num. Лишние вызовы обычно и являются узким местом всех программ на C++, поэтому в данном случае это надо пофиксить (В твоей реализации, на самом деле менять num не надо, можно обойтись сменой знака у главного числа и вообще не копировать num)

 */