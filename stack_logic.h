#include <iostream>

using std::cin;
using std::cout;
char** storage;
int rl_size = 4;
int dt_size = 0;
int im_size = 0;

void Delete(int size) {
  for (int i = 0; i < size; i++) {
    delete[] storage[i];
  }
  delete[] storage;
}

char* Copy(const char* string, int size) {
  char* s_copy = new char[size];
  for (int i = 0; i < size; i++) {
    s_copy[i] = string[i];
  }
  return s_copy;
}

void Resize() {
  rl_size *= 2;
  char** storage_cpy = new char*[rl_size];
  for (int i = 0; i < im_size; i++) {
    storage_cpy[i] = Copy(storage[i], 256);
  }
  Delete(im_size);
  storage = storage_cpy;
}

void Push(char* element) {
  if (im_size == rl_size) {
    Resize();
  }
  if (dt_size > im_size) {
    delete[] storage[im_size];
  }
  storage[im_size++] = Copy(element, 256);
  if (im_size > dt_size) {
    dt_size++;
  }
}

void Initialize() {
  dt_size = 0;
  im_size = 0;
  rl_size = 4;
  storage = new char*[4];
}

void Clear() {
  Delete(dt_size);
  Initialize();
  cout << "ok" << '\n';
}

void Pop() {
  if (im_size == 0) {
    cout << "error" << '\n';
  } else {
    cout << storage[--im_size] << '\n';
  }
}
void Back() {
  if (im_size == 0) {
    cout << "error" << '\n';
  } else {
    cout << storage[im_size - 1] << '\n';
  }
}
void Print() {
  for (int i = 0; i < im_size; i++) {
    cout << storage[i] << " ";
  }
  cout << '\n';
}

int main() {
  Initialize();
  char inp[10];
  cin >> inp;
  while (inp[0] != 'e') {
    switch (inp[0]) {
      case 'p':
        if (inp[1] == 'u') {
          char value[256];
          cin >> value;
          Push(value);
          cout << "ok" << '\n';
        } else {
          Pop();
        }
        break;
      case 'b':
        Back();
        break;
      case 's':
        cout << im_size << '\n';
        break;
      case 'c':
        Clear();
        break;
    }
    cin >> inp;
  }
  Delete(dt_size);
  cout << "bye";
}