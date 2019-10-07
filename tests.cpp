// -*- C++ -*-
//
//===---------------------------- tests -----------------------------------===//
//
// Copyright (c) 2018, 2019, Christophe Pijcke
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses.
//
//===----------------------------------------------------------------------===//

#include <iostream>

#include "mass.hpp"
#include "distance.hpp"
#include "electriccurrent.hpp"
#include "energy.hpp"
#include "force.hpp"
#include "frequency.hpp"
#include "power.hpp"
#include "pressure.hpp"
#include "speed.hpp"
#include "voltage.hpp"
#include "volume.hpp"
#include "volumetricflow.hpp"
#include "angularspeed.hpp"


int main(int argc, char* argv[])
{
	// Tests of masses
	metric::kilogram _12kg(12);
	assert(_12kg == metric::gram(12000));
	assert(_12kg == metric::milligram(12000000));
	assert(_12kg == metric::microgram(12000000000));
	assert(metric::ton(12) == metric::kilogram(12000));
	assert(metric::ton(12) != metric::kilogram(12));
	metric::kilogram _kg10plus2(10);
	_kg10plus2 += metric::kilogram(2);
	assert(_kg10plus2 == metric::nanogram(12000000000000));


	// Tests of distances
	metric::kilometre _12km(12);
	assert(_12km == metric::metre(12000));
	assert(_12km == metric::millimetre(12000000));
	assert(_12km == metric::micrometre(12000000000));
	assert(metric::megametre(12) == metric::kilometre(12000));
	assert(metric::megametre(12) != metric::kilometre(12));
	metric::kilometre _km14minus2(14);
	_km14minus2 -= metric::kilometre(2);
	assert(_km14minus2 == metric::nanometre(12000000000000));


	// Tests of electric current
	metric::ampere _12amp(12);
	auto _4amp = _12amp / 3;
	assert(_4amp.count() == 4);


	// Tests of energy
	metric::milliwatthour _40k_mwh(40000);
	assert(_40k_mwh == metric::watthour(40));
	assert(_40k_mwh == metric::microwatthour(40000000));
	std::cout << "1 joule := " << metric::energy_cast<metric::microwatthour>(metric::joule(1)).count() << " microWatt/hour." << std::endl;
	assert(metric::joule(1) >= metric::microwatthour(277));
	assert(metric::joule(1) < metric::microwatthour(278));
	std::cout << "1 calorie := " << metric::energy_cast<metric::microwatthour>(metric::calorie(1)).count() << " microWatt/hour." << std::endl;
	assert(metric::calorie(1) >= metric::microwatthour(1163));
	assert(metric::calorie(1) < metric::microwatthour(1164));


	// Tests of force:
	assert(metric::millinewton(1000) == metric::newton(1));
	assert(metric::decanewton(1) == metric::newton(10));
	assert(metric::decanewton(1) == metric::newton(10));
	assert(metric::gramforce(1000) > metric::millinewton(9806));
	assert(metric::gramforce(1000) < metric::millinewton(9807));
	std::cout << "1000 gramforce := " << metric::force_cast<metric::millinewton>(metric::gramforce(1000)).count() << " millinewton." << std::endl;

	// Test of frequency:
	assert(metric::gigahertz(1) == metric::millihertz(1000000000000));

	// Test of power
	assert(metric::gigawatt(1) == metric::milliwatt(1000000000000));

	// Test of pressure
	assert(metric::hectopascal(100) == metric::millibar(100));
	assert(metric::pascal(1000000000000) == metric::terapascal(1));
	assert(metric::bar(1) == metric::microbar(1000000));
	std::cout << "1 bar := " << metric::pressure_cast<metric::millimetremercure>(metric::bar(1)).count() << " mmHg." << std::endl;
	assert(metric::bar(1) > metric::millimetremercure(750));
	assert(metric::bar(1) < metric::millimetremercure(751));


	// Tests of speed
	assert(metric::micrometre_second(10000) == metric::metre_day(864));  // 1 micrometre second = 60/min = 3600/hour = 86400/day = 86,4 mm/j = 0,0864m/j

	// Tests of voltage
	assert(metric::megavolt(1) == metric::nanovolt(1000000000000000));

	// Tests of volume
	assert(metric::megalitre(1) == metric::nanolitre(1000000000000000));

	// Tests of volumetricflows
	assert(metric::millilitre_minute(60) == metric::microlitre_second(1000));

	// Tests of angularspeed
	assert(metric::turn_second(1) == metric::turn_hour(3600));
	assert(metric::degree_second(360) == metric::turn_hour(3600));

	return 0;
}


