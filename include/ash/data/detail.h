#ifndef ASH_DATA_DETAIL_H_
#define ASH_DATA_DETAIL_H_

namespace ash::data::detail {

// Yields T if T and U are the same ignoring const.
template<typename T, typename U>
struct if_noconst_same {
};

// Specialization for same T.
template<typename T>
struct if_noconst_same<T, T> {
  using type = T;
};

// Specialization for U = const T.
template<typename T>
struct if_noconst_same<T, const T> {
  using type = T;
};

// Specialization for T = const U.
template<typename T>
struct if_noconst_same<const T, T> {
  using type = const T;
};

// Yields T if T and U are the same ignoring const.
template<typename T, typename U>
using if_noconst_same_t = typename if_noconst_same<T, U>::type;

// Yields T if T is const.
template<typename T>
struct if_const {
};

// Specialization for const T.
template<typename T>
struct if_const<const T> {
  using type = const T;
};

// Yields T if T is const.
template<typename T>
using if_const_t = typename if_const<T>::type;

// Yields T if T is not const.
template<typename T>
struct if_mutable {
  using type = T;
};

// Empty specialization for const T.
template<typename T>
struct if_mutable<const T> {
};

// Yields T if T is not const.
template<typename T>
using if_mutable_t = typename if_mutable<T>::type;

// Yields mutable T when T is const, const T otherwise.
template<typename T>
struct opposite_const {
  using type = const T;
};

// Specialization to map const T -> T.
template<typename T>
struct opposite_const<const T> {
  using type = T;
};

// Yields mutable T when T is const, const T otherwise.
template<typename T>
using opposite_const_t = typename opposite_const<T>::type;

// Resolves T<const U> to const T<U>.
template<template<typename> typename T, typename U>
struct hoist_const {
  using type = T<U>;
};

} // namespace ash::data::detail

#endif // ASH_DATA_DETAIL_H_
