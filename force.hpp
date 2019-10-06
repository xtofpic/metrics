// -*- C++ -*-
//
//===---------------------------- force ------------------------------------===//
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

#ifndef METRICS_FORCE_HPP
#define METRICS_FORCE_HPP


namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class force;

template <class _Tp> 
    struct __is_force: std::false_type {};

template <class _Rep, class _Period>
    struct __is_force<force<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_force<const force<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_force<volatile force<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_force<const volatile force<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::force<_Rep1, _Period1>,
                    metric::force<_Rep2, _Period2> >
{   
	    typedef metric::force<typename common_type<_Rep1, _Rep2>::type,
	                         typename __ratio_gcd<_Period1, _Period2>::type> type;
};

} // namespace std


namespace metric {


// force_cast

template <class _FromForce, class _ToForce,
          class _Period = typename std::ratio_divide<typename _FromForce::period, typename _ToForce::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __force_cast;

template <class _FromForce, class _ToForce, class _Period>
struct __force_cast<_FromForce, _ToForce, _Period, true, true>
{   
    inline constexpr 
    _ToForce operator()(const _FromForce& __fd) const
    {   
        return _ToForce(static_cast<typename _ToForce::rep>(__fd.count()));
    }
};

template <class _FromForce, class _ToForce, class _Period>
struct __force_cast<_FromForce, _ToForce, _Period, true, false>
{   
    inline constexpr 
    _ToForce operator()(const _FromForce& __fd) const
    {   
        typedef typename std::common_type<typename _ToForce::rep, typename _FromForce::rep, intmax_t>::type _Ct;
        return _ToForce(static_cast<typename _ToForce::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromForce, class _ToForce, class _Period>
struct __force_cast<_FromForce, _ToForce, _Period, false, true>
{   
    inline constexpr 
    _ToForce operator()(const _FromForce& __fd) const
    {   
        typedef typename std::common_type<typename _ToForce::rep, typename _FromForce::rep, intmax_t>::type _Ct;
        return _ToForce(static_cast<typename _ToForce::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromForce, class _ToForce, class _Period>
struct __force_cast<_FromForce, _ToForce, _Period, false, false>
{
    inline constexpr
    _ToForce operator()(const _FromForce& __fd) const
    {
        typedef typename std::common_type<typename _ToForce::rep, typename _FromForce::rep, intmax_t>::type _Ct;
        return _ToForce(static_cast<typename _ToForce::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToForce, class _Rep, class _Period>
inline
constexpr
typename std::enable_if
<
    __is_force<_ToForce>::value,
    _ToForce
>::type
force_cast(const force<_Rep, _Period>& __fd)
{
    return __force_cast<force<_Rep, _Period>, _ToForce>()(__fd);
}


template <class _Rep>
struct force_values
{
public:
    inline static constexpr _Rep zero() {return _Rep(0);}
    inline static constexpr _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static constexpr _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class force
{
    static_assert(!__is_force<_Rep>::value, "A force representation can not be a force");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of force must be a std::ratio");
    static_assert(_Period::num > 0, "force period must be positive");

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
    force() = default;

    template <class _Rep2>
        inline constexpr
        explicit force(const _Rep2& __r,
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
        force(const force<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::force_cast<force>(__d).count()) {}

    // observer

    inline constexpr rep count() const {return __rep_;}

    // arithmetic

    inline constexpr force  operator+() const {return *this;}
    inline constexpr force  operator-() const {return force(-__rep_);}
    inline const force& operator++()      {++__rep_; return *this;}
    inline const force  operator++(int)   {return force(__rep_++);}
    inline const force& operator--()      {--__rep_; return *this;}
    inline const force  operator--(int)   {return force(__rep_--);}

    inline const force& operator+=(const force& __d) {__rep_ += __d.count(); return *this;}
    inline const force& operator-=(const force& __d) {__rep_ -= __d.count(); return *this;}

    inline const force& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const force& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const force& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const force& operator%=(const force& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static constexpr force zero() {return force(force_values<rep>::zero());}
    inline static constexpr force min()  {return force(force_values<rep>::min());}
    inline static constexpr force max()  {return force(force_values<rep>::max());}
};


typedef force<long long, std::milli                    > millinewton;
typedef force<long long                                > newton;
typedef force<long long, std::deca                     > decanewton;
typedef force<long long, std::ratio<980665, 100000000> > gramforce;
typedef force<long long, std::ratio<980665,    100000> > kilogramforce;



// Force ==

template <class _LhsForce, class _RhsForce>
struct __force_eq
{
    inline constexpr
    bool operator()(const _LhsForce& __lhs, const _RhsForce& __rhs) const
        {
            typedef typename std::common_type<_LhsForce, _RhsForce>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsForce>
struct __force_eq<_LhsForce, _LhsForce>
{
    inline constexpr
    bool operator()(const _LhsForce& __lhs, const _LhsForce& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator==(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    return __force_eq<force<_Rep1, _Period1>, force<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Force !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator!=(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Force <

template <class _LhsForce, class _RhsForce>
struct __force_lt
{
    inline constexpr
    bool operator()(const _LhsForce& __lhs, const _RhsForce& __rhs) const
        {
            typedef typename std::common_type<_LhsForce, _RhsForce>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsForce>
struct __force_lt<_LhsForce, _LhsForce>
{
    inline constexpr
    bool operator()(const _LhsForce& __lhs, const _LhsForce& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator< (const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    return __force_lt<force<_Rep1, _Period1>, force<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Force >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator> (const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Force <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator<=(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Force >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator>=(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Force +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type
operator+(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Force -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type
operator-(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Force *

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    force<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const force<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef force<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    force<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const force<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Force /

template <class _Force, class _Rep, bool = __is_force<_Rep>::value>
struct __force_divide_result
{
};

template <class _Force, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Force::rep, _Rep2>::type>::value>
struct __force_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __force_divide_imp<force<_Rep1, _Period>, _Rep2, true>
{
    typedef force<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __force_divide_result<force<_Rep1, _Period>, _Rep2, false>
    : __force_divide_imp<force<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __force_divide_result<force<_Rep1, _Period>, _Rep2>::type
operator/(const force<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef force<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<_Rep1, _Rep2>::type
operator/(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Force %

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __force_divide_result<force<_Rep1, _Period>, _Rep2>::type
operator%(const force<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef force<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type
operator%(const force<_Rep1, _Period1>& __lhs, const force<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<force<_Rep1, _Period1>, force<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_FORCE_HPP


