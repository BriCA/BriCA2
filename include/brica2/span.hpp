#ifndef __BRICA2_SPAN_HPP__
#define __BRICA2_SPAN_HPP__

#include "brica2/macros.h"
#include "brica2/assert.hpp"

#include <array>
#include <iterator>
#include <type_traits>

NAMESPACE_BEGIN(BRICA2_NAMESPACE)

enum class byte : unsigned char {};

constexpr const std::ptrdiff_t dynamic_extent = -1;

struct narrowing_error : public std::exception {};

template <class ElementType, std::ptrdiff_t Extent = dynamic_extent> class span;

NAMESPACE_BEGIN(detail)

template <class T> struct is_span_oracle : std::false_type {};
template <class ElementType, std::ptrdiff_t Extent>
struct is_span_oracle<brica2::span<ElementType, Extent>> : std::true_type {};
template <class T>
struct is_span : public is_span_oracle<std::remove_cv_t<T>> {};
;

template <class T> struct is_std_array_oracle : std::false_type {};
template <class ElementType, std::size_t Extent>
struct is_std_array_oracle<std::array<ElementType, Extent>> : std::true_type {};
template <class T>
struct is_std_array : public is_std_array_oracle<std::remove_cv_t<T>> {};

template <std::ptrdiff_t From, std::ptrdiff_t To>
struct is_allowed_extent_conversion
    : public std::integral_constant<
          bool,
          From == To || From == brica2::dynamic_extent ||
              To == brica2::dynamic_extent> {};

template <class From, class To>
struct is_allowed_element_type_conversion
    : public std::integral_constant<
          bool,
          std::is_convertible<From (*)[], To (*)[]>::value> {};

template <class Span, bool IsConst> class span_iterator {
  using T = typename Span::element_type;

 public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::remove_cv_t<T>;
  using difference_type = typename Span::size_type;
  using reference = std::conditional_t<IsConst, const T, T>&;
  using pointer = std::add_pointer_t<reference>;

  span_iterator() = default;

  constexpr span_iterator(
      const Span* span, typename Span::index_type idx) noexcept
      : span_(span), index_(idx) {}

  friend span_iterator<Span, true>;
  template <bool B, std::enable_if_t<!B && IsConst>* = nullptr>
  constexpr span_iterator(const span_iterator<Span, B>& other) noexcept
      : span_iterator(other.span_, other.index_) {}

  constexpr reference operator*() const {
    Expects(index_ != span_->size());
    return *(span_->data() + index_);
  }

  constexpr pointer operator->() const {
    Expects(0 <= index_ && index_ != span_->size());
    return span_->data() + index_;
  }

  constexpr span_iterator& operator++() {
    ++index_;
    return *this;
  }

  constexpr span_iterator operator++(int) {
    auto ret = *this;
    ++(*this);
    return ret;
  }

  constexpr span_iterator& operator--() {
    Expects(0 != index_ && index_ <= span_->size());
    --index_;
    return *this;
  }

  constexpr span_iterator operator--(int) {
    auto ret = *this;
    --(*this);
    return ret;
  }

  constexpr span_iterator& operator+=(difference_type n) {
    Expects(0 <= (index + n) && (index + n) <= span_->size());
    index_ += n;
    return *this;
  }

  constexpr span_iterator operator+(difference_type n) const {
    auto ret = *this;
    return ret += n;
  }

  friend constexpr span_iterator operator+(
      difference_type n, span_iterator const& rhs) {
    return rhs + n;
  }

  constexpr span_iterator& operator-=(difference_type n) { return *this += -n; }

  constexpr span_iterator operator-(difference_type n) const {
    auto ret = *this;
    return ret -= n;
  }

  constexpr difference_type operator-(span_iterator rhs) const {
    return index_ - rhs.index_;
  }

  constexpr reference operator[](difference_type n) const {
    return *(*this + n);
  }

  constexpr friend bool operator==(
      span_iterator lhs, span_iterator rhs) noexcept {
    return lhs.span_ == rhs.span_ && lhs.index_ == rhs.index_;
  }

  constexpr friend bool operator!=(
      span_iterator lhs, span_iterator rhs) noexcept {
    return !(lhs == rhs);
  }

  constexpr friend bool operator<(
      span_iterator lhs, span_iterator rhs) noexcept {
    return lhs.index_ < rhs.index_;
  }

  constexpr friend bool operator<=(
      span_iterator lhs, span_iterator rhs) noexcept {
    return !(rhs < lhs);
  }

  constexpr friend bool operator>(
      span_iterator lhs, span_iterator rhs) noexcept {
    return rhs < lhs;
  }

  constexpr friend bool operator>=(
      span_iterator lhs, span_iterator rhs) noexcept {
    return !(rhs > lhs);
  }

 protected:
  const Span* span_ = nullptr;
  std::size_t index_ = 0;
};

