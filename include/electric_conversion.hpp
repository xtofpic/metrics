// -*- C++ -*-
//
//===---------------------------- distance ------------------------------------===//
//
// Copyright (c) 2019, Christophe Pijcke
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses.
//
//===----------------------------------------------------------------------===//

#ifndef METRICS_ELECTRIC_CONVERSION_HPP
#define METRICS_ELECTRIC_CONVERSION_HPP

#include "voltage.hpp"
#include "electricresistance.hpp"
#include "electriccurrent.hpp"
#include "power.hpp"

namespace metric {


template <template<class, class> class Unit, typename _Rep1, typename _Period1, typename _Rep2, typename _Period2>
struct __operator_mixunit_result
{
    typedef Unit<typename std::common_type<_Rep1, _Rep2>::type,
        std::ratio< GCD<_Period1::num, _Period2::num>::value,
                    LCM<_Period1::den, _Period2::den>::value> > type;
};


// U = R * I
template <typename ResistanceRep, typename ResistancePeriod, typename CurrentRep, typename CurrentPeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<voltage, ResistanceRep, ResistancePeriod, CurrentRep, CurrentPeriod>::type
operator*(
	const electricresistance<ResistanceRep, ResistancePeriod>& r,
	const electriccurrent<CurrentRep, CurrentPeriod>& i)
{
    typedef typename __operator_mixunit_result<voltage, ResistanceRep, ResistancePeriod, CurrentRep, CurrentPeriod>::type _Cd;
    typedef electricresistance<typename _Cd::rep, typename _Cd::period> _R;
    typedef electriccurrent<typename _Cd::rep, typename _Cd::period> _I;
	return _Cd(_R(r).count() * _I(i).count() * _Cd::period::num / _Cd::period::den);
}

template <typename ResistanceRep, typename ResistancePeriod, typename CurrentRep, typename CurrentPeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<voltage, ResistanceRep, ResistancePeriod, CurrentRep, CurrentPeriod>::type
operator*(
	const electriccurrent<CurrentRep, CurrentPeriod>& i,
	const electricresistance<ResistanceRep, ResistancePeriod>& r)
{
    typedef typename __operator_mixunit_result<voltage, ResistanceRep, ResistancePeriod, CurrentRep, CurrentPeriod>::type _Cd;
    typedef electricresistance<typename _Cd::rep, typename _Cd::period> _R;
    typedef electriccurrent<typename _Cd::rep, typename _Cd::period> _I;
	return _Cd(_R(r).count() * _I(i).count() * _Cd::period::num / _Cd::period::den);
}


// R = U / I
template <typename VoltageRep, typename VoltagePeriod, typename CurrentRep, typename CurrentPeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<electricresistance, VoltageRep, VoltagePeriod, CurrentRep, CurrentPeriod>::type
operator/(
	const voltage<VoltageRep, VoltagePeriod>& v,
	const electriccurrent<CurrentRep, CurrentPeriod>& i)
{
    typedef typename __operator_mixunit_result<electricresistance, VoltageRep, VoltagePeriod, CurrentRep, CurrentPeriod>::type _Cd;
    typedef voltage<typename _Cd::rep, typename _Cd::period> _V;
    typedef electriccurrent<typename _Cd::rep, typename _Cd::period> _I;
    return _Cd((_V(v).count() * _Cd::period::den) / (_I(i).count() * _Cd::period::num));
}


// I = U / R
template <typename VoltageRep, typename VoltagePeriod, typename ResistanceRep, typename ResistancePeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<electriccurrent, VoltageRep, VoltagePeriod, ResistanceRep, ResistancePeriod>::type
operator/(
	const voltage<VoltageRep, VoltagePeriod>& v,
	const electricresistance<ResistanceRep, ResistancePeriod>& r)
{
    typedef typename __operator_mixunit_result<electriccurrent, VoltageRep, VoltagePeriod, ResistanceRep, ResistancePeriod>::type _Cd;
    typedef voltage<typename _Cd::rep, typename _Cd::period> _V;
    typedef electricresistance<typename _Cd::rep, typename _Cd::period> _R;
    return _Cd((_V(v).count() * _Cd::period::den) / (_R(r).count() * _Cd::period::num));
}


// P = U * I
template <typename VoltageRep, typename VoltagePeriod, typename CurrentRep, typename CurrentPeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<power, VoltageRep, VoltagePeriod, CurrentRep, CurrentPeriod>::type
operator*(
	const voltage<VoltageRep, VoltagePeriod>& u,
	const electriccurrent<CurrentRep, CurrentPeriod>& i)
{
    typedef typename __operator_mixunit_result<power, VoltageRep, VoltagePeriod, CurrentRep, CurrentPeriod>::type _Cd;
    typedef voltage<typename _Cd::rep, typename _Cd::period> _U;
    typedef electriccurrent<typename _Cd::rep, typename _Cd::period> _I;
	return _Cd(_U(u).count() * _I(i).count() * _Cd::period::num / _Cd::period::den);
}

template <typename VoltageRep, typename VoltagePeriod, typename CurrentRep, typename CurrentPeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<power, VoltageRep, VoltagePeriod, CurrentRep, CurrentPeriod>::type
operator*(
	const electriccurrent<CurrentRep, CurrentPeriod>& i,
	const voltage<VoltageRep, VoltagePeriod>& u)
{
    typedef typename __operator_mixunit_result<power, VoltageRep, VoltagePeriod, CurrentRep, CurrentPeriod>::type _Cd;
    typedef voltage<typename _Cd::rep, typename _Cd::period> _U;
    typedef electriccurrent<typename _Cd::rep, typename _Cd::period> _I;
	return _Cd(_U(u).count() * _I(i).count() * _Cd::period::num / _Cd::period::den);
}


// I = P / U
template <typename PowerRep, typename PowerPeriod, typename VoltageRep, typename VoltagePeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<electriccurrent, PowerRep, PowerPeriod, VoltageRep, VoltagePeriod>::type
operator/(
	const power<PowerRep, PowerPeriod>& p,
	const voltage<VoltageRep, VoltagePeriod>& v)
{
    typedef typename __operator_mixunit_result<electriccurrent, PowerRep, PowerPeriod, VoltageRep, VoltagePeriod>::type _Cd;
    typedef power<typename _Cd::rep, typename _Cd::period> _P;
    typedef voltage<typename _Cd::rep, typename _Cd::period> _V;
    return _Cd((_P(p).count() * _Cd::period::den) / (_V(v).count() * _Cd::period::num));
}

// U = P / I
template <typename PowerRep, typename PowerPeriod, typename CurrentRep, typename CurrentPeriod>
inline
METRICCONSTEXPR
typename __operator_mixunit_result<voltage, PowerRep, PowerPeriod, CurrentRep, CurrentPeriod>::type
operator/(
	const power<PowerRep, PowerPeriod>& p,
	const electriccurrent<CurrentRep, CurrentPeriod>& i)
{
    typedef typename __operator_mixunit_result<voltage, PowerRep, PowerPeriod, CurrentRep, CurrentPeriod>::type _Cd;
    typedef power<typename _Cd::rep, typename _Cd::period> _P;
    typedef electriccurrent<typename _Cd::rep, typename _Cd::period> _I;
    return _Cd((_P(p).count() * _Cd::period::den) / (_I(i).count() * _Cd::period::num));
}



} // namespace metric

#endif // METRICS_ELECTRIC_CONVERSION_HPP
