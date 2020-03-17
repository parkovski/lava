#ifndef ASH_AST_POLYVECTOR_H_
#define ASH_AST_POLYVECTOR_H_

#include <type_traits>
#include <utility>

namespace ash::data {

namespace detail {

// Wrapper to make an iterator over smart pointers into an iterator over
// references.
template<typename TIter>
class SmartPtrIteratorWrapper {
public:
  using value_type = typename TIter::value_type::element_type;
  using difference_type = typename TIter::difference_type;
  using pointer = std::conditional_t<
    std::is_const_v<std::remove_pointer_t<typename TIter::pointer>>,
    const value_type *,
    value_type *
  >;
  using reference = std::conditional_t<
    std::is_const_v<std::remove_reference_t<typename TIter::reference>>,
    const value_type &,
    value_type &
  >;
  using iterator_category = typename TIter::iterator_category;

  SmartPtrIteratorWrapper() = default;

  explicit SmartPtrIteratorWrapper(TIter iter)
    noexcept(std::is_nothrow_move_constructible_v<TIter>)
    : _iter{std::move(iter)}
  {}

  SmartPtrIteratorWrapper(const SmartPtrIteratorWrapper &) = default;
  SmartPtrIteratorWrapper &
  operator=(const SmartPtrIteratorWrapper &) = default;

  reference operator*() const { return *_iter->get(); }
  pointer operator->() const { return _iter->get(); }
  reference operator[](difference_type d) const {
    return *_iter[d].get();
  }

  SmartPtrIteratorWrapper &operator++() {
    ++_iter;
    return *this;
  }
  SmartPtrIteratorWrapper operator++(int) {
    SmartPtrIteratorWrapper current(*this);
    ++*this;
    return current;
  }
  SmartPtrIteratorWrapper &operator+=(difference_type d) {
    _iter += d;
    return *this;
  }

  SmartPtrIteratorWrapper &operator--() {
    --_iter;
    return *this;
  }
  SmartPtrIteratorWrapper operator--(int) {
    SmartPtrIteratorWrapper current(*this);
    --*this;
    return current;
  }
  SmartPtrIteratorWrapper &operator-=(difference_type d) {
    _iter -= d;
    return *this;
  }

  friend SmartPtrIteratorWrapper
  operator+(const SmartPtrIteratorWrapper &self, difference_type d) {
    SmartPtrIteratorWrapper next{self};
    next += d;
    return next;
  }
  friend SmartPtrIteratorWrapper
  operator+(difference_type d, const SmartPtrIteratorWrapper &self) {
    return self + d;
  }
  friend SmartPtrIteratorWrapper
  operator-(const SmartPtrIteratorWrapper &self, difference_type d) {
    SmartPtrIteratorWrapper next{self};
    next -= d;
    return next;
  }
  friend difference_type
  operator-(const SmartPtrIteratorWrapper &self,
            const SmartPtrIteratorWrapper &other) {
    return self._iter - other._iter;
  }

  bool operator==(const SmartPtrIteratorWrapper &other) const {
    return _iter == other._iter;
  }
  bool operator!=(const SmartPtrIteratorWrapper &other) const {
    return _iter != other._iter;
  }
  bool operator<(const SmartPtrIteratorWrapper &other) const {
    return _iter < other._iter;
  }
  bool operator<=(const SmartPtrIteratorWrapper &other) const {
    return _iter <= other._iter;
  }
  bool operator>(const SmartPtrIteratorWrapper &other) const {
    return _iter > other._iter;
  }
  bool operator>=(const SmartPtrIteratorWrapper &other) const {
    return _iter >= other._iter;
  }

  explicit operator TIter &() {
    return _iter;
  }

private:
  TIter _iter;
};

} // namespace detail

template<
  typename T,
  template<typename> typename Vector,
  template<typename> typename Pointer
>
class PolyVector {
public:
  using vector_type             = Vector<Pointer<T>>;

  using value_type              = T;
  using allocator_type          = typename vector_type::allocator_type;
  using size_type               = typename vector_type::size_type;
  using difference_type         = typename vector_type::difference_type;
  using reference               = T &;
  using const_reference         = const T &;
  using pointer                 = Pointer<T>;
  using const_pointer           = Pointer<const T>;
  using iterator                =
    detail::SmartPtrIteratorWrapper<typename vector_type::iterator>;
  using const_iterator          =
    detail::SmartPtrIteratorWrapper<typename vector_type::const_iterator>;
  using reverse_iterator        =
    detail::SmartPtrIteratorWrapper<
      typename vector_type::reverse_iterator
    >;
  using const_reverse_iterator  =
    detail::SmartPtrIteratorWrapper<
      typename vector_type::const_reverse_iterator
    >;

  PolyVector() = default;
  ~PolyVector() = default;

  PolyVector(const PolyVector &) = default;
  PolyVector &operator=(const PolyVector &) = default;

  PolyVector(PolyVector &&) = default;
  PolyVector &operator=(PolyVector &&) = default;

  template<typename U,
           typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  U &emplace_back(Pointer<U> element) {
    return *static_cast<U *>(
      _vector.emplace_back(std::move(element)).get()
    );
  }

