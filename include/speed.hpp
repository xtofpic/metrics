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

#include "metric_config.hpp"
#include "distance.hpp"
#include <chrono>

namespace metric {

template <typename _distance, typename _time> class speed;

template <typename A> struct __is_speed: __is_specialization<A, speed> {};

}

namespace std
{
    // CLASS TEMPLATE common_type SPECIALIZATIONS
    template<
		typename _DistanceRep1,
		typename _DistancePeriod1,
		typename _TimeRep1,
		typename _TimePeriod1,
		typename _DistanceRep2,
		typename _DistancePeriod2,
		typename _TimeRep2,
		typename _TimePeriod2
    >
        struct common_type<
		metric::speed<metric::distance<_DistanceRep1, _DistancePeriod1>, std::chrono::duration<_TimeRep1, _TimePeriod1> >,
		metric::speed<metric::distance<_DistanceRep2, _DistancePeriod2>, std::chrono::duration<_TimeRep2, _TimePeriod2> >
        >
    {    // common type of two durations

        // Define distance type to use
        typedef typename common_type<metric::distance<_DistanceRep1, _DistancePeriod1>, metric::distance<_DistanceRep2, _DistancePeriod2>>::type distance_type;
        typedef typename common_type<std::chrono::duration<_TimeRep1, _TimePeriod1>, std::chrono::duration<_TimeRep2, _TimePeriod2>>::type duration_type;

        typedef metric::speed<
            distance_type,
            duration_type
        > type;
    };
}

namespace metric {

template <class _FromSpeed, class _ToSpeed,
    class _PeriodDistance = typename std::ratio_divide<typename _FromSpeed::distance_period, typename _ToSpeed::distance_period>::type,
    class _PeriodDuration = typename std::ratio_divide<typename _FromSpeed::duration_period, typename _ToSpeed::duration_period>::type,
    bool = _PeriodDistance::num == 1,
    bool = _PeriodDistance::den == 1,
    bool = _PeriodDuration::num == 1,
    bool = _PeriodDuration::den == 1>
    struct __speed_cast;

template <class _FromSpeed, class _ToSpeed, class _PeriodDistance, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodDistance, _PeriodDuration, true, true, true, true>
{
    inline  _ToSpeed operator()(const _FromSpeed& __fd) const
    {
        return _ToSpeed(static_cast<typename _ToSpeed::distance_rep>(__fd.count()));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodDistance, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodDistance, _PeriodDuration, true, true, false, true>
{
    inline
        _ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::distance_rep>(
            static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_PeriodDuration::num)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodDistance, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodDistance, _PeriodDuration, true, true, true, false>
{
    inline
        _ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::distance_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodDuration::den)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodDistance, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodDistance, _PeriodDuration, false, true, false, true>
{
    inline
        _ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::distance_rep>(
        		(static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodDistance::num)) / static_cast<_Ct>(_PeriodDuration::num)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodDistance, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodDistance, _PeriodDuration, false, true, true, true>
{
    inline
        _ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::power_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodDistance::num)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodVolume, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodVolume, _PeriodDuration, false, false, true, true>
{
    inline
	_ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::distance_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodVolume::num)
										   / static_cast<_Ct>(_PeriodVolume::den) ));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodSpeed, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodSpeed, _PeriodDuration, false, true, true, false>
{
    inline
        _ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::speed_rep>(
        		(static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodSpeed::num)) * static_cast<_Ct>(_PeriodDuration::den)));
    }
};

template <class _FromSpeed, class _ToSpeed, class _PeriodSpeed, class _PeriodDuration>
struct __speed_cast<_FromSpeed, _ToSpeed, _PeriodSpeed, _PeriodDuration, true, false, true, false>
{
    inline
        _ToSpeed operator()(const _FromSpeed& __fd) const
    {
    	typedef typename std::common_type<typename _ToSpeed::distance_rep, typename _FromSpeed::distance_rep, intmax_t>::type _Ct;

        return _ToSpeed(static_cast<typename _ToSpeed::distance_rep>(
        		(static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodDuration::den) / static_cast<_Ct>(_PeriodSpeed::den))));
    }
};


template <class _ToSpeed, class _Vol, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_speed<_ToSpeed>::value,
    _ToSpeed
>::type
speed_cast(const speed<_Vol, _Period>& __fd)
{
    return __speed_cast<speed<_Vol, _Period>, _ToSpeed>()(__fd);
}


template <typename _Distance, typename _Time>
class speed
{
    static_assert(!__is_speed<typename _Distance::rep>::value, "A speed representation can not be a speed");
    static_assert(__is_distance<_Distance>::value, "First template paramater of speed must be a distance");
    // static_assert(std::chrono::__is_duration<_Time>::value, "Second template paramater of speed must be a duration"); // not cross compilable.  TODO: Find a fix
    static_assert(std::__is_ratio<typename _Distance::period>::value, "Second template parameter of speed distance must be a std::ratio");
    static_assert(std::__is_ratio<typename _Time::period>::value, "Second template parameter of speed duration must be a std::ratio");
    static_assert(_Distance::period::num > 0, "speed distance period must be positive");
    static_assert(_Time::period::num > 0, "speed duration period must be positive");

public:
    typedef typename _Distance::rep		distance_rep;
    typedef typename _Distance::period	distance_period;
    typedef typename _Time::rep			duration_rep;
    typedef typename _Time::period		duration_period;

private:
    distance_rep __rep_;

public:

    inline METRICCONSTEXPR
    speed() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit speed(const _Rep2& __r,
            typename std::enable_if
            <
               std::is_convertible<_Rep2, distance_rep>::value &&
               (std::is_floating_point<distance_rep>::value ||
               !std::is_floating_point<_Rep2>::value)
            >::type* = 0)
                : __rep_(__r) {}

