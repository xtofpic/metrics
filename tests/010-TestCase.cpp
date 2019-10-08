// 010-TestCase.cpp

// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../include/metrics.hpp"


TEST_CASE( "Mass conversion (pass)", "[single-file]" )
{
	metric::kilogram _12kg(12);
	REQUIRE(_12kg == metric::gram(12000));
	REQUIRE(_12kg == metric::milligram(12000000));
	REQUIRE(_12kg == metric::microgram(12000000000));
	REQUIRE(metric::ton(12) == metric::kilogram(12000));
	REQUIRE(metric::ton(12) != metric::kilogram(12));
	metric::kilogram _kg10plus2(10);
	_kg10plus2 += metric::kilogram(2);
	REQUIRE(_kg10plus2 == metric::nanogram(12000000000000));
}



TEST_CASE( "Distances conversion (pass)", "[single-file]" )
{
	metric::kilometre _12km(12);
	REQUIRE(_12km == metric::metre(12000));
	REQUIRE(_12km == metric::millimetre(12000000));
	REQUIRE(_12km == metric::micrometre(12000000000));
	REQUIRE(metric::megametre(12) == metric::kilometre(12000));
	REQUIRE(metric::megametre(12) != metric::kilometre(12));
	metric::kilometre _km14minus2(14);
	_km14minus2 -= metric::kilometre(2);
	REQUIRE(_km14minus2 == metric::nanometre(12000000000000));
}


TEST_CASE( "Electric current conversion (pass)", "[single-file]" )
{
	metric::ampere _12amp(12);
	auto _4amp = _12amp / 3;
	REQUIRE(_4amp.count() == 4);
}

TEST_CASE( "Energy conversion (pass)", "[single-file]" )
{
	metric::milliwatthour _40k_mwh(40000);
	REQUIRE(_40k_mwh == metric::watthour(40));
	REQUIRE(_40k_mwh == metric::microwatthour(40000000));
	std::cout << "1 joule := " << metric::energy_cast<metric::microwatthour>(metric::joule(1)).count() << " microWatt/hour." << std::endl;
	REQUIRE(metric::joule(1) >= metric::microwatthour(277));
	REQUIRE(metric::joule(1) < metric::microwatthour(278));
	std::cout << "1 calorie := " << metric::energy_cast<metric::microwatthour>(metric::calorie(1)).count() << " microWatt/hour." << std::endl;
	REQUIRE(metric::calorie(1) >= metric::microwatthour(1163));
	REQUIRE(metric::calorie(1) < metric::microwatthour(1164));
}

TEST_CASE( "ForceMass conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::millinewton(1000) == metric::newton(1));
	REQUIRE(metric::decanewton(1) == metric::newton(10));
	REQUIRE(metric::decanewton(1) == metric::newton(10));
	REQUIRE(metric::gramforce(1000) > metric::millinewton(9806));
	REQUIRE(metric::gramforce(1000) < metric::millinewton(9807));
	std::cout << "1000 gramforce := " << metric::force_cast<metric::millinewton>(metric::gramforce(1000)).count() << " millinewton." << std::endl;
}

TEST_CASE( "Frequency conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::gigahertz(1) == metric::millihertz(1000000000000));
}

TEST_CASE( "Power conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::gigawatt(1) == metric::milliwatt(1000000000000));
}

// Test of pressure
TEST_CASE( "Pressure conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::hectopascal(100) == metric::millibar(100));
	REQUIRE(metric::pascal(1000000000000) == metric::terapascal(1));
	REQUIRE(metric::bar(1) == metric::microbar(1000000));
	std::cout << "1 bar := " << metric::pressure_cast<metric::millimetremercure>(metric::bar(1)).count() << " mmHg." << std::endl;
	REQUIRE(metric::bar(1) > metric::millimetremercure(750));
	REQUIRE(metric::bar(1) < metric::millimetremercure(751));
}

TEST_CASE( "Speed conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::micrometre_second(10000) == metric::metre_day(864));  // 1 micrometre second = 60/min = 3600/hour = 86400/day = 86,4 mm/j = 0,0864m/j
}

TEST_CASE( "Voltage conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::megavolt(1) == metric::nanovolt(1000000000000000));
}

TEST_CASE( "Volume conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::megalitre(1) == metric::nanolitre(1000000000000000));
}

TEST_CASE( "Volumetric flows conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::millilitre_minute(60) == metric::microlitre_second(1000));
}

TEST_CASE( "Angular speed conversion (pass)", "[single-file]" )
{
	REQUIRE(metric::turn_second(1) == metric::turn_hour(3600));
	REQUIRE(metric::degree_second(360) == metric::turn_hour(3600));
}