  template<typename U,
           typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  U &emplace_back(U *element) {
    return *static_cast<U *>(
      _vector.emplace_back(Pointer<T>(element)).get()
    );
  }

  template<
    typename U, typename... Args,
    typename = std::enable_if_t<std::conjunction_v<
      std::is_base_of_v<T, U>,
      std::is_constructible_v<U, Args...>
    >>
  >
  U &emplace_back(Args &&...args) {
    return *static_cast<U *>(_vector.emplace_back(
      Pointer<U>(new U(std::forward<Args>(args)...))
    ).get());
  }

  void push_back(pointer element) {
    _vector.push_back(std::move(element));
  }

  void push_back(T *element) {
    _vector.push_back(Pointer<T>(element));
  }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  void push_back(const U &element) {
    _vector.push_back(Pointer<U>(new U(element)));
  }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  void push_back(U &&element) {
    _vector.push_back(Pointer<U>(new U(std::move(element))));
  }

  iterator insert(const_iterator pos, pointer value) {
    return iterator(_vector.insert(
      typename vector_type::const_iterator(pos), value
    ));
  }

  iterator insert(const_iterator pos, T *value) {
    return iterator(_vector.insert(
      typename vector_type::const_iterator(pos), Pointer<T>(value)
    ));
  }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  iterator insert(const_iterator pos, const U &value) {
    return iterator(_vector.insert(
      typename vector_type::const_iterator(pos), Pointer<U>(new U(value))
    ));
  }

  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  iterator insert(const_iterator pos, U &&value) {
    return iterator(_vector.insert(
      typename vector_type::const_iterator(pos),
      Pointer<U>(new U(std::move(value)))
    ));
  }

#if 0
  template<typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
  }

  iterator insert(const_iterator pos, std::initializer_list<T> list) {
  }
#endif

  template<typename ...Args>
  iterator emplace(const_iterator pos, Args &&...args) {
    return iterator(_vector.emplace(
      typename vector_type::const_iterator(pos),
      std::forward<Args>(args)...
    ));
  }

  iterator erase(const_iterator pos) {
    return iterator(_vector.erase(
      typename vector_type::const_iterator(pos)
    ));
  }

  iterator erase(const_iterator first, const_iterator last) {
    return iterator(_vector.erase(
      typename vector_type::const_iterator(first),
      typename vector_type::const_iterator(last)
    ));
  }

  void pop_back() {
    _vector.pop_back();
  }

  void resize(size_type count) {
    _vector.resize(count);
  }

#if 0
  template<typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
  void resize(size_type count, const U &value) {
  }
#endif

  void clear() noexcept { _vector.clear(); }

  void swap(PolyVector &other)
    noexcept(noexcept(_vector.swap(other._vector)))
  {
    _vector.swap(other._vector);
  }

  size_type size() const noexcept { return _vector.size(); }
  size_type max_size() const noexcept { return _vector.max_size(); }
  bool empty() const noexcept { return _vector.empty(); }
  void reserve(size_type new_cap) { _vector.reserve(new_cap); }
  size_type capacity() const noexcept { return _vector.capacity(); }
  void shrink_to_fit() { _vector.shrink_to_fit(); }

  reference at(size_type index) { return *_vector[index].at(index); }
  const_reference at(size_type index) const {
    return *_vector[index].at(index);
  }
  reference operator[](size_type index) { return *_vector[index].get(); }
  const_reference operator[](size_type index) const {
    return *_vector[index].get();
  }
  reference front() { return *_vector.front().get(); }
  const_reference front() const { return *_vector.front().get(); }
  reference back() { return *_vector.back().get(); }
  const_reference back() const { return *_vector.back().get(); }

  iterator begin() { return iterator(_vector.begin()); }
  const_iterator begin() const { return const_iterator(_vector.begin()); }
  const_iterator cbegin() const { return const_iterator(_vector.cbegin()); }

  iterator end() { return iterator(_vector.end()); }
  const_iterator end() const { return const_iterator(_vector.end()); }
  const_iterator cend() const { return const_iterator(_vector.cend()); }

  reverse_iterator rbegin() { return reverse_iterator(_vector.rbegin()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(_vector.rbegin());
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(_vector.crbegin());
  }

  reverse_iterator rend() { return reverse_iterator(_vector.rend()); }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(_vector.rend());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(_vector.crend());
  }

  bool operator==(const PolyVector &other) const {
    return _vector == other._vector;
  }
  bool operator!=(const PolyVector &other) const {
    return _vector != other._vector;
  }
  bool operator<(const PolyVector &other) const {
    return _vector < other._vector;
  }
  bool operator<=(const PolyVector &other) const {
    return _vector <= other._vector;
  }
  bool operator>(const PolyVector &other) const {
    return _vector > other._vector;
  }
  bool operator>=(const PolyVector &other) const {
    return _vector >= other._vector;
  }

private:
  vector_type _vector;
};

} // namespace ash::data

#endif // ASH_AST_POLYVECTOR_H_