template <std::ptrdiff_t Ext> class extent_type {
 public:
  static_assert(Ext >= 0, "A fixed-size span must be >= 0 in size.");

  using index_type = std::ptrdiff_t;

  constexpr extent_type() noexcept {};

  template <index_type Other> constexpr extent_type(extent_type<Other> ext) {
    static_assert(
        Other == Ext || Other == dynamic_extent,
        "Mistmatch between fixed-size extent and size of initializing data.");
    Expects(ext.size() == Ext);
  }

  constexpr extent_type(index_type size) { Expects(size == Ext); }
  constexpr index_type size() const noexcept { return Ext; }
};

template <> class extent_type<dynamic_extent> {
 public:
  using index_type = std::ptrdiff_t;

  template <index_type Other>
  explicit constexpr extent_type(extent_type<Other> ext) : size_(ext.size()) {}

  explicit constexpr extent_type(index_type size) : size_(size) {
    Expects(size >= 0);
  }

  constexpr index_type size() const noexcept { return size_; }

 private:
  index_type size_;
};

template <
    class ElementType,
    std::ptrdiff_t Extent,
    std::ptrdiff_t Offset,
    std::ptrdiff_t Count>
struct calculate_subspan_type {
  using type = span<
      ElementType,
      Count != dynamic_extent
          ? Count
          : (Extent != dynamic_extent ? Extent - Offset : Extent)>;
};

template <
    class ElementType,
    std::ptrdiff_t Extent,
    std::ptrdiff_t Offset,
    std::ptrdiff_t Count>
using calculate_subspan_type_t =
    typename calculate_subspan_type<ElementType, Extent, Offset, Count>::type;

template <class T, class U>
struct is_same_signedness
    : public std::integral_constant<
          bool,
          std::is_signed<T>::value == std::is_signed<U>::value> {};

NAMESPACE_END(detail)

template <class T, class U> constexpr T narrow_cast(U&& u) noexcept {
  return static_cast<T>(std::forward<U>(u));
}

template <class T, class U> constexpr T narrow(U u) noexcept(false) {
  T t = narrow_cast<T>(u);
  if (static_cast<U>(t) != u) detail::throw_exception(narrowing_error());
  if (!detail::is_same_signedness<T, U>::value && ((t < T{}) != (u < U{})))
    detail::throw_exception(narrowing_error());
  return t;
}

