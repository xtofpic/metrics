// -*- C++ -*-
//
//===---------------------------- voltage ------------------------------------===//
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

#ifndef METRICS_VOLTAGE_HPP
#define METRICS_VOLTAGE_HPP


namespace metric {


template <class _Rep, class _Period = std::ratio<1> > class voltage;

template <class _Tp> 
    struct __is_voltage: std::false_type {};

template <class _Rep, class _Period>
    struct __is_voltage<voltage<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_voltage<const voltage<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_voltage<volatile voltage<_Rep, _Period> > : std::true_type  {};

template <class _Rep, class _Period>
    struct __is_voltage<const volatile voltage<_Rep, _Period> > : std::true_type  {};


} // namespace metric



namespace std {

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
struct common_type< metric::voltage<_Rep1, _Period1>,
                    metric::voltage<_Rep2, _Period2> >
{   
	    typedef metric::voltage<typename common_type<_Rep1, _Rep2>::type,
	                         typename __ratio_gcd<_Period1, _Period2>::type> type;
};

} // namespace std


namespace metric {


// voltage_cast

template <class _FromVoltage, class _ToVoltage,
          class _Period = typename std::ratio_divide<typename _FromVoltage::period, typename _ToVoltage::period>::type,
          bool = _Period::num == 1,
          bool = _Period::den == 1>
struct __voltage_cast;

template <class _FromVoltage, class _ToVoltage, class _Period>
struct __voltage_cast<_FromVoltage, _ToVoltage, _Period, true, true>
{   
    inline constexpr 
    _ToVoltage operator()(const _FromVoltage& __fd) const
    {   
        return _ToVoltage(static_cast<typename _ToVoltage::rep>(__fd.count()));
    }
};

template <class _FromVoltage, class _ToVoltage, class _Period>
struct __voltage_cast<_FromVoltage, _ToVoltage, _Period, true, false>
{   
    inline constexpr 
    _ToVoltage operator()(const _FromVoltage& __fd) const
    {   
        typedef typename std::common_type<typename _ToVoltage::rep, typename _FromVoltage::rep, intmax_t>::type _Ct;
        return _ToVoltage(static_cast<typename _ToVoltage::rep>(
                           static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_Period::den)));
    }
};

template <class _FromVoltage, class _ToVoltage, class _Period>
struct __voltage_cast<_FromVoltage, _ToVoltage, _Period, false, true>
{   
    inline constexpr 
    _ToVoltage operator()(const _FromVoltage& __fd) const
    {   
        typedef typename std::common_type<typename _ToVoltage::rep, typename _FromVoltage::rep, intmax_t>::type _Ct;
        return _ToVoltage(static_cast<typename _ToVoltage::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)));
    }
};

template <class _FromVoltage, class _ToVoltage, class _Period>
struct __voltage_cast<_FromVoltage, _ToVoltage, _Period, false, false>
{
    inline constexpr
    _ToVoltage operator()(const _FromVoltage& __fd) const
    {
        typedef typename std::common_type<typename _ToVoltage::rep, typename _FromVoltage::rep, intmax_t>::type _Ct;
        return _ToVoltage(static_cast<typename _ToVoltage::rep>(
                           static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_Period::num)
                                                          / static_cast<_Ct>(_Period::den)));
    }
};


template <class _ToVoltage, class _Rep, class _Period>
inline
constexpr
typename std::enable_if
<
    __is_voltage<_ToVoltage>::value,
    _ToVoltage
>::type
voltage_cast(const voltage<_Rep, _Period>& __fd)
{
    return __voltage_cast<voltage<_Rep, _Period>, _ToVoltage>()(__fd);
}


template <class _Rep>
struct voltage_values
{
public:
    inline static constexpr _Rep zero() {return _Rep(0);}
    inline static constexpr _Rep max()  {return std::numeric_limits<_Rep>::max();}
    inline static constexpr _Rep min()  {return std::numeric_limits<_Rep>::lowest();}
};


template <class _Rep, class _Period>
class voltage
{
    static_assert(!__is_voltage<_Rep>::value, "A voltage representation can not be a voltage");
    static_assert(std::__is_ratio<_Period>::value, "Second template parameter of voltage must be a std::ratio");
    static_assert(_Period::num > 0, "voltage period must be positive");

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
    voltage() = default;

