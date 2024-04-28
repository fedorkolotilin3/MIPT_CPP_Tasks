#include <iostream>
#include <memory>
#include <cassert>

//std::shared_ptr<int> alloc1_num;
//std::shared_ptr<int> alloc2_num;

template <size_t N>
class StackStorage {
  char storage[N];
  size_t position;
 public:
  char* start() {
    return &storage[0];
  }
  char* current_position() {
    return &storage[position];
  }
  void increase_current_position(size_t ind) {
    position += ind;
  }

  StackStorage(const StackStorage<N>&) = delete;
  StackStorage() {
    position = 0;
  }
};

template <typename T, size_t N>
class StackAllocator
{
 public:
  StackStorage<N>* storage;
  using value_type = T;

  template<typename U >
  struct rebind
  {
    typedef StackAllocator<U, N> other;
  };

  T* allocate(size_t n) {
    size_t buffer_size = N - (storage->current_position() - storage->start());
    char* current_position = storage->current_position();
    std::align(alignof(T), sizeof(T), reinterpret_cast<void*&>(current_position), buffer_size);
    storage->increase_current_position(current_position - storage->current_position());
    T* old_position = reinterpret_cast<T*>(storage->current_position());
    storage->increase_current_position(n * sizeof(T));
    return old_position;
  }
  void deallocate(T*, size_t) {}

  template <typename U>
  bool operator==(const StackAllocator<U, N>& other) const {
    //    std::cout << "==" << '\n';
    return storage == other.storage;
  }
  template <typename U>
  bool operator!=(const StackAllocator<U, N>& other) const {
    //    std::cout << "!=" << '\n';
    return storage != other.storage;
  }

  StackAllocator() = default;

  StackAllocator(StackStorage<N>& storage) : storage(&storage) {
    //    std::cout << "storage constr" << '\n';
  }

  template <typename U>
  StackAllocator(const StackAllocator<U, N>& other) :storage(other.storage) {
    //    std::cout << "copy constr" << '\n';
  }

  template <typename U>
  StackAllocator& operator=(StackAllocator<U, N>& other) {
    //    std::cout << "operator=" << '\n';
    this->storage = other.storage;
    return *this;
  }

};
template<typename T>
class BaseNode {
 public:
  BaseNode* next;
  BaseNode* previous;
};
template<typename T>
class Node : public BaseNode<T> {
 public:
  T object;
  Node() = default;
  Node(const T& item) try : object(item) {}
  catch (...) {
    throw;
  }
};

template <typename T, bool is_const>
class ListIterator {

  //  using base_node_pointer = std::conditional_t<is_const, const BaseNode<T>*, BaseNode<T>*>;
  //  using node_pointer = std::conditional_t<is_const, const Node<T>*, Node<T>*>;

  using base_node_pointer = BaseNode<T>*;
  using node_pointer = Node<T>*;

  base_node_pointer node;
 public:

  using value_type = T;
  using reference = std::conditional_t<is_const, const value_type&, value_type&>;
  using pointer = std::conditional_t<is_const, const value_type*, value_type*>;
  using difference_type = ssize_t;
  using iterator_category = std::bidirectional_iterator_tag;

  ListIterator(base_node_pointer node) : node(node){}
  operator ListIterator<T, true> () const {
    return ListIterator<T, true>(node);
  }

  ListIterator<T, is_const>& operator--() {
    node = node->previous;
    return *this;
  }
  ListIterator<T, is_const> operator--(int) {
    auto copy = *this;
    --(*this);
    return copy;
  }
  ListIterator<T, is_const>& operator++() {
    node = node->next;
    return *this;
  }
  ListIterator<T, is_const> operator++(int) {
    auto copy = *this;
    ++(*this);
    return copy;
  }
  reference operator*() {
    return static_cast<node_pointer>(node)->object;
  }
  pointer operator->() {
    return &static_cast<Node<T>*>(node)->object;
  }
  bool operator==(const ListIterator& other) {
    return other.node == this->node;
  }
  bool operator!=(const ListIterator& other) {
    return this->node != other.node;
  }
  node_pointer getNode() const{
    return static_cast<node_pointer>(node);
  }
};

