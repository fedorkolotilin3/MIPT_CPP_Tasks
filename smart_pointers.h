#ifndef CPPTASKS__SMART_POINTERS_H_
#define CPPTASKS__SMART_POINTERS_H_

template <class T>
class WeakPtr;
template <class T>
class SharedPtr;
template <class T>
class EnableSharedFromThis {
 public:
  mutable WeakPtr<T> weak_this;
 public:
  SharedPtr<T> shared_from_this() {
    if (weak_this.control_block == nullptr || weak_this.expired()) {
      throw std::bad_exception();
    }
    return weak_this.lock();
  }
};

class BaseControlBlock {
 public:
  size_t shared_counter = 0;
  size_t weak_counter = 0;
  virtual void* get_ptr() {
    return nullptr;
  }
  virtual void delete_resource() {};
  virtual void suicide() {};
  virtual ~BaseControlBlock() = default;
  void increase_s_counter() {
    shared_counter++;
  }
  void decrease_s_counter() {
    shared_counter--;
    if (shared_counter == 0) {
      if (weak_counter == 0) {
        delete_resource();
        suicide();
        return;
      }
      delete_resource();
    }
  }
  void increase_w_counter() {
    weak_counter++;
  }
  void decrease_w_counter() {
    weak_counter--;
    if (shared_counter == 0 && weak_counter == 0) {
      suicide();
    }
  }

};

template <class T, class Deleter = std::default_delete<T>, class Alloc = std::allocator<T>>
class ControlBlockPtr : public BaseControlBlock {
  T* res;
  Alloc alloc;
  Deleter deleter;
 public:
  void* get_ptr() override {
    return res;
  }

  void delete_resource() override {
    deleter(res);
    res = nullptr;
  }
  ControlBlockPtr() : res(nullptr) {}
  ControlBlockPtr(T* other_ptr, Deleter deleter= Deleter(), Alloc alloc = Alloc()) : alloc(alloc), deleter(deleter) {
    res = other_ptr;
    shared_counter = 0;
    weak_counter = 0;
  }

  void suicide() override {
    this->~ControlBlockPtr();
    using CBPtrAllocTraits = std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockPtr>;
    CBPtrAllocTraits{alloc}.deallocate(this, 1);
  }

  ~ControlBlockPtr() override = default;
};
template <class T, class Alloc = std::allocator<T>>
class ControlBlockObj : public BaseControlBlock {
  T res;
  T* res_ptr = &res;
 public:
  template <typename ...Args>
  ControlBlockObj(Args... args) : res(std::forward<Args>(args)...) {}
  void* get_ptr() override {
    return res_ptr;
  }

  void delete_resource() override {
    std::allocator_traits<Alloc>::destroy(alloc, res_ptr);
    res_ptr = nullptr;
  }
  void suicide() override {
    using CBPtrAlloc = std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockObj>;
    auto correct_alloc = CBPtrAlloc{alloc};
    CBPtrAlloc{alloc}.deallocate(this, 1);
  }

  ~ControlBlockObj() override {
  }
  Alloc alloc;
};

template <class T>
class WeakPtr {
 public:
  T* ptr;
  BaseControlBlock* control_block;

  size_t use_count() const {
    return control_block->shared_counter;
  }
  bool expired() const {
    if (control_block->get_ptr() == nullptr) {
      return true;
    }
    return false;
  }
  SharedPtr<T> lock() const {
    if (!expired()) {
      return SharedPtr<T>(control_block);
    }
    return SharedPtr<T>();
  }
  template <typename ...Args>
  void reset(Args&&... args) {
    WeakPtr(args...).swap(*this);
  }

  WeakPtr(): ptr(nullptr), control_block(nullptr) {}
  ~WeakPtr() {
    if (control_block != nullptr) {
      control_block->decrease_w_counter();
    }
  }
  template <class Y>
  WeakPtr(Y* other_ptr) {
    ptr = other_ptr;
    control_block = new ControlBlockPtr<Y>(other_ptr);
    if (control_block != nullptr) {
      control_block->increase_w_counter();
    }
  }

