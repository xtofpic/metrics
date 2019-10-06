// -*- C++ -*-
//
//===---------------------------- speed ------------------------------------===//
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

#ifndef METRICS_SPEED_HPP
#define METRICS_SPEED_HPP


namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class speed;

template <class _Tp> 
    struct __is_speed: std::false_type {};

template <class _Rep, class _Period>
    struct __is_speed<speed<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_speed<const speed<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_speed<volatile speed<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_speed<const volatile speed<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::speed<_Rep1, _Period1>,
                    metric::speed<_Rep2, _Period2> >
{   
	    typedef metric::speed<typename common_type<_Rep1, _Rep2>::type,
	                         typename __ratio_gcd<_Period1, _Period2>::type> type;
};

} // namespace std


namespace metric {


// speed_cast

template <class _FromSpeed, class _ToSpeed,
          class _Period = typename std::ratio_divide<typename _FromSpeed::period, typename _ToSpeed::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __speed_cast;

template <class _FromSpeed, class _ToSpeed, class _Period>
struct __speed_cast<_FromSpeed, _ToSpeed, _Period, true, true>
{   
    inline constexpr 
    _ToSpeed operator()(const _FromSpeed& __fd) const
    {   
        return _ToSpeed(static_cast<typename _ToSpeed::rep>(__fd.count()));
    }
};

template <class _FromSpeed, class _ToSpeed, class _Period>
struct __speed_cast<_FromSpeed, _ToSpeed, _Period, true, false>
{   
    inline constexpr 
    _ToSpeed operator()(const _FromSpeed& __fd) const
    {   
        typedef typename std::common_type<typename _ToSpeed::rep, typename _FromSpeed::rep, intmax_t>::type _Ct;
        return _ToSpeed(static_cast<typename _ToSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _Period>
struct __speed_cast<_FromSpeed, _ToSpeed, _Period, false, true>
{   
    inline constexpr 
    _ToSpeed operator()(const _FromSpeed& __fd) const
    {   
        typedef typename std::common_type<typename _ToSpeed::rep, typename _FromSpeed::rep, intmax_t>::type _Ct;
        return _ToSpeed(static_cast<typename _ToSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _Period>
struct __speed_cast<_FromSpeed, _ToSpeed, _Period, false, false>
{
    inline constexpr
    _ToSpeed operator()(const _FromSpeed& __fd) const
    {
        typedef typename std::common_type<typename _ToSpeed::rep, typename _FromSpeed::rep, intmax_t>::type _Ct;
        return _ToSpeed(static_cast<typename _ToSpeed::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToSpeed, class _Rep, class _Period>
inline
constexpr
typename std::enable_if
<
    __is_speed<_ToSpeed>::value,
    _ToSpeed
>::type
speed_cast(const speed<_Rep, _Period>& __fd)
{
    return __speed_cast<speed<_Rep, _Period>, _ToSpeed>()(__fd);
}


template <class _Rep>
struct speed_values
{
public:
    inline static constexpr _Rep zero() {return _Rep(0);}
    inline static constexpr _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static constexpr _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class speed
{
    static_assert(!__is_speed<_Rep>::value, "A speed representation can not be a speed");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of speed must be a std::ratio");
    static_assert(_Period::num > 0, "speed period must be positive");

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
    speed() = default;

    template <class _Rep2>
        inline constexpr
        explicit speed(const _Rep2& __r,
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
        speed(const speed<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::speed_cast<speed>(__d).count()) {}

    // observer

    inline constexpr rep count() const {return __rep_;}

    // arithmetic

    inline constexpr speed  operator+() const {return *this;}
    inline constexpr speed  operator-() const {return speed(-__rep_);}
    inline const speed& operator++()      {++__rep_; return *this;}
    inline const speed  operator++(int)   {return speed(__rep_++);}
    inline const speed& operator--()      {--__rep_; return *this;}
    inline const speed  operator--(int)   {return speed(__rep_--);}

    inline const speed& operator+=(const speed& __d) {__rep_ += __d.count(); return *this;}
    inline const speed& operator-=(const speed& __d) {__rep_ -= __d.count(); return *this;}

    inline const speed& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const speed& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const speed& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const speed& operator%=(const speed& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static constexpr speed zero() {return speed(speed_values<rep>::zero());}
    inline static constexpr speed min()  {return speed(speed_values<rep>::min());}
    inline static constexpr speed max()  {return speed(speed_values<rep>::max());}
};



typedef speed<long long, std::ratio<86400, 1000000> > micrometre_second;
typedef speed<long long, std::ratio< 1440, 1000000> > micrometre_minute;
typedef speed<long long, std::ratio<   24, 1000000> > micrometre_hour;
typedef speed<long long, std::ratio<86400,    1000> > millimetre_second;
typedef speed<long long, std::ratio< 1440,    1000> > millimetre_minute;
typedef speed<long long, std::ratio<   24,    1000> > millimetre_hour;
typedef speed<long long, std::ratio<    1,    1000> > millimetre_day;
typedef speed<long long, std::ratio<86400,       1> > metre_second;
typedef speed<long long, std::ratio< 1440,       1> > metre_minute;
typedef speed<long long, std::ratio<   24,       1> > metre_hour;
typedef speed<long long, std::ratio<    1,       1> > metre_day;



// Speed ==

template <class _LhsSpeed, class _RhsSpeed>
struct __speed_eq
{
    inline constexpr
    bool operator()(const _LhsSpeed& __lhs, const _RhsSpeed& __rhs) const
        {
            typedef typename std::common_type<_LhsSpeed, _RhsSpeed>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsSpeed>
struct __speed_eq<_LhsSpeed, _LhsSpeed>
{
    inline constexpr
    bool operator()(const _LhsSpeed& __lhs, const _LhsSpeed& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator==(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    return __speed_eq<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Speed !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator!=(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Speed <

template <class _LhsSpeed, class _RhsSpeed>
struct __speed_lt
{
    inline constexpr
    bool operator()(const _LhsSpeed& __lhs, const _RhsSpeed& __rhs) const
        {
            typedef typename std::common_type<_LhsSpeed, _RhsSpeed>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsSpeed>
struct __speed_lt<_LhsSpeed, _LhsSpeed>
{
    inline constexpr
    bool operator()(const _LhsSpeed& __lhs, const _LhsSpeed& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator< (const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    return __speed_lt<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Speed >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator> (const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Speed <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator<=(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Speed >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator>=(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Speed +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type
operator+(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Speed -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type
operator-(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Speed *

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    speed<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const speed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef speed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    speed<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const speed<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Speed /

template <class _Speed, class _Rep, bool = __is_speed<_Rep>::value>
struct __speed_divide_result
{
};

template <class _Speed, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Speed::rep, _Rep2>::type>::value>
struct __speed_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __speed_divide_imp<speed<_Rep1, _Period>, _Rep2, true>
{
    typedef speed<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __speed_divide_result<speed<_Rep1, _Period>, _Rep2, false>
    : __speed_divide_imp<speed<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __speed_divide_result<speed<_Rep1, _Period>, _Rep2>::type
operator/(const speed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef speed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<_Rep1, _Rep2>::type
operator/(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Speed %

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __speed_divide_result<speed<_Rep1, _Period>, _Rep2>::type
operator%(const speed<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef speed<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type
operator%(const speed<_Rep1, _Period1>& __lhs, const speed<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<speed<_Rep1, _Period1>, speed<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_SPEED_HPP


