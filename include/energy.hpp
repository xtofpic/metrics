// -*- C++ -*-
//
//===---------------------------- energy ------------------------------------===//
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

#ifndef METRICS_ENERGY_HPP
#define METRICS_ENERGY_HPP

#include "metric_config.hpp"
#include "power.hpp"
#include <chrono>

namespace metric {

template <typename _power, typename _time> class energy;

template <typename A> struct __is_energy: __is_specialization<A, energy> {};

}

namespace std
{
    // CLASS TEMPLATE common_type SPECIALIZATIONS
    template<
		typename _PowerRep1,
		typename _PowerPeriod1,
		typename _TimeRep1,
		typename _TimePeriod1,
		typename _PowerRep2,
		typename _PowerPeriod2,
		typename _TimeRep2,
		typename _TimePeriod2
    >
        struct common_type<
		metric::energy<metric::power<_PowerRep1, _PowerPeriod1>, std::chrono::duration<_TimeRep1, _TimePeriod1> >,
		metric::energy<metric::power<_PowerRep2, _PowerPeriod2>, std::chrono::duration<_TimeRep2, _TimePeriod2> >
        >
    {    // common type of two durations

        // Define power type to use
        typedef typename common_type<metric::power<_PowerRep1, _PowerPeriod1>, metric::power<_PowerRep2, _PowerPeriod2>>::type power_type;
        typedef typename common_type<std::chrono::duration<_TimeRep1, _TimePeriod1>, std::chrono::duration<_TimeRep2, _TimePeriod2>>::type duration_type;

        typedef typename metric::energy<
            power_type,
            duration_type
        > type;
    };
}

namespace metric {

template <class _FromEnergy, class _ToEnergy,
    class _PeriodPower = typename std::ratio_divide<typename _FromEnergy::power_period, typename _ToEnergy::power_period>::type,
    class _PeriodDuration = typename std::ratio_divide<typename _FromEnergy::duration_period, typename _ToEnergy::duration_period>::type,
    bool = _PeriodPower::num == 1,
    bool = _PeriodPower::den == 1,
    bool = _PeriodDuration::num == 1,
    bool = _PeriodDuration::den == 1>
    struct __energy_cast;

template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, true, true, true, true>
{
    inline  _ToPower operator()(const _FromPower& __fd) const
    {
        return _ToPower(static_cast<typename _ToPower::power_rep>(__fd.count()));
    }
};

template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, true, true, false, true>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
    	typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodDuration::num)));
    }
};

template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, true, true, true, false>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
        typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
            static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_PeriodDuration::den)));
    }
};


template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, false, true, false, true>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
    	typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
        		(static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodPower::num)) * static_cast<_Ct>(_PeriodDuration::num)));
    }
};

template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, false, true, true, true>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
    	typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodPower::num)));
    }
};

/*
template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, false, false, true, true>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
        typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
            static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodPower::num)
										   / static_cast<_Ct>(_PeriodPower::den) ));
    }
};

template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, false, true, true, false>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
        typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
        		(static_cast<_Ct>(__fd.count()) * static_cast<_Ct>(_PeriodPower::num)) * static_cast<_Ct>(_PeriodDuration::den)));
    }
};
*/

template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, true, false, false, true>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
    	typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
        		(static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_PeriodPower::den))
											    * static_cast<_Ct>(_PeriodDuration::num)));

    }
};


template <class _FromPower, class _ToPower, class _PeriodPower, class _PeriodDuration>
struct __energy_cast<_FromPower, _ToPower, _PeriodPower, _PeriodDuration, true, false, true, true>
{
    inline
        _ToPower operator()(const _FromPower& __fd) const
    {
    	typedef typename std::common_type<typename _ToPower::power_rep, typename _FromPower::power_rep, intmax_t>::type _Ct;

        return _ToPower(static_cast<typename _ToPower::power_rep>(
            static_cast<_Ct>(__fd.count()) / static_cast<_Ct>(_PeriodPower::den)));
    }
};


template <class _ToPower, class _Vol, class _Period>
inline
METRICCONSTEXPR
typename std::enable_if
<
    __is_energy<_ToPower>::value,
    _ToPower
>::type
energy_cast(const energy<_Vol, _Period>& __fd)
{
    return __energy_cast<energy<_Vol, _Period>, _ToPower>()(__fd);
}



template <typename _Power, typename _Time>
class energy
{
    static_assert(!__is_energy<typename _Power::rep>::value, "A energy representation can not be a energy");
    static_assert(__is_power<_Power>::value, "First template paramater of energy must be a power");
    // static_assert(std::chrono::__is_duration<_Time>::value, "Second template paramater of energy must be a duration"); // not cross compilable.  TODO: Find a fix
    static_assert(std::__is_ratio<typename _Power::period>::value, "Second template parameter of energy power must be a std::ratio");
    static_assert(std::__is_ratio<typename _Time::period>::value, "Second template parameter of energy duration must be a std::ratio");
    static_assert(_Power::period::num > 0, "energy power period must be positive");
    static_assert(_Time::period::num > 0, "energy duration period must be positive");

public:
    typedef typename _Power::rep		power_rep;
    typedef typename _Power::period		power_period;
    typedef typename _Time::rep			duration_rep;
    typedef typename _Time::period		duration_period;

private:
    power_rep __rep_;

public:

    inline METRICCONSTEXPR
    energy() = default;

    template <class _Rep2>
        inline METRICCONSTEXPR
        explicit energy(const _Rep2& __r,
            typename std::enable_if
            <
               std::is_convertible<_Rep2, power_rep>::value &&
               (std::is_floating_point<power_rep>::value ||
               !std::is_floating_point<_Rep2>::value)
            >::type* = 0)
                : __rep_(__r) {}

