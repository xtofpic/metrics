// -*- C++ -*-
//
//===---------------------------- flowrate ------------------------------------===//
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

#ifndef METRICS_FLOWRATE_HPP
#define METRICS_FLOWRATE_HPP

#include "metric_config.hpp"
#include "volume.hpp"
#include <chrono>

namespace metric {

template <typename _volume, typename _time> class flowrate;

template <typename A> struct __is_flowrate: __is_specialization<A, flowrate> {};

}

namespace std
{
    // CLASS TEMPLATE common_type SPECIALIZATIONS
    template<
		typename _VolumeRep1,
		typename _VolumePeriod1,
		typename _TimeRep1,
		typename _TimePeriod1,
		typename _VolumeRep2,
		typename _VolumePeriod2,
		typename _TimeRep2,
		typename _TimePeriod2
    >
        struct common_type<
		metric::flowrate<metric::volume<_VolumeRep1, _VolumePeriod1>, std::chrono::duration<_TimeRep1, _TimePeriod1> >,
		metric::flowrate<metric::volume<_VolumeRep2, _VolumePeriod2>, std::chrono::duration<_TimeRep2, _TimePeriod2> >
        >
    {    // common type of two durations

        // Define volume type to use
        typedef typename common_type<metric::volume<_VolumeRep1, _VolumePeriod1>, metric::volume<_VolumeRep2, _VolumePeriod2>>::type volume_type;
        typedef typename common_type<std::chrono::duration<_TimeRep1, _TimePeriod1>, std::chrono::duration<_TimeRep2, _TimePeriod2>>::type duration_type;

        typedef metric::flowrate<
            volume_type,
            duration_type
        > type;
    };
}

namespace metric {

template <class _FromFlowrate, class _ToFlowrate,
    class _PeriodVolume = typename std::ratio_divide<typename _FromFlowrate::volume_period, typename _ToFlowrate::volume_period>::type,
    class _PeriodDuration = typename std::ratio_divide<typename _FromFlowrate::duration_period, typename _ToFlowrate::duration_period>::type,
    bool = _PeriodVolume::num == 1,
    bool = _PeriodVolume::den == 1,
    bool = _PeriodDuration::num == 1,
    bool = _PeriodDuration::den == 1>
    struct __flowrate_cast;

template <class _FromFlowRate, class _ToFlowRate, class _PeriodVolume, class _PeriodDuration>
struct __flowrate_cast<_FromFlowRate, _ToFlowRate, _PeriodVolume, _PeriodDuration, true, true, true, true>
{
    inline  _ToFlowRate operator()(const _FromFlowRate& __fd) const
    {
        return _ToFlowRate(static_cast<typename _ToFlowRate::rep>(__fd.count()));
    }
};

template <class _FromFlowRate, class _ToFlowRate, class _PeriodVolume, class _PeriodDuration>
struct __flowrate_cast<_FromFlowRate, _ToFlowRate, _PeriodVolume, _PeriodDuration, true, true, false, true>
{
    inline
        _ToFlowRate operator()(const _FromFlowRate& __fd) const
    {
        typedef typename std::common_type<typename _ToFlowRate::volume_rep, typename _FromFlowRate::volume_rep, intmax_t>::type _Ct;

        return _ToFlowRate(static_cast<typename _ToFlowRate::volume_rep>(
            static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_PeriodDuration::num)));
    }
};

template <class _FromFlowRate, class _ToFlowRate, class _PeriodVolume, class _PeriodDuration>
struct __flowrate_cast<_FromFlowRate, _ToFlowRate, _PeriodVolume, _PeriodDuration, true, true, true, false>
{
    inline
        _ToFlowRate operator()(const _FromFlowRate& __fd) const
    {
        typedef typename std::common_type<typename _ToFlowRate::volume_rep, typename _FromFlowRate::volume_rep, intmax_t>::type _Ct;

        return _ToFlowRate(static_cast<typename _ToFlowRate::volume_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodDuration::den)));
    }
};