    template <class _Rep2>
        inline constexpr
        explicit voltage(const _Rep2& __r,
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
        voltage(const voltage<_Rep2, _Period2>& __d,
            typename std::enable_if
            <
                __no_overflow<_Period2, period>::value && (
                std::is_floating_point<rep>::value ||
                (__no_overflow<_Period2, period>::type::den == 1 &&
                 !std::is_floating_point<_Rep2>::value))
            >::type* = 0)
                : __rep_(metric::voltage_cast<voltage>(__d).count()) {}

    // observer

    inline constexpr rep count() const {return __rep_;}

    // arithmetic

    inline constexpr voltage  operator+() const {return *this;}
    inline constexpr voltage  operator-() const {return voltage(-__rep_);}
    inline const voltage& operator++()      {++__rep_; return *this;}
    inline const voltage  operator++(int)   {return voltage(__rep_++);}
    inline const voltage& operator--()      {--__rep_; return *this;}
    inline const voltage  operator--(int)   {return voltage(__rep_--);}

    inline const voltage& operator+=(const voltage& __d) {__rep_ += __d.count(); return *this;}
    inline const voltage& operator-=(const voltage& __d) {__rep_ -= __d.count(); return *this;}

    inline const voltage& operator*=(const rep& rhs) {__rep_ *= rhs; return *this;}
    inline const voltage& operator/=(const rep& rhs) {__rep_ /= rhs; return *this;}
    inline const voltage& operator%=(const rep& rhs) {__rep_ %= rhs; return *this;}
    inline const voltage& operator%=(const voltage& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static constexpr voltage zero() {return voltage(voltage_values<rep>::zero());}
    inline static constexpr voltage min()  {return voltage(voltage_values<rep>::min());}
    inline static constexpr voltage max()  {return voltage(voltage_values<rep>::max());}
};


typedef voltage<long long, std::nano > nanovolt;
typedef voltage<long long, std::micro> microvolt;
typedef voltage<long long, std::milli> millivolt;
typedef voltage<long long            > volt;
typedef voltage<long long, std::kilo > kilovolt;
typedef voltage<long long, std::mega > megavolt;



// Voltage ==

template <class _LhsVoltage, class _RhsVoltage>
struct __voltage_eq
{
    inline constexpr
    bool operator()(const _LhsVoltage& __lhs, const _RhsVoltage& __rhs) const
        {
            typedef typename std::common_type<_LhsVoltage, _RhsVoltage>::type _Ct;
            return _Ct(__lhs).count() == _Ct(__rhs).count();
        }
};

template <class _LhsVoltage>
struct __voltage_eq<_LhsVoltage, _LhsVoltage>
{
    inline constexpr
    bool operator()(const _LhsVoltage& __lhs, const _LhsVoltage& __rhs) const
        {return __lhs.count() == __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator==(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    return __voltage_eq<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Voltage !=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator!=(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    return !(__lhs == __rhs);
}

// Voltage <

template <class _LhsVoltage, class _RhsVoltage>
struct __voltage_lt
{
    inline constexpr
    bool operator()(const _LhsVoltage& __lhs, const _RhsVoltage& __rhs) const
        {
            typedef typename std::common_type<_LhsVoltage, _RhsVoltage>::type _Ct;
            return _Ct(__lhs).count() < _Ct(__rhs).count();
        }
};

template <class _LhsVoltage>
struct __voltage_lt<_LhsVoltage, _LhsVoltage>
{
    inline constexpr
    bool operator()(const _LhsVoltage& __lhs, const _LhsVoltage& __rhs) const
        {return __lhs.count() < __rhs.count();}
};

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator< (const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    return __voltage_lt<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >()(__lhs, __rhs);
}

// Voltage >

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator> (const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    return __rhs < __lhs;
}

// Voltage <=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator<=(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    return !(__rhs < __lhs);
}

// Voltage >=

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
bool
operator>=(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    return !(__lhs < __rhs);
}

// Voltage +

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type
operator+(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() + _Cd(__rhs).count());
}

// Voltage -

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type
operator-(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type _Cd;
    return _Cd(_Cd(__lhs).count() - _Cd(__rhs).count());
}

// Voltage *

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep2, typename std::common_type<_Rep1, _Rep2>::type>::value,
    voltage<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const voltage<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef voltage<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() * static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename std::enable_if
<
    std::is_convertible<_Rep1, typename std::common_type<_Rep1, _Rep2>::type>::value,
    voltage<typename std::common_type<_Rep1, _Rep2>::type, _Period>
>::type
operator*(const _Rep1& __s, const voltage<_Rep2, _Period>& __d)
{
    return __d * __s;
}

// Voltage /

template <class _Voltage, class _Rep, bool = __is_voltage<_Rep>::value>
struct __voltage_divide_result
{
};

template <class _Voltage, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Voltage::rep, _Rep2>::type>::value>
struct __voltage_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __voltage_divide_imp<voltage<_Rep1, _Period>, _Rep2, true>
{
    typedef voltage<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __voltage_divide_result<voltage<_Rep1, _Period>, _Rep2, false>
    : __voltage_divide_imp<voltage<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __voltage_divide_result<voltage<_Rep1, _Period>, _Rep2>::type
operator/(const voltage<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef voltage<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<_Rep1, _Rep2>::type
operator/(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type _Ct;
    return _Ct(__lhs).count() / _Ct(__rhs).count();
}

// Voltage %

template <class _Rep1, class _Period, class _Rep2>
inline
constexpr
typename __voltage_divide_result<voltage<_Rep1, _Period>, _Rep2>::type
operator%(const voltage<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef voltage<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}

template <class _Rep1, class _Period1, class _Rep2, class _Period2>
inline
constexpr
typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type
operator%(const voltage<_Rep1, _Period1>& __lhs, const voltage<_Rep2, _Period2>& __rhs)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef typename std::common_type<voltage<_Rep1, _Period1>, voltage<_Rep2, _Period2> >::type _Cd;
    return _Cd(static_cast<_Cr>(_Cd(__lhs).count()) % static_cast<_Cr>(_Cd(__rhs).count()));
}

} // namespace metric

#endif // METRICS_VOLTAGE_HPP