template <typename T, class Alloc = std::allocator<T>>
class List {
 public:

  using const_iterator = ListIterator<T, true>;
  using iterator = ListIterator<T, false>;
  using const_reverse_iterator = std::reverse_iterator<ListIterator<T, true>>;
  using reverse_iterator = std::reverse_iterator<ListIterator<T, false>>;


  using AllocTraits = std::allocator_traits<Alloc>;
  using NodeAlloc = AllocTraits::template rebind_alloc<Node<T>>;
  using NodeAllocTraits = AllocTraits::template rebind_traits<Node<T>>;

  mutable BaseNode<T> fake_node{&fake_node, &fake_node};
  size_t _size;
  [[no_unique_address]] Alloc alloc;
  [[no_unique_address]] NodeAlloc node_alloc{alloc};
  NodeAllocTraits node_alloc_traits;

  Node<T>* allocate_node() {
    Node<T>* pointer = NodeAllocTraits::allocate(node_alloc, 1);
    pointer->next = nullptr;
    pointer->previous = nullptr;
    return pointer;
  }

  Node<T>* create_node() {
    Node<T>* pointer = allocate_node();
    try {
      NodeAllocTraits::construct(node_alloc, pointer);
    } catch (...) {
      throw;
    }
    return pointer;
  }
  Node<T>* create_node(const T& item) {
    Node<T>* pointer = allocate_node();
    try {
      NodeAllocTraits::construct(node_alloc, pointer, item);
    } catch (...) {
      throw;
    }
    return pointer;
  }

 public:
  void ConstructNodesWithoutValues(size_t n) {
    Node<T>* pointer = allocate_node();
    Node<T>* begin_pointer = pointer;
    pointer->previous = &fake_node;
    for (size_t i = 1; i < n; i++) {
      Node<T>* next_pointer = allocate_node();
      next_pointer->previous = pointer;
      pointer->next= next_pointer;
      pointer = next_pointer;
    }
    fake_node.previous = pointer;
    pointer->next = &fake_node;
    fake_node.next = begin_pointer;
  }

  void DestructNodes(iterator iter) {
    fake_node.next = &fake_node;
    fake_node.previous = &fake_node;
    iterator iter_copy(iter);
    iter_copy++;
    while (iter_copy != end()) {
      iterator prev_iter(iter_copy);
      prev_iter++;
      NodeAllocTraits::deallocate(node_alloc, static_cast<Node<T>*>(iter_copy.getNode()), 1);
      iter_copy = prev_iter;
    }
    while (iter != end()) {
      iterator prev_iter(iter);
      prev_iter--;
      AllocTraits::destroy(alloc, iter.operator->());
      NodeAllocTraits::deallocate(node_alloc, static_cast<Node<T>*>(iter.getNode()), 1);
      iter = prev_iter;
    }
  }

  List() : _size(0) {}
  List(size_t n) : List() {
    _size = n;
    ConstructNodesWithoutValues(n);
    iterator iter = begin();
    try {
      for (T &l_item : *this) {
        AllocTraits::construct(alloc, &l_item);
        iter++;
      }
    } catch (...) {
      DestructNodes(--iter);
    }
  }
  List(size_t n, const T& item) :List() {
    _size = n;
    ConstructNodesWithoutValues(n);
    iterator iter = begin();
    try {
      for (T &l_item : *this) {
        AllocTraits::construct(alloc, &l_item, item);
        iter++;
      }
    } catch (...) {
      DestructNodes(--iter);
    }
  }
  List(const Alloc& alloc) :  _size(0), alloc(alloc) {}
  List(size_t n, Alloc alloc) : List(alloc) {
    _size = n;
    ConstructNodesWithoutValues(n);
    iterator iter = begin();
    try {
      for (T &l_item : *this) {
        AllocTraits::construct(alloc, &l_item);
        iter++;
      }
    } catch (...) {
      DestructNodes(--iter);
    }
  }
  List(size_t n, const T& item, Alloc alloc) : List(alloc) {
    _size = n;
    ConstructNodesWithoutValues(n);
    iterator iter = begin();
    try {
      for (T &l_item : *this) {
        AllocTraits::construct(alloc, &l_item, item);
        iter++;
      }
    } catch (...) {
      DestructNodes(--iter);
    }
  }
  List(const List& other) : List(AllocTraits::select_on_container_copy_construction(other.alloc)) {
    ConstructNodesWithoutValues(other.size());
    iterator this_i = begin();
    _size = other.size();
    const_iterator other_i = other.begin();
    try {
      while (this_i != end()) {
        AllocTraits::construct(alloc, this_i.operator->(), *other_i);
        this_i++;
        other_i++;
      }
    } catch (...) {
      DestructNodes(--this_i);
    }
  }

