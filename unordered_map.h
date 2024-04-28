#ifndef CPPTASKS__UNORDERED_MAP_H_
#define CPPTASKS__UNORDERED_MAP_H_

#include <iostream>
#include <memory>
#include <cassert>
#include <vector>
#include <cmath>

template <class T>
class Debuger{
 public:
  Debuger(const T& other) = delete;
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
  Node(const T& item) : object(item) {}
  Node(T&& item) : object(std::move(item)) {}
  template <typename ...Args>
  Node(Args&& ...args) : object(std::forward<Args&&>(args)...) {}
};

template <typename T, bool is_const>
class ListIterator {

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
  reference operator*() const {
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

template <class K, class V, class Alloc = std::allocator<std::pair<const K, V>>>
class List {
 public:

  using T = std::pair<const K, V>;
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
      AllocTraits ::construct(alloc, &pointer->object, item);
    } catch (...) {
      throw;
    }
    return pointer;
  }
  Node<T>* create_node(T&& item) {
    Node<T>* pointer = allocate_node();
    try {
      AllocTraits::construct(alloc, &pointer->object, std::move(item));
    } catch (...) {
      throw;
    }
    return pointer;
  }
  template <typename ...Args>
  Node<T>* create_node(Args&& ...args) {
    Node<T>* pointer = allocate_node();
    try {
      AllocTraits::construct(alloc, &pointer->object, std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    return pointer;
  }
  void delete_node(Node<T>* node_to_delete) {
    NodeAllocTraits::destroy(node_alloc, node_to_delete);
    node_alloc.deallocate(node_to_delete, 1);
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
  List(List&& other) : List(other.alloc){
    fake_node.next = other.fake_node.next;
    fake_node.previous = other.fake_node.previous;
    other.fake_node.previous->next = &fake_node;
    other.fake_node.next->previous = &fake_node;
    _size = other.size();
    other._size = 0;
    other.fake_node.next = &other.fake_node;
    other.fake_node.previous = &other.fake_node;
  }
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
  List& operator=(List&& other) {
    if (&other == this) {
      return *this;
    }
    Alloc old_alloc = alloc;
    Alloc new_alloc = alloc;
    int start_size = _size;
    if (NodeAllocTraits::propagate_on_container_move_assignment::value) {
      new_alloc = other.alloc;
    }
    alloc = new_alloc;
    const_iterator other_it = other.begin();
    const_iterator this_it = begin();
    try {
      while (other_it != other.end()) {
        emplace(this_it, std::move(*const_cast<K*>(&(*other_it).first)), std::move(*const_cast<V*>(&(*other_it).second)));
        other_it++;
      }
      alloc = old_alloc;
      while (_size != other.size()) {
        pop_back();
      }
      alloc = new_alloc;
      other.clear();
    } catch (...) {
      alloc = new_alloc;
      while (_size != start_size) {
        pop_front();
      }
    }
    return *this;
  }
  void clear() {
    while (begin() != end()) {
      pop_back();
    }
    _size = 0;
  }
  ~List() {
    clear();
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

  template <typename ...Args>
  void emplace(const_iterator iter, Args&&... args) {
    try {
      auto* node_ptr = &fake_node;
      if (iter != iterator(node_ptr)) {
        node_ptr = iter.getNode();
      }
      Node<T>* new_node = create_node(std::forward<Args>(args)...);
      new_node->previous = node_ptr->previous;
      new_node->next = node_ptr->next->previous;
      new_node->previous->next = new_node;
      new_node->next->previous = new_node;
      _size++;
    } catch (...) {
      throw;
    }
  }
  template <typename ...Args>
  void emplace_back(Args&&... args) {
    emplace(cend(), std::forward<Args&&>(args)...);
  }
  void move_iter(const_iterator it1, const_iterator it2) {
    it2.getNode()->previous->next = it2.getNode()->next;
    it2.getNode()->next->previous = it2.getNode()->previous;
    it1.getNode()->previous = it1.getNode()->previous;
    it1.getNode()->next = it1.getNode();
  }
  void insert(const_iterator iter, const T& item) {
    try {
      emplace(iter, item);
    } catch (...) {
      throw;
    }
  }
  void insert(const_iterator iter, T&& item) {
    try {
      emplace(iter, std::move(item));
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

template <class Key, class Value, typename Hash = std::hash<Key>, class Equal = std::equal_to<Key>, class Alloc = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
 public:
  using NodeType = std::pair<const Key, Value>;
  using size_type = size_t;
  using const_iterator = ListIterator<NodeType, true>;
  using iterator = ListIterator<NodeType, false>;
  using const_reverse_iterator = std::reverse_iterator<ListIterator<NodeType, true>>;
  using reverse_iterator = std::reverse_iterator<ListIterator<NodeType, false>>;


  using AllocTraits = std::allocator_traits<Alloc>;
  using IteratorAlloc = AllocTraits::template rebind_alloc<iterator>;
  using NodeTypeAlloc = AllocTraits::template rebind_alloc<NodeType>;
  using NodeTypeAllocTraits = std::allocator_traits<NodeTypeAlloc>;

  size_type bucket_count;
  [[no_unique_address]] Hash hasher;
  [[no_unique_address]] Equal equal;
  [[no_unique_address]] Alloc alloc;
  List<Key, Value, NodeTypeAlloc> node_list;
  std::vector<iterator, IteratorAlloc> bucket_vec;
  float _max_load_factor = 0.9f;

  UnorderedMap(size_type bucket_count = 11, const Hash& hasher = Hash(), const Equal equal = Equal(),
               const Alloc& alloc = Alloc()) : bucket_count(bucket_count), hasher(hasher), equal(equal), alloc(alloc), node_list(alloc),
                                               bucket_vec(bucket_count, node_list.end(), IteratorAlloc(alloc))
  {
  }

  UnorderedMap(const UnorderedMap& other) : UnorderedMap(other.bucket_count, other.hasher, other.equal,
                                                         AllocTraits::select_on_container_copy_construction(other.alloc)) {
    for (const NodeType& other_it : other) {
      emplace(other_it);
    }
  }
  UnorderedMap(UnorderedMap&& other) = default;
  UnorderedMap& operator=(const UnorderedMap& other) {
    if (&other == this) {
      return *this;
    }
    auto before_assignment_copy = this;
    clear();
    if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc = other.alloc;
    }
    try {
      for (auto it : other) {
        emplace(it);
      }
    }
    catch (...) {
      clear();
      *this = std::move(before_assignment_copy);
    }
    return *this;
  }
  UnorderedMap& operator=(UnorderedMap&& other) {
    if constexpr (AllocTraits::propagate_on_container_move_assignment::value) {
      alloc = std::move(other.alloc);
    }
    node_list = std::move(other.node_list);
    hasher = std::move(other.hasher);
    equal = std::move(other.equal);
    _max_load_factor = std::move(other._max_load_factor);
    rehash(other.bucket_count);
    return *this;
  }

  Value& operator[](Key&& key) {
    auto find_res = find(key);
    if (find_res == end()) {
      return emplace(std::make_pair(std::forward<Key>(key), Value())).first->second;
    }
    return find_res->second;
  }
  Value& operator[](const Key& key) {
    auto find_res = find(key);
    if (find_res == end()) {
      return emplace(std::make_pair(std::forward<const Key&>(key), Value())).first->second;
    }
    return find_res->second;
  }

  Value& at(const Key& key) {
    auto find_res = find(key);
    if (find_res == end()) {
      throw std::out_of_range("no element under this key");
    }
    return find_res->second;
  }
  size_type size() const {
    return node_list.size();
  }

  iterator find(const Key& key) {
    if (load_factor() > max_load_factor()) {
      rehash(bucket_count * 2.345);
    }
    size_type hash = hasher(key);
    iterator iter = std::next(bucket_vec[hash % bucket_count], 1);
    while (iter != node_list.end() && hasher(iter->first) == hash) {
      if (equal(iter->first, key)) {
        return iter;
      }
      iter++;
    }
    return node_list.end();
  }

  const_iterator find(const Key& key) const {
    size_type hash = hasher(key);
    iterator iter = std::next(bucket_vec[hash % bucket_count], 1);
    while (iter != node_list.end() && hasher((*iter).first) == hash) {
      if (equal(iter->first, key)) {
        return iter;
      }
      iter++;
    }
    return node_list.end();
  }

  std::pair<iterator , bool> insert(const NodeType& node_type) {
    auto find_res = find(node_type.first);
    if (find_res != end()) {
      return {find_res, false};
    }
    size_type bucket_ind = hasher(node_type.first) % bucket_count;
    auto prev_iter = bucket_vec[bucket_ind];
    if (prev_iter == end()) {
      bucket_vec[bucket_ind] = std::next(prev_iter,  -1);
      node_list.insert(prev_iter, node_type);
      return {std::next(prev_iter, -1), true};
    }
    node_list.insert(std::next(bucket_vec[bucket_ind], 1), node_type);
    return {std::next(bucket_vec[bucket_ind], 1), true};
  }

  std::pair<iterator , bool> insert(NodeType&& node_type) {
    auto find_res = find(node_type.first);
    if (find_res != end()) {
      return {find_res, false};
    }
    size_type bucket_ind = hasher(node_type.first) % bucket_count;
    auto prev_iter = bucket_vec[bucket_ind];
    if (prev_iter == end()) {
      bucket_vec[bucket_ind] = std::next(prev_iter,  -1);
      node_list.emplace(prev_iter, std::move(*const_cast<Key*>(&node_type.first)), std::move(*const_cast<Value*>(&node_type.second)));
      return {std::next(prev_iter, -1), true};
    }
    node_list.emplace(std::next(bucket_vec[bucket_ind], 1), std::move(*const_cast<Key*>(&node_type.first)), std::move(*const_cast<Value*>(&node_type.second)));
    return {std::next(bucket_vec[bucket_ind], 1), true};
  }

  template <class InputIter>
  void insert(InputIter _begin, InputIter _end) {
    InputIter run_iter = _begin;
    while (run_iter != _end) {
      insert(*run_iter);
      run_iter++;
    }
  }

  template <typename ...Args>
  std::pair<iterator, bool> emplace(Args&&... args) {
    try {
      auto* create_node = node_list.create_node(std::forward<Args>(args)...);
      const auto& key = create_node->object.first;
      auto& value = create_node->object.second;
      auto find_res = find(key);
      if (find_res != end()) {
        node_list.delete_node(create_node);
        return {find_res, false};
      }
      size_type bucket_ind = hasher(key) % bucket_count;
      auto prev_iter = bucket_vec[bucket_ind];
      if (prev_iter == end()) {
        bucket_vec[bucket_ind] = std::next(prev_iter,  -1);
        node_list.emplace(prev_iter, std::move(*(const_cast<Key*>(&(key)))), std::move(value));
        return {std::next(prev_iter, -1), true};
      }
      node_list.emplace(std::next(bucket_vec[bucket_ind], 1), std::move(*(const_cast<Key*>(&(key)))), std::move(value));
      return {std::next(bucket_vec[bucket_ind], 1), true};
    } catch (...) {
      throw;
    }
  }

  void erase(const const_iterator& erase_iter) {
    const Key& key = (*erase_iter).first;
    size_type hash_of_next = hasher((*std::next(erase_iter, 1)).first);
    try {
      node_list.erase(erase_iter);
    } catch (...) {
      throw;
    }
    if (hash_of_next == hasher(key)) {
      return;
    }
    bucket_vec[hash_of_next % bucket_count] = bucket_vec[hasher(key) % bucket_count];
    bucket_vec[hasher(key) % bucket_count] = node_list.end();
  }
  void erase(iterator start, iterator end) {
    iterator it = start;
    while (it != end) {
      iterator next = std::next(it, 1);
      erase(it);
      it = next;
    }
  }
  iterator begin() {
    return node_list.begin();
  }
  iterator end() {
    return node_list.end();
  }
  const_iterator begin() const {
    return node_list.begin();
  }
  const_iterator end() const {
    return node_list.end();
  }
  const_iterator cbegin() const {
    return node_list.cbegin();
  }
  const_iterator cend() const {
    return node_list.cend();
  }

  float max_load_factor() const {
    return _max_load_factor;
  }
  float load_factor() const {
    return size() / bucket_count;
  }
  void max_load_factor(float new_factor) {
    _max_load_factor = new_factor;
  }

  void rehash(size_type aim_buckets_count) {
    bucket_count = std::max(aim_buckets_count, size_type(size() / max_load_factor()));
    bucket_vec.clear();
    bucket_vec.resize(bucket_count, end());
    iterator iter = end();
    iterator before_end = std::next(end(), -1);
    while (iter != before_end) {
      size_type next_iter_hash = hasher((*std::next(iter, 1)).first);
      if (iter == end() || next_iter_hash != hasher(iter->first)) {
        bucket_vec[next_iter_hash % bucket_count] = iter;
      }
      iter++;
    }
  }

  void reserve(int reserve_value) {
    rehash(std::ceil(reserve_value / max_load_factor()));
  }
  void clear() {
    node_list.clear();
    bucket_vec.clear();
    bucket_count = 11;
    bucket_vec.resize(bucket_count, node_list.end());
  }
  std::pair<Key, Value>& non_const_node_type(const NodeType& node_type) {
    auto ans = const_cast<std::pair<Key, Value>*>(&node_type);
    return *ans;
  }
};
template <class T>
void swap(T& first, T& second) {
  T buffer = std::move(first);
  first = std::move(second);
  second = std::move(buffer);
}

#endif //CPPTASKS__UNORDERED_MAP_H_