    // conversions
    template <class _Power2, class _Time2>
        inline METRICCONSTEXPR
        energy(const energy<_Power2, _Time2>& __d /*,  TODO: Re-activate this
            typename std::enable_if
            <
                __no_overflow<typename _Power2::power_period, power_period>::value && (
                std::is_floating_point<power_rep>::value ||
                (__no_overflow<typename _Power2::power_period, power_period>::type::den == 1 &&
                 !std::is_floating_point<typename _Power2::power_rep>::value))
            >::type* = 0*/)
                : __rep_(metric::energy_cast<energy>(__d).count()) {}

    // observer

    inline METRICCONSTEXPR power_rep count() const {return __rep_;}

    // arithmetic

    inline METRICCONSTEXPR energy  operator+() const {return *this;}
    inline METRICCONSTEXPR energy  operator-() const {return energy(-__rep_);}
    inline const energy& operator++()      {++__rep_; return *this;}
    inline const energy  operator++(int)   {return energy(__rep_++);}
    inline const energy& operator--()      {--__rep_; return *this;}
    inline const energy  operator--(int)   {return energy(__rep_--);}

    inline const energy& operator+=(const energy& __d) {__rep_ += __d.count(); return *this;}
    inline const energy& operator-=(const energy& __d) {__rep_ -= __d.count(); return *this;}

    inline const energy& operator*=(const power_rep& rhs) {__rep_ *= rhs; return *this;}
    inline const energy& operator/=(const power_rep& rhs) {__rep_ /= rhs; return *this;}
    inline const energy& operator%=(const power_rep& rhs) {__rep_ %= rhs; return *this;}
    inline const energy& operator%=(const energy& rhs) {__rep_ %= rhs.count(); return *this;}

    // special values

    inline static METRICCONSTEXPR energy zero() {return energy(limits_values<power_rep>::zero());}
    inline static METRICCONSTEXPR energy min()  {return energy(limits_values<power_rep>::min());}
    inline static METRICCONSTEXPR energy max()  {return energy(limits_values<power_rep>::max());}
};


typedef energy<microwatt, std::chrono::hours  > microwatthour;
typedef energy<milliwatt, std::chrono::hours  > milliwatthour;
typedef energy<     watt, std::chrono::hours  > watthour;
typedef energy<     watt, std::chrono::seconds> wattsecond;
typedef energy< kilowatt, std::chrono::hours  > kilowatthour;
typedef energy< megawatt, std::chrono::hours  > megawatthour;
typedef energy< gigawatt, std::chrono::hours  > gigawatthour;
typedef energy< terawatt, std::chrono::hours  > terawatthour;
typedef energy< petawatt, std::chrono::hours  > petawatthour;
typedef energy<     watt, std::chrono::seconds> joule;
typedef energy< kilowatt, std::chrono::seconds> kilojoule;
typedef energy< megawatt, std::chrono::seconds> megajoule;
typedef energy< gigawatt, std::chrono::seconds> gigajoule;
typedef energy<power<long long, std::ratio<   180, 154800>>, std::chrono::hours>     calorie;
typedef energy<power<long long, std::ratio<180000, 154800>>, std::chrono::hours> kilocalorie;

namespace literals {
constexpr microwatthour operator ""_uWh(unsigned long long v) { return microwatthour(v); }
constexpr milliwatthour operator ""_mWh(unsigned long long v) { return milliwatthour(v); }
constexpr      watthour operator ""_Wh( unsigned long long v) { return      watthour(v); }
constexpr    wattsecond operator ""_Ws( unsigned long long v) { return    wattsecond(v); }
constexpr  kilowatthour operator ""_kWh(unsigned long long v) { return  kilowatthour(v); }
constexpr  megawatthour operator ""_MWh(unsigned long long v) { return  megawatthour(v); }
constexpr  gigawatthour operator ""_GWh(unsigned long long v) { return  gigawatthour(v); }
constexpr  terawatthour operator ""_TWh(unsigned long long v) { return  terawatthour(v); }
constexpr  petawatthour operator ""_PWh(unsigned long long v) { return  petawatthour(v); }
constexpr         joule operator ""_j(  unsigned long long v) { return         joule(v); }
// constexpr       calorie operator ""_c(  unsigned long long v) { return       calorie(v); } TODO
}


// Power = Energy / Time
template <
	typename Power,
	typename DurationRep,
	typename DurationPer
>
inline Power operator/(
	const energy<Power, std::chrono::duration<DurationRep, DurationPer>>& e,
	const std::chrono::duration<DurationRep, DurationPer>& d)
{
	return Power(e.count() / d.count());
}

// Time = Energy / Power
template <
	typename Duration,
	typename PowerRep,
	typename PowerPer
>
inline Duration operator/(
	const energy<power<PowerRep, PowerPer>, Duration>& e,
	const power<PowerRep, PowerPer>& p)
{
	return Duration(e.count() / p.count());
}

// Energy = Power * Time
template <
	typename PowerRep,
	typename PowerPer,
	typename DurationRep,
	typename DurationPer
>
inline energy<power<PowerRep, PowerPer>, std::chrono::duration<DurationRep, DurationPer>> operator*(
	const power<PowerRep, PowerPer>& p,
	const std::chrono::duration<DurationRep, DurationPer>& d)
{
	return energy<power<PowerRep, PowerPer>, std::chrono::duration<DurationRep, DurationPer>>(p.count() * d.count());
}


} // namespace metric

#endif // METRICS_ENERGY_HPP

