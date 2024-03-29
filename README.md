# metrics

C++ library to handle metric type and conversion

[![Build Status](https://travis-ci.org/xtofpic/metrics.svg?branch=master)](https://travis-ci.org/xtofpic/metrics)
[![Build status](https://ci.appveyor.com/api/projects/status/github/xtofpic/metrics?svg=true)](https://ci.appveyor.com/project/xtofpic/metrics)
[![codecov](https://codecov.io/gh/xtofpic/metrics/branch/master/graph/badge.svg)](https://codecov.io/gh/xtofpic/metrics)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/17136e673c9c4af9a6d7034bb332515c)](https://www.codacy.com/manual/xtofpic/metrics?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=xtofpic/metrics&amp;utm_campaign=Badge_Grade)


## Overview

### Motivation

Metrics library provides strongly typed types to make interfaces and API precise.
It also provide implicit conversions on similar types, and explicit conversions on arithmetic.

### Description

This library provide types and conversion for:
*   angular speed
*   distance
*   electric current
*   energy
*   force
*   frequency
*   mass
*   power
*   pressure
*   speed
*   voltage
*   volume
*   flowrate

## User's Guide

### Getting Started

#### Installing the library

The Metrics library is a header's only library. Just copy the necessary metrics (.hpp file) into your code.

### Examples

An example of strongly typed signature.

```c++
#include <iostream>
#include <metrics.hpp>

void printSpeed(metric::metre distance, std::chrono::minutes duration) // Enforce type used.
{
	metric::mph speed = distance / duration;
	std::cout << "You're driving at " << speed.count() << " mph" << std::endl;

	// Create a custom speed:
	typedef metric::distance<long long, std::ratio<1852LL, 1LL>> nauticalmile;
	typedef metric::speed<nauticalmile, std::chrono::hours> knot;
	knot speed = distance / duration;
	std::cout << "You're driving at " << speed.count() << " knots" << std::endl;
}

int main()
{
	printSpeed(metric::metre(247530), std::chrono::minutes(128));
	return 0;
}
```

An example of unit conversion

```c++
#include <iostream>
#include <metrics.hpp>

using namespace metric::literals;

template<uint8_t Temperature>
struct waterDensity
{
    static constexpr double temp = static_cast<double>(Temperature);
    // (Air saturated) Formula source: https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4909168/
    static constexpr double value = 999.84847 + (6.337563 * 0.01) * temp + (-8.523829 * 0.001) * temp * temp + (6.943248 * 0.00001) * temp * temp * temp + (-3.821216 * 0.0000001) * temp * temp * temp * temp;
};

template<int8_t Temperature>
struct mercuryDensity
{
    // Formula source: https://www.techniques-ingenieur.fr/base-documentaire/sciences-fondamentales-th8/introduction-aux-constantes-physico-chimiques-42342210/corrections-barometriques-k64/masse-volumique-du-mercure-k64niv10002.html
    static constexpr double value = 13595.1 / (1 + (1.818 * 0.0001 * (Temperature)));
};

template<typename Density, typename MassType, typename MassRatio>
metric::volume<double, MassRatio> operator/ (const metric::mass<MassType, MassRatio>& m, Density d)
{
    return metric::volume<double, MassRatio>(m.count() / Density::value);
}

int main()
{
    std::cout << "10kg of   water at 25°C = " << metric::volume_cast<metric::millilitre>(10_kg /   waterDensity<25>()).count() << " millilitre" << std::endl;
    std::cout << " 1kg of mercury at  5°C = " << metric::volume_cast<metric::microlitre>( 1_kg / mercuryDensity< 5>()).count() << " microlitre" << std::endl;
    return 0;
}
```

## known types

|                       |                   | ratio                  | literal   |
|-                      |-                  |-                       |-          |
| **angularspeed**      | degree / second   | 10/1                   | _degsec   |
|                       | turn / second     | 3600/1                 | _rps      |
|                       | turn / minute     | 60/1                   | _rpm      |
|                       | turn / hour       | 1/1                    | _rph      |
| **distance**          | attometre         | atto                   | _am       |
|                       | femtometre        | femto                  | _fm       |
|                       | picometre         | pico                   | _pm       |
|                       | nanometre         | nano                   | _nm       |
|                       | micrometre        | micro                  | _um       |
|                       | millimetre        | milli                  | _mm       |
|                       | centimetre        | centi                  | _cm       |
|                       | metre             | 1/1                    | _m        |
|                       | kilometre         | kilo                   | _km       |
|                       | megametre         | mega                   | _Mm       |
|                       | gigametre         | giga                   | _Gm       |
|                       | terametre         | tera                   | _Tm       |
|                       | petametre         | peta                   | _Pm       |
|                       | exametre          | exa                    | _Em       |
|                       | yard              | 9144/10000             | _yd       |
|                       | inch              | 254/10000              | _in       |
|                       | nauticalmile      | 1852/1                 | _nmi      |
|                       | foot              | 3048/10000             | _ft       |
| **electric current:** | femtoampere       | femto                  | _fA       |
|                       | picoampere        | pico                   | _pA       |
|                       | nanoampere        | nano                   | _nA       |
|                       | microampere       | micro                  | _uA       |
|                       | milliampere       | milli                  | _mA       |
|                       | ampere            | 1/1                    | _A        |
|                       | kiloampere        | kilo                   | _kA       |
|                       | megaampere        | mega                   | _MA       |
| **energy:**           | microwatthour     |                        | _uWh      |
|                       | milliwatthour     |                        | _mWh      |
|                       | watthour          |                        | _Wh       |
|                       | kilowatthour      |                        | _kWh      |
|                       | megawatthour      |                        | _MWh      |
|                       | gigawatthour      |                        | _GWh      |
|                       | terawatthour      |                        | _TWh      |
|                       | petawatthour      |                        | _PWh      |
|                       | joule             |                        | _j        |
|                       | calorie           |                        | _c        |
| **force:**            | millinewton       | milli                  | _mN       |
|                       | newton            | 1/1                    | _N        |
|                       | decanewton        | deca                   | _dN       |
|                       | gramforce         | 980665/100000000       | _gf       |
|                       | kilogramforce     | 980665/100000          | _kgf      |
| **frequency:**        | millihertz        | milli                  | _mHz      |
|                       | hertz             | 1/1                    | _Hz       |
|                       | kilohertz         | kilo                   | _kHz      |
|                       | megahertz         | mega                   | _MHz      |
|                       | gigahertz         | giga                   | _GHz      |
| **mass:**             | nanogram          | nano                   | _ng       |
|                       | microgram         | micro                  | _ug       |
|                       | milligram         | milli                  | _mg       |
|                       | gram              | 1/1                    | _g        |
|                       | kilogram          | kilo                   | _kg       |
|                       | ton               | mega                   | _ton      |
| **power:**            | nanowatt          | nano                   | _nW       |
|                       | microwatt         | micro                  | _uW       |
|                       | milliwatt         | milli                  | _mW       |
|                       | watt              | 1/1                    | _W        |
|                       | kilowatt          | kilo                   | _kW       |
|                       | megawatt          | mega                   | _MW       |
|                       | gigawatt          | giga                   | _GW       |
| **pressure:**         | millimetremercury | 1/760                  | _mmHg     |
|                       | pascal            | 1/101325               | _Pa       |
|                       | hectopascal       | 100/101325             | _hPa      |
|                       | kilopascal        | 1000/101325            | _kPa      |
|                       | megapascal        | 1000000/101325         | _MPa      |
|                       | gigapascal        | 1000000000/101325      | _GPa      |
|                       | terapascal        | 1000000000000/101325   | _TPa      |
|                       | bar               | 1000000/1013250        | _bar      |
|                       | millibar          | 1000/1013250           | _mbar     |
|                       | microbar          | 1/1013250              | _ubar     |
| **speed:**            | micrometre/second |                        | _um_sec   |
|                       | micrometre/minute |                        | _um_m     |
|                       | micrometre/hour   |                        | _um_h     |
|                       | millimetre/second |                        | _mm_sec   |
|                       | millimetre/minute |                        | _mm_m     |
|                       | millimetre/hour   |                        | _mm_h     |
|                       | millimetre/day    |                        | _mm_d     |
|                       | metre/second      |                        | _m_sec    |
|                       | metre/minute      |                        | _m_m      |
|                       | metre/hour        |                        | _m_h      |
|                       | metre/day         |                        | _m_d      |
| **voltage:**          | nanovolt          | nano                   | _nV       |
|                       | microvolt         | micro                  | _uV       |
|                       | millivolt         | milli                  | _mV       |
|                       | volt              | 1/1                    | _V        |
|                       | kilovolt          | kilo                   | _kV       |
|                       | megavolt          | mega                   | _MV       |
| **volume:**           | nanolitre         | nano                   | _nl       |
|                       | microlitre        | micro                  | _ul       |
|                       | millilitre        | milli                  | _ml       |
|                       | litre             | 1/1                    | _l        |
|                       | kilolitre         | kilo                   | _kl       |
|                       | megalitre         | mega                   | _Ml       |
| **flowrate:**         | microlitre/second |                        | _ul_sec   |
|                       | microlitre/minute |                        | _ul_m     |
|                       | microlitre/hour   |                        | _ul_h     |
|                       | millilitre/second |                        | _ml_sec   |
|                       | millilitre/minute |                        | _ml_m     |
|                       | millilitre/hour   |                        | _ml_h     |
|                       | millilitre/day    |                        | _ml_d     |