    // conversions
    template <class _Distance2, class _Time2>
        inline METRICCONSTEXPR
		speed(const speed<_Distance2, _Time2>& __d /*,  TODO: Re-activate this
            typename std::enable_if
            <
                __no_overflow<typename _Distance2::distance_period, distance_period>::value && (
                std::is_floating_point<distance_rep>::value ||
                (__no_overflow<typename _Distance2::distance_period, distance_period>::type::den == 1 &&
                 !std::is_floating_point<typename _Distance2::distance_rep>::value))
            >::type* = 0*/)
                : __rep_(metric::speed_cast<speed>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR distance_rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR speed  operator+() const {return *this;}
    inline METRICCONSTEXPR speed  operator-() const {return speed(-__rep_);}
    inline const speed& operator++()      {++__rep_; return *this;}
    inline const speed  operator++(int)   {return speed(__rep_++);}
    inline const speed& operator--()      {--__rep_; return *this;}
    inline const speed  operator--(int)   {return speed(__rep_--);}

    inline const speed& operator+=(const speed& __d) {__rep_ += __d.count(); return *this;}
    inline const speed& operator-=(const speed& __d) {__rep_ -= __d.count(); return *this;}

    inline const speed& operator*=(const distance_rep& rhs) {__rep_ *= rhs; return *this;}
    inline const speed& operator/=(const distance_rep& rhs) {__rep_ /= rhs; return *this;}
    inline const speed& operator%=(const distance_rep& rhs) {__rep_ %= rhs; return *this;}
    inline const speed& operator%=(const speed& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR speed zero() {return speed(limits_values<distance_rep>::zero());}
    inline static METRICCONSTEXPR speed min()  {return speed(limits_values<distance_rep>::min());}
    inline static METRICCONSTEXPR speed max()  {return speed(limits_values<distance_rep>::max());}
};


// typedef speed<millilitre, std::chrono::seconds> millilitre_second;
typedef speed<micrometre, std::chrono::seconds> micrometre_second;
typedef speed<micrometre, std::chrono::minutes> micrometre_minute;
typedef speed<micrometre, std::chrono::hours  > micrometre_hour;
typedef speed<millimetre, std::chrono::seconds> millimetre_second;
typedef speed<millimetre, std::chrono::minutes> millimetre_minute;
typedef speed<millimetre, std::chrono::hours  > millimetre_hour;
typedef speed<     metre, std::chrono::seconds> metre_second;
typedef speed<     metre, std::chrono::minutes> metre_minute;
typedef speed<     metre, std::chrono::hours  > metre_hour;
typedef speed< kilometre, std::chrono::hours  > kilometre_hour;
typedef speed<      mile, std::chrono::hours  > mph;

#if _LIBCPP_STD_VER > 17
typedef speed<     metre, std::chrono::days   > metre_day;
typedef speed<millimetre, std::chrono::days   > millimetre_day;
#endif


namespace literals {
// constexpr microlitre_second operator ""_ul_sec(unsigned long long v) { return microlitre_second(v); }

constexpr micrometre_second operator ""_um_sec(unsigned long long v) { return   micrometre_second(v); }
constexpr micrometre_minute operator ""_um_m(unsigned long long v)   { return   micrometre_minute(v); }
constexpr   micrometre_hour operator ""_um_h(unsigned long long v)   { return   micrometre_hour(v);   }
constexpr millimetre_second operator ""_mm_sec(unsigned long long v) { return   millimetre_second(v); }
constexpr millimetre_minute operator ""_mm_m(unsigned long long v)   { return   millimetre_minute(v); }
constexpr   millimetre_hour operator ""_mm_h(unsigned long long v)   { return   millimetre_hour(v);   }
constexpr      metre_second operator ""_m_sec(unsigned long long v)  { return   metre_second(v);      }
constexpr      metre_minute operator ""_m_m(unsigned long long v)    { return   metre_minute(v);      }
constexpr        metre_hour operator ""_m_h(unsigned long long v)    { return   metre_hour(v);        }
constexpr    kilometre_hour operator ""_km_h(unsigned long long v)   { return   kilometre_hour(v);    }
constexpr               mph operator ""_mph(unsigned long long v)    { return   mph(v);               }

#if _LIBCPP_STD_VER > 17
constexpr    millimetre_day operator ""_mm_d(unsigned long long v)   { return   millimetre_day(v);    }
constexpr         metre_day operator ""_m_d(unsigned long long v)    { return   metre_day(v);         }
#endif
} // namespace literals


// Distance = Speed * Time
template <
	typename Distance,
	typename DurationRep,
	typename DurationPer
>
inline Distance operator*(
	const speed<Distance, std::chrono::duration<DurationRep, DurationPer>>& s,
	const std::chrono::duration<DurationRep, DurationPer>& d)
{
	return Distance(s.count() * d.count());
}

// Time = Distance / Speed
template <
	typename Duration,
	typename DistanceRep,
	typename DistancePer
>
inline Duration operator/(
	const distance<DistanceRep, DistancePer>& v,
	const speed<distance<DistanceRep, DistancePer>, Duration>& f)
{
	return Duration(v.count() / f.count());
}

// Speed = Distance / Time
template <
	typename DistanceRep,
	typename DistancePer,
	typename DurationRep,
	typename DurationPer
>
inline speed<distance<DistanceRep, DistancePer>, std::chrono::duration<DurationRep, DurationPer>> operator/(
	const distance<DistanceRep, DistancePer>& v,
	const std::chrono::duration<DurationRep, DurationPer>& d)
{
	return speed<distance<DistanceRep, DistancePer>, std::chrono::duration<DurationRep, DurationPer>>(v.count() / d.count());
}


} // namespace metric

#endif // METRICS_SPEED_HPP
