template <class _FromFlowRate, class _ToFlowRate, class _PeriodVolume, class _PeriodDuration>
struct __flowrate_cast<_FromFlowRate, _ToFlowRate, _PeriodVolume, _PeriodDuration, false, true, false, true>
{
    inline
        _ToFlowRate operator()(const _FromFlowRate& __fd) const
    {
        typedef typename std::common_type<typename _ToFlowRate::volume_rep, typename _FromFlowRate::volume_rep, intmax_t>::type _Ct;

        std::cout << "fd: " << static_cast<_Ct>(__fd.count()) << std::endl;
        std::cout << "After converting to volume: " << static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodVolume::num) << std::endl;
        std::cout << "After converting to time: " << static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_PeriodDuration::num) << std::endl;

        return _ToFlowRate(static_cast<typename _ToFlowRate::volume_rep>(
        		(static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodVolume::num)) / static_cast<_Ct>(_PeriodDuration::num)));
    }
};



/*  No type defined with duration having num/den different.  (TODO if necessary later).
template <class _FromFlowRate, class _ToFlowRate, class _PeriodVolume, class _PeriodDuration>
struct __flowrate_cast<_FromFlowRate, _ToFlowRate, _Period, true, true, false, false>
{   }; */


template <class _ToFlowRate, class _Vol, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_flowrate<_ToFlowRate>::value,
    _ToFlowRate
>::type
flowrate_cast(const flowrate<_Vol, _Period>& __fd)
{
    return __flowrate_cast<flowrate<_Vol, _Period>, _ToFlowRate>()(__fd);
}



template <typename _Volume, typename _Time>
class flowrate
{
    static_assert(!__is_flowrate<typename _Volume::rep>::value, "A flowrate representation can not be a flowrate");
    static_assert(__is_volume<_Volume>::value, "First template paramater of flowrate must be a volume");
    static_assert(std::chrono::__is_duration<_Time>::value, "Second template paramater of flowrate must be a duration");
    static_assert(std::__is_ratio<typename _Volume::period>::value, "Second template parameter of flowrate volume must be a std::ratio");
    static_assert(std::__is_ratio<typename _Time::period>::value, "Second template parameter of flowrate duration must be a std::ratio");
    static_assert(_Volume::period::num > 0, "flowrate volume period must be positive");
    static_assert(_Time::period::num > 0, "flowrate duration period must be positive");

    template <class _R1, class _R2>
    struct __no_overflow
    {
    private:
        static const intmax_t __gcd_n1_n2 = GCD<_R1::num, _R2::num>::value;
        static const intmax_t __gcd_d1_d2 = GCD<_R1::den, _R2::den>::value;
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
    typedef typename _Volume::rep		volume_rep;
    typedef typename _Volume::period	volume_period;
    typedef typename _Time::rep			duration_rep;
    typedef typename _Time::period		duration_period;

private:
    volume_rep __rep_;

public:

    inline METRICCONSTEXPR
    flowrate() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit flowrate(const _Rep2& __r,
            typename std::enable_if
            <
               std::is_convertible<_Rep2, volume_rep>::value &&
               (std::is_floating_point<volume_rep>::value ||
               !std::is_floating_point<_Rep2>::value)
            >::type* = 0)
                : __rep_(__r) {}