template <class ElementType, std::ptrdiff_t Extent> class span {
 public:
  using element_type = ElementType;
  using value_type = std::remove_cv_t<ElementType>;
  using index_type = std::ptrdiff_t;
  using pointer = element_type*;
  using reference = element_type&;

  using iterator = detail::span_iterator<span<ElementType, Extent>, false>;
  using const_iterator =
      detail::span_iterator<span<ElementType, Extent>, true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using size_type = index_type;

  static constexpr index_type extent{Extent};

  template <
      bool Dependent = false,
      class = std::enable_if_t<(Dependent || Extent <= 0)>>
  constexpr span() noexcept : storage_(nullptr, detail::extent_type<0>()) {}

  constexpr span(pointer ptr, index_type count) : storage_(ptr, count) {}
  constexpr span(pointer first, pointer last)
      : storage_(first, std::distance(first, last)) {}

  template <std::size_t N>
  constexpr span(element_type (&arr)[N]) noexcept
      : storage_(
            KnownNotNull{std::addressof(arr[0])}, detail::extent_type<N>()) {}

  template <std::size_t N, class = std::enable_if_t<(N > 0)>>
  constexpr span(std::array<value_type, N>& arr) noexcept
      : storage_(KnownNotNull{arr.data()}, detail::extent_type<N>()) {}

  constexpr span(std::array<value_type, 0>&) noexcept
      : storage_(static_cast<pointer>(nullptr), detail::extent_type<0>()) {}

  template <std::size_t N, class = std::enable_if_t<(N > 0)>>
  constexpr span(const std::array<value_type, N>& arr) noexcept
      : storage_(KnownNotNull{arr.data()}, detail::extent_type<N>()) {}

  constexpr span(const std::array<value_type, 0>&) noexcept
      : storage_(static_cast<pointer>(nullptr), detail::extent_type<0>()) {}

  template <
      class Container,
      class = std::enable_if_t<
          !detail::is_span<Container>::value &&
          !detail::is_std_array<Container>::value &&
          std::is_convertible<typename Container::pointer, pointer>::value &&
          std::is_convertible<
              typename Container::pointer,
              decltype(std::declval<Container>().data())>::value>>
  constexpr span(Container& cont)
      : span(cont.data(), narrow<index_type>(cont.size())) {}

  template <
      class Container,
      class = std::enable_if_t<
          !detail::is_span<Container>::value &&
          !detail::is_std_array<Container>::value &&
          std::is_convertible<typename Container::pointer, pointer>::value &&
          std::is_convertible<
              typename Container::pointer,
              decltype(std::declval<Container>().data())>::value>>
  constexpr span(const Container& cont)
      : span(cont.data(), narrow<index_type>(cont.size())) {}

  constexpr span(const span& other) noexcept = default;

  template <
      class OtherElementType,
      std::ptrdiff_t OtherExtent,
      class = std::enable_if_t<
          detail::is_allowed_extent_conversion<OtherExtent, Extent>::value &&
          detail::is_allowed_element_type_conversion<
              OtherElementType,
              element_type>::value>>
  constexpr span(const span<OtherElementType, OtherExtent>& other)
      : storage_(
            other.data(), detail::extent_type<OtherExtent>(other.size())) {}

  ~span() noexcept = default;

  constexpr span& operator=(const span& other) noexcept = default;

  template <std::ptrdiff_t Count>
  constexpr span<element_type, Count> first() const {
    Expects(0 <= Count && Count <= size());
    return {data(), Count};
  }

  template <std::ptrdiff_t Count>
  constexpr span<element_type, Count> last() const {
    Expects(0 <= Count && size() - Count >= 0);
    return {data() + (size() - Count), Count};
  }

  template <std::ptrdiff_t Offset, std::ptrdiff_t Count = dynamic_extent>
  constexpr auto subspan() const -> detail::
      calculate_subspan_type_t<ElementType, Extent, Offset, Count> {
    Expects(
        (0 <= Offset && 0 <= size() - Offset) &&
        (Count == dynamic_extent || (0 <= Count && Offset + Count <= size())));
    return {data() + Offset, Count == dynamic_extent ? size() - Offset : Count};
  }

  constexpr span<element_type, dynamic_extent> first(index_type count) const {
    Expects(0 <= count && count <= size());
    return {data(), count};
  }

  constexpr span<element_type, dynamic_extent> last(index_type count) const {
    return make_subspan(
        size() - count, dynamic_extent, subspan_selector<Extent>{});
  }

  constexpr span<element_type, dynamic_extent> subspan(
      index_type offset, index_type count = dynamic_extent) const {
    return make_subspan(offset, count, subspan_selector<Extent>{});
  }

  constexpr index_type size() const noexcept { return storage_.size(); }
  constexpr index_type size_bytes() const noexcept {
    return size() * narrow_cast<index_type>(sizeof(element_type));
  }

  constexpr bool empty() const noexcept { return size() == 0; }

  constexpr reference operator[](index_type idx) const {
    Expects(CheckRange(idx, storage_.size()));
    return data()[idx];
  }

  constexpr reference at(index_type idx) const { return (*this)[idx]; }
  constexpr reference operator()(index_type idx) const { return (*this)[idx]; };
  constexpr pointer data() const noexcept { return storage_.data(); }

  constexpr iterator begin() const noexcept { return {this, 0}; }
  constexpr iterator end() const noexcept { return {this, size()}; }

  constexpr const_iterator cbegin() const noexcept { return {this, 0}; }
  constexpr const_iterator cend() const noexcept { return {this, size()}; }

  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator{end()};
  }
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator{begin()};
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{cend()};
  }
  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{cbegin()};
  }

 private:
  static constexpr bool CheckRange(index_type idx, index_type size) noexcept {
    if (sizeof(index_type) < sizeof(std::size_t)) {
      return narrow_cast<std::size_t>(idx) < narrow_cast<std::size_t>(size);
    }
    return idx >= 0 && idx < size;
  }

  struct KnownNotNull {
    pointer p;
  };

  template <class ExtentType> class storage_type : public ExtentType {
   public:
    template <class OtherExtentType>
    constexpr storage_type(KnownNotNull data, OtherExtentType ext)
        : ExtentType(ext), data_(data.p) {
      Expects(ExtentType::size >= 0);
    }

    template <class OtherExtentType>
    constexpr storage_type(pointer data, OtherExtentType ext)
        : ExtentType(ext), data_(data) {
      Expects(ExtentType::size() >= 0);
      Expects(data || ExtentType::size() == 0);
    }

    constexpr pointer data() const noexcept { return data_; }

   private:
    pointer data_;
  };

  storage_type<detail::extent_type<Extent>> storage_;

  constexpr span(KnownNotNull ptr, index_type count) : storage_(ptr, count) {}
  template <std::ptrdiff_t CallerExtent> class subspan_selector {};

  template <std::ptrdiff_t CallerExtent>
  span<element_type, dynamic_extent> make_subspan(
      index_type offset,
      index_type count,
      subspan_selector<CallerExtent>) const {
    const span<element_type, dynamic_extent> tmp(*this);
    return tmp.subspan(offset, count);
  }

  span<element_type, dynamic_extent> make_subspan(
      index_type offset,
      index_type count,
      subspan_selector<dynamic_extent>) const {
    Expects(offset >= 0 && size() - offset >= 0);
    if (count == dynamic_extent) {
      return {KnownNotNull{data() + offset}, size() - offset};
    }
    Expects(count >= 0 && size() - offset >= count);
    return {KnownNotNull{data() + offset}, count};
  }
};

NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_SPAN_HPP__
