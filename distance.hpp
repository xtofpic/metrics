// -*- C++ -*-
//
//===---------------------------- distance ------------------------------------===//
//
// Copyright (c) 2018, 2019, Christophe Pijcke
//
// This file was largely inspired by the chrono library
//  from the LLVM Compiler Infrastructure.
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses.
//
//===----------------------------------------------------------------------===//

#ifndef METRICS_DISTANCE_HPP
#define METRICS_DISTANCE_HPP


namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class distance;

template <class _Tp>
    struct __is_distance: std::false_type {};

template <class _Rep, class _Period>
    struct __is_distance<distance<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_distance<const distance<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_distance<volatile distance<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_distance<const volatile distance<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::distance<_Rep1, _Period1>,
                    metric::distance<_Rep2, _Period2> >
{
	    typedef metric::distance<typename common_type<_Rep1, _Rep2>::type,
	                         typename __ratio_gcd<_Period1, _Period2>::type> type;
};

} // namespace std


namespace metric {


// distance_cast

template <class _FromDistance, class _ToDistance,
          class _Period = typename std::ratio_divide<typename _FromDistance::period, typename _ToDistance::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __distance_cast;

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, true, true>
{
    inline constexpr
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        return _ToDistance(static_cast<typename _ToDistance::rep>(__fd.count()));
    }
};

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, true, false>
{
    inline constexpr
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        typedef typename std::common_type<typename _ToDistance::rep, typename _FromDistance::rep, intmax_t>::type _Ct;
        return _ToDistance(static_cast<typename _ToDistance::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, false, true>
{
    inline constexpr
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        typedef typename std::common_type<typename _ToDistance::rep, typename _FromDistance::rep, intmax_t>::type _Ct;
        return _ToDistance(static_cast<typename _ToDistance::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromDistance, class _ToDistance, class _Period>
struct __distance_cast<_FromDistance, _ToDistance, _Period, false, false>
{
    inline constexpr
    _ToDistance operator()(const _FromDistance& __fd) const
    {
        typedef typename std::common_type<typename _ToDistance::rep, typename _FromDistance::rep, intmax_t>::type _Ct;
        return _ToDistance(static_cast<typename _ToDistance::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToDistance, class _Rep, class _Period>
inline
constexpr
typename std::enable_if
<
    __is_distance<_ToDistance>::value,
    _ToDistance
>::type
distance_cast(const distance<_Rep, _Period>& __fd)
{
    return __distance_cast<distance<_Rep, _Period>, _ToDistance>()(__fd);
}


template <class _Rep>
struct distance_values
{
public:
    inline static constexpr _Rep zero() {return _Rep(0);}
    inline static constexpr _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static constexpr _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class distance
{
    static_assert(!__is_distance<_Rep>::value, "A distance representation can not be a distance");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of distance must be a std::ratio");
    static_assert(_Period::num > 0, "distance period must be positive");

    template <class _R1, class _R2>
    struct __no_overflow
    {
    private:
        static const intmax_t __gcd_n1_n2 = std::__static_gcd<_R1::num, _R2::num>::value;
        static const intmax_t __gcd_d1_d2 = std::__static_gcd<_R1::den, _R2::den>::value;
        static const intmax_t __n1 = _R1::num / __gcd_n1_n2;
        static const intmax_t __d1 = _R1::den / __gcd_d1_d2;
        static const intmax_t __n2 = _R2::num / __gcd_n1_n2;
        static const intmax_t __d2 = _R2::den / __gcd_d1_d2;
        static const intmax_t max = -((intmax_t(1) << (sizeof(intmax_t) * CHAR_BIT - 1)) + 1);

        template <intmax_t _Xp, intmax_t _Yp, bool __overflow>
        struct __mul    // __overflow == false
        {
            static const intmax_t value = _Xp * _Yp;
        };

        template <intmax_t _Xp, intmax_t _Yp>
        struct __mul<_Xp, _Yp, true>
        {
            static const intmax_t value = 1;
        };

    public:
        static const bool value = (__n1 <= max / __d2) && (__n2 <= max / __d1);
        typedef std::ratio<__mul<__n1, __d2, !value>::value,
                      __mul<__n2, __d1, !value>::value> type;
    };

public:
    typedef _Rep rep;
    typedef _Period period;
private:
    rep __rep_;
public:

    inline constexpr
    distance() = default;

    template <class _Rep2>
        inline constexpr
        explicit distance(const _Rep2& __r,
            typename std::enable_if
            <
               std::is_convertible<_Rep2, rep>::value &&
               (std::is_floating_point<rep>::value ||
               !std::is_floating_point<_Rep2>::value)
            >::type* = 0)
                : __rep_(__r) {}

    // conversions
    template <class _Rep2, class _Period2>
        inline constexpr
        distance(const distance<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::distance_cast<distance>(__d).count()) {}

    // observer

    inline constexpr rep count() const {return __rep_;}

    // arithmetic

    inline constexpr distance  operator+() const {return *this;}
    inline constexpr distance  operator-() const {return distance(-__rep_);}
    inline const distance& operator++()      {++__rep_; return *this;}
    inline const distance  operator++(int)   {return distance(__rep_++);}
    inline const distance& operator--()      {--__rep_; return *this;}
    inline const distance  operator--(int)   {return distance(__rep_--);}

    inline const distance& operator+=(const distance& __d) {__rep_ += __d.count(); return *this;}
    inline const distance& operator-=(const distance& __d) {__rep_ -= __d.count(); return *this;}

    inline const distance& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const distance& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const distance& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const distance& operator%=(const distance& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static constexpr distance zero() {return distance(distance_values<rep>::zero());}
    inline static constexpr distance min()  {return distance(distance_values<rep>::min());}
    inline static constexpr distance max()  {return distance(distance_values<rep>::max());}
};


typedef distance<long long, std::nano > nanometre;
typedef distance<long long, std::micro> micrometre;
typedef distance<long long, std::milli> millimetre;
typedef distance<long long, std::centi> centimetre;
typedef distance<long long            > metre;
typedef distance<     long, std::kilo > kilometre;
typedef distance<     long, std::mega > megametre;



// Distance ==

template <class _LhsDistance, class _RhsDistance>
struct __distance_eq
{
    inline constexpr
    bool operator()(const _LhsDistance& __lhs, const _RhsDistance& __rhs) const
        {
            typedef typename std::common_type<_LhsDistance, _RhsDistance>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsDistance>
struct __distance_eq<_LhsDistance, _LhsDistance>
{
    inline constexpr
    bool operator()(const _LhsDistance& __lhs, const _LhsDistance& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator==(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return __distance_eq<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Distance !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator!=(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Distance <

template <class _LhsDistance, class _RhsDistance>
struct __distance_lt
{
    inline constexpr
    bool operator()(const _LhsDistance& __lhs, const _RhsDistance& __rhs) const
        {
            typedef typename std::common_type<_LhsDistance, _RhsDistance>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsDistance>
struct __distance_lt<_LhsDistance, _LhsDistance>
{
    inline constexpr
    bool operator()(const _LhsDistance& __lhs, const _LhsDistance& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator< (const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return __distance_lt<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Distance >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator> (const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Distance <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator<=(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Distance >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator>=(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Distance +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type
operator+(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Distance -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type
operator-(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Distance *

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    distance<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const distance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef distance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    distance<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const distance<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Distance /

template <class _Distance, class _Rep, bool = __is_distance<_Rep>::value>
struct __distance_divide_result
{
};

template <class _Distance, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Distance::rep, _Rep2>::type>::value>
struct __distance_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __distance_divide_imp<distance<_Rep1, _Period>, _Rep2, true>
{
    typedef distance<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __distance_divide_result<distance<_Rep1, _Period>, _Rep2, false>
    : __distance_divide_imp<distance<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __distance_divide_result<distance<_Rep1, _Period>, _Rep2>::type
operator/(const distance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef distance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<_Rep1, _Rep2>::type
operator/(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Distance %

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __distance_divide_result<distance<_Rep1, _Period>, _Rep2>::type
operator%(const distance<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef distance<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type
operator%(const distance<_Rep1, _Period1>& __lhs, const distance<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<distance<_Rep1, _Period1>, distance<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_DISTANCE_HPP