    // conversions
    template <class _Volume2, class _Time2>
        inline METRICCONSTEXPR
        flowrate(const flowrate<_Volume2, _Time2>& __d /*,  TODO: Re-activate this
            typename std::enable_if
            <
                __no_overflow<typename _Volume2::volume_period, volume_period>::value && (
                std::is_floating_point<volume_rep>::value ||
                (__no_overflow<typename _Volume2::volume_period, volume_period>::type::den == 1 &&
                 !std::is_floating_point<typename _Volume2::volume_rep>::value))
            >::type* = 0*/)
                : __rep_(metric::flowrate_cast<flowrate>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR volume_rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR flowrate  operator+() const {return *this;}
    inline METRICCONSTEXPR flowrate  operator-() const {return flowrate(-__rep_);}
    inline const flowrate& operator++()      {++__rep_; return *this;}
    inline const flowrate  operator++(int)   {return flowrate(__rep_++);}
    inline const flowrate& operator--()      {--__rep_; return *this;}
    inline const flowrate  operator--(int)   {return flowrate(__rep_--);}

    inline const flowrate& operator+=(const flowrate& __d) {__rep_ += __d.count(); return *this;}
    inline const flowrate& operator-=(const flowrate& __d) {__rep_ -= __d.count(); return *this;}

    inline const flowrate& operator*=(const volume_rep& rhs) {__rep_ *= rhs; return *this;}
    inline const flowrate& operator/=(const volume_rep& rhs) {__rep_ /= rhs; return *this;}
    inline const flowrate& operator%=(const volume_rep& rhs) {__rep_ %= rhs; return *this;}
    inline const flowrate& operator%=(const flowrate& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR flowrate zero() {return flowrate(limits_values<volume_rep>::zero());}
    inline static METRICCONSTEXPR flowrate min()  {return flowrate(limits_values<volume_rep>::min());}
    inline static METRICCONSTEXPR flowrate max()  {return flowrate(limits_values<volume_rep>::max());}
};


// Flowrate /
template <class _Flowrate, class _Rep, bool = __is_flowrate<_Rep>::value>
struct __flowrate_divide_result
{
};

template <class _Flowrate, class _Rep2,
    bool = std::is_convertible<_Rep2,
                          typename std::common_type<typename _Flowrate::rep, _Rep2>::type>::value>
struct __flowrate_divide_imp
{
};

template <class _Rep1, class _Period, class _Rep2>
struct __flowrate_divide_imp<flowrate<_Rep1, _Period>, _Rep2, true>
{
    typedef flowrate<typename std::common_type<_Rep1, _Rep2>::type, _Period> type;
};

template <class _Rep1, class _Period, class _Rep2>
struct __flowrate_divide_result<flowrate<_Rep1, _Period>, _Rep2, false>
    : __flowrate_divide_imp<flowrate<_Rep1, _Period>, _Rep2>
{
};

template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __flowrate_divide_result<flowrate<_Rep1, _Period>, _Rep2>::type
operator/(const flowrate<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef flowrate<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() / static_cast<_Cr>(__s));
}


// Flowrate %
template <class _Rep1, class _Period, class _Rep2>
inline
METRICCONSTEXPR
typename __flowrate_divide_result<flowrate<_Rep1, _Period>, _Rep2>::type
operator%(const flowrate<_Rep1, _Period>& __d, const _Rep2& __s)
{
    typedef typename std::common_type<_Rep1, _Rep2>::type _Cr;
    typedef flowrate<_Cr, _Period> _Cd;
    return _Cd(_Cd(__d).count() % static_cast<_Cr>(__s));
}



typedef flowrate<millilitre, std::chrono::seconds> millilitre_second;
typedef flowrate<millilitre, std::chrono::minutes> millilitre_minute;
typedef flowrate<millilitre, std::chrono::hours>   millilitre_hour;

typedef flowrate<microlitre, std::chrono::seconds> microlitre_second;
typedef flowrate<microlitre, std::chrono::minutes> microlitre_minute;
typedef flowrate<microlitre, std::chrono::hours>   microlitre_hour;

#if _LIBCPP_STD_VER > 17
typedef flowrate<microlitre, std::chrono::days>   microlitre_day;
typedef flowrate<millilitre, std::chrono::days>   millilitre_day;
#endif


namespace literals {
constexpr microlitre_second operator ""_ul_sec(unsigned long long v) { return microlitre_second(v); }
constexpr microlitre_minute operator ""_ul_m(unsigned long long v)   { return microlitre_minute(v); }
constexpr microlitre_hour   operator ""_ul_h(unsigned long long v)   { return microlitre_hour(v);   }
constexpr millilitre_second operator ""_ml_sec(unsigned long long v) { return millilitre_second(v); }
constexpr millilitre_minute operator ""_ml_m(unsigned long long v)   { return millilitre_minute(v); }
constexpr millilitre_hour   operator ""_ml_h(unsigned long long v)   { return millilitre_hour(v);   }

#if _LIBCPP_STD_VER > 17
constexpr microlitre_day    operator ""_ul_d(unsigned long long v)   { return microlitre_day(v);    }
constexpr millilitre_day    operator ""_ml_d(unsigned long long v)   { return millilitre_day(v);    }
#endif
} // namespace literals


// Volume = Flowrate * Time
template <
	typename Volume,
	typename DurationRep,
	typename DurationPer
>
inline Volume operator*(
	const flowrate<Volume, std::chrono::duration<DurationRep, DurationPer>>& s,
	const std::chrono::duration<DurationRep, DurationPer>& d)
{
	return Volume(s.count() * d.count());
}

// Time = Volume / Flowrate
template <
	typename Duration,
	typename VolumeRep,
	typename VolumePer
>
inline Duration operator/(
	const volume<VolumeRep, VolumePer>& v,
	const flowrate<volume<VolumeRep, VolumePer>, Duration>& f)
{
	return Duration(v.count() / f.count());
}

} // namespace metric

#endif // METRICS_FLOWRATE_HPP