  template <class Y>
  WeakPtr(const SharedPtr<Y>& other) {
    control_block = other.control_block;
    ptr = other.ptr;
    if (control_block != nullptr) {
      control_block->increase_w_counter();
    }
  }
  WeakPtr(const WeakPtr<T>& other) {
    control_block = other.control_block;
    ptr = other.ptr;
    if (control_block != nullptr) {
      control_block->increase_w_counter();
    }
  }
  WeakPtr(WeakPtr<T>&& other) {
    ptr = other.ptr;
    control_block = other.control_block;
    other.ptr = nullptr;
    other.control_block = nullptr;
  }
  template <class Y>
  WeakPtr(const WeakPtr<Y>& other) {
    control_block = other.control_block;
    ptr = other.ptr;
    if (control_block != nullptr) {
      control_block->increase_w_counter();
    }
  }
  template <class Y>
  WeakPtr(WeakPtr<Y>&& other) {
    ptr = other.ptr;
    control_block = other.control_block;
    other.ptr = nullptr;
    other.control_block = nullptr;
  }
  WeakPtr& operator=(const WeakPtr<T>& other) {
    if (control_block != nullptr) {
      control_block->decrease_w_counter();
    }
    new (this) WeakPtr(other);
    if (control_block != nullptr) {
      control_block->increase_w_counter();
    }
    return *this;
  }
  WeakPtr& operator=(WeakPtr<T>&& other) {
    if (control_block != nullptr) {
      control_block->decrease_w_counter();
    }
    new (this) WeakPtr(std::move(other));
    other.control_block = nullptr;
    other.ptr = nullptr;
    return *this;
  }
  template <class Y>
  WeakPtr& operator=(const WeakPtr<Y>& other) {
    if (control_block != nullptr) {
      control_block->decrease_w_counter();
    }
    new (this) WeakPtr(other);
    control_block->increase_w_counter();
    return *this;
  }
  template <class Y>
  WeakPtr& operator=(WeakPtr<Y>&& other) {
    if (control_block != nullptr) {
      control_block->decrease_w_counter();
    }
    new (this) WeakPtr(std::move(other));
    other.control_block = nullptr;
    other.ptr = nullptr;
    return *this;
  }
  void swap(WeakPtr other) {
    if(&other == this) {
      control_block->decrease_w_counter();
      control_block = nullptr;
    }
    std::swap(this->ptr, other.ptr);
    std::swap(this->control_block, other.control_block);
  }
  T& operator*() {
    return *(static_cast<T*>(control_block->get_ptr()));
  }
  const T& operator*() const {
    return *(static_cast<T*>(control_block->get_ptr()));
  }
  T* operator->() {
    return static_cast<T*>(control_block->get_ptr());
  }
  const T* operator->() const {
    return static_cast<T*>(control_block->get_ptr());
  }
  T* get() const {
    return static_cast<T*>(control_block->get_ptr());
  }
};

template <class T>
class SharedPtr {
 public:
  T* ptr;
  BaseControlBlock* control_block;

  size_t use_count() const {
    return control_block->shared_counter;
  }

  template <typename ...Args>
  void reset(Args&&... args) {
    SharedPtr(args...).swap(*this);
  }