  //  template <typename A>
  //  int getNum(A alloc) {
  //    return alloc.getNum();
  //  }
  //  template <typename A>
  //  std::shared_ptr<int>  getPtr(A alloc) {
  //    return alloc.getPtr();
  //  }


  List& operator=(const List& other) {
    if (&other == this) {
      return *this;
    }
    Alloc old_alloc = alloc;
    Alloc new_alloc = alloc;
    int start_size = _size;
    if (NodeAllocTraits::propagate_on_container_copy_assignment::value) {
      new_alloc = other.alloc;
    }
    alloc = new_alloc;
    const_iterator other_it = other.begin();
    const_iterator this_it = begin();
    try {
      while (other_it != other.end()) {
        insert(this_it, *other_it);
        other_it++;
        //        this_it++;
        //        this_it++;
      }
      alloc = old_alloc;
      while (_size != other.size()) {
        pop_back();
      }
      alloc = new_alloc;
    } catch (...) {
      alloc = new_alloc;
      while (_size != start_size) {
        pop_front();
      }
    }
    return *this;
  }
  ~List() {
    while (begin() != end()) {
      pop_back();
    }
    _size = 0;
  }

  size_t size() const{
    return _size;
  }
  void push_back(const T& item) {
    insert(end(), item);
  }
  void push_front(const T& item) {
    insert(begin(), item);
  }
  void pop_back() {
    erase(--end());
  }
  void pop_front() {
    erase(begin());
  }

  iterator begin() {
    return iterator(fake_node.next);
  }
  iterator end() {
    return iterator(&fake_node);
  }
  reverse_iterator rbegin() {
    return std::reverse_iterator<iterator>(end());
  }
  reverse_iterator rend() {
    return std::reverse_iterator<iterator>(begin());
  }

  const_iterator begin() const {
    return iterator(fake_node.next);
  }
  const_iterator end() const {
    return const_iterator(fake_node.next->previous);
  }
  const_reverse_iterator rbegin() const {
    return std::reverse_iterator<const_iterator>(end());
  }
  const_reverse_iterator rend() const {
    return std::reverse_iterator<const_iterator>(begin());
  }
  const_iterator cbegin() const {
    return begin();
  }
  const_iterator cend() const {
    return end();
  }
  const_reverse_iterator crbegin() const {
    return std::reverse_iterator<iterator>(end());
  }
  const_reverse_iterator crend() const {
    return std::reverse_iterator<iterator>(begin());
  }

  void insert(const_iterator iter, const T& item) {
    try {
      Node<T>* new_node = create_node(item);
      new_node->previous = iter.getNode()->previous;
      new_node->next = iter.getNode();
      new_node->previous->next = new_node;
      new_node->next->previous = new_node;
      _size++;
    } catch (...) {
      throw;
    }
  }
  void erase(const_iterator iter) {
    Node<T>* node_to_delete = static_cast<Node<T>*>(iter.getNode());
    node_to_delete->next->previous = node_to_delete->previous;
    node_to_delete->previous->next = node_to_delete->next;
    NodeAllocTraits::destroy(node_alloc, node_to_delete);
    node_alloc.deallocate(node_to_delete, 1);
    _size--;
  }
  Alloc get_allocator() {
    return alloc;
  }
};