  SharedPtr(): ptr(nullptr), control_block(nullptr) {
  }
  SharedPtr(BaseControlBlock* cb_ref) {
    control_block = cb_ref;
    ptr = static_cast<T*>(control_block->get_ptr());
    control_block->increase_s_counter();
    if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
      ptr->weak_this = *this;
    }
  }
  ~SharedPtr() {
    if (control_block != nullptr) {
      control_block->decrease_s_counter();
    }
  }
  template <class Y, class Deleter = std::default_delete<Y>, class Alloc = std::allocator<Y>>
  SharedPtr(Y* other_ptr, Deleter deleter = Deleter(), Alloc alloc = Alloc()) {
    ptr = other_ptr;
    using CBptrAllocator = std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockPtr<T, Deleter, Alloc>>;
    auto cbptr_alloc = CBptrAllocator{alloc};
    control_block = std::allocator_traits<CBptrAllocator>::allocate(cbptr_alloc, 1);
    new (control_block) ControlBlockPtr<Y, Deleter, Alloc>(other_ptr, deleter, alloc);
    if (control_block != nullptr) {
      control_block->increase_s_counter();
    }
    if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
      ptr->weak_this = *this;
    }
  }
  SharedPtr(const SharedPtr& other) {
    control_block = other.control_block;
    ptr = other.ptr;
    if (control_block != nullptr) {
      control_block->increase_s_counter();
    }
  }
  SharedPtr(SharedPtr<T>&& other) {
    ptr = other.ptr;
    control_block = other.control_block;
    other.ptr = nullptr;
    other.control_block = nullptr;
  }
  template <class Y>
  SharedPtr(const SharedPtr<Y>& other) {
    control_block = other.control_block;
    ptr = other.ptr;
    if (control_block != nullptr) {
      control_block->increase_s_counter();
    }
  }
  template <class Y>
  SharedPtr(SharedPtr<Y>&& other) {
    ptr = other.ptr;
    control_block = other.control_block;
    other.ptr = nullptr;
    other.control_block = nullptr;
  }
  SharedPtr& operator=(const SharedPtr& other) {
    if (control_block != nullptr) {
      control_block->decrease_s_counter();
    }
    new (this) SharedPtr(other);
    return *this;
  }
  SharedPtr& operator=(SharedPtr&& other) {
    if (control_block != nullptr) {
      control_block->decrease_s_counter();
    }
    new (this) SharedPtr(std::move(other));

    other.control_block = nullptr;
    other.ptr = nullptr;
    return *this;
  }

  template <class Y>
  SharedPtr& operator=(const SharedPtr<Y>& other) {
    if (control_block != nullptr) {
      control_block->decrease_s_counter();
    }
    new (this) SharedPtr(other);
    return *this;
  }
  template <class Y>
  SharedPtr& operator=(SharedPtr<Y>&& other) {
    if (control_block != nullptr) {
      control_block->decrease_s_counter();
    }
    new (this) SharedPtr(std::move(other));
    other.control_block = nullptr;
    other.ptr = nullptr;
    return *this;
  }
  void swap(SharedPtr& other) {
    if(&other == this) {
      control_block->decrease_s_counter();
      control_block = nullptr;
      return;
    }
    std::swap(this->ptr, other.ptr);
    std::swap(this->control_block, other.control_block);
  }
  T& operator*() {
    return *(static_cast<T*>(control_block->get_ptr()));
  }
  const T& operator*() const {
    return *(static_cast<T*>(control_block->get_ptr()));
  }
  T* operator->() {
    return static_cast<T*>(control_block->get_ptr());
  }
  const T* operator->() const {
    return static_cast<T*>(control_block->get_ptr());
  }
  T* get() const {
    if (control_block == nullptr) {
      return nullptr;
    }
    return static_cast<T*>(control_block->get_ptr());
  }
};
template <class T, typename ...Args>
SharedPtr<T> makeShared(Args&&... args) {
  return SharedPtr<T>(static_cast<BaseControlBlock*>(new ControlBlockObj<T>(std::forward<Args>(args)...)));
}
template< class T, class Alloc, class... Args >
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args ) {
  using CBObjAlloc = std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockObj<T, Alloc>>;
  auto correct_alloc = CBObjAlloc(alloc);
  auto ptr = correct_alloc.allocate(1);
  std::allocator_traits<CBObjAlloc>::construct(correct_alloc, ptr, std::forward<Args>(args)...);
  ptr->alloc = alloc;
  return SharedPtr<T>(static_cast<BaseControlBlock*>(ptr));
}


#endif //CPPTASKS__SMART_POINTERS_H_