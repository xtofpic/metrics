# metrics
C++ library to handle metric type and conversion

[![Build Status](https://travis-ci.org/xtofpic/metrics.svg?branch=master)](https://travis-ci.org/xtofpic/metrics)


## Overview

### Motivation

Metrics library provides strongly typed types to make interfaces and API precise.
It also provide implicit conversions on similar type.

Actually, this library is used/tested on macosx/clang and visual studio only.


### Description

This library provide types and conversion for the following metrics:

**angularspeed:**
* degree / second
* turn / second
* turn / minute
* turn / hour

**distance:**
* nanometre
* micrometre
* millimetre
* centimetre
* metre
* kilometre
* megametre

**electric current:**
* nanoampere
* microampere
* milliampere
* ampere
* kiloampere
* megaampere

**energy:**
* microwatt / hour
* milliwatt / hour
* watt / hour
* kilowatt / hour
* joule
* calorie

**force:**
* millinewton
* newton
* decanewton
* gramforce
* kilogramforce

**frequency:**
* millihertz
* hertz
* kilohertz
* megahertz
* gigahertz

**mass:**
* nanogram
* microgram
* milligram
* gram
* kilogram
* ton

**power:**
* nanowatt
* microwatt
* milliwatt
* watt
* kilowatt
* megawatt
* gigawatt

**pressure:**
* millimetremercure
* pascal
* hectopascal
* kilopascal
* megapascal
* gigapascal
* terapascal
* bar
* millibar
* microbar

**speed:**
* micrometre / second
* micrometre / minute
* micrometre / hour
* millimetre / second
* millimetre / minute
* millimetre / hour
* millimetre / day
* metre / second
* metre / minute
* metre / hour
* metre / day

**voltage:**
* nanovolt
* microvolt
* millivolt
* volt
* kilovolt
* megavolt

**volume:**
* nanolitre
* microlitre
* millilitre
* litre
* kilolitre
* megalitre

**volumetric flow:**
* microlitre / second
* microlitre / minute
* microlitre / hour
* millilitre / second
* millilitre / minute
* millilitre / hour
* millilitre / day


## User's Guide

### Getting Started

#### Installing the library

The Metrics library is a header's only library. Just copy the necessary metrics (.hpp file) into your code.


### Examples


```c++
#include <iostream>
#include <mass.hpp>

void printMass(const metric::gram& gram)
{
	std::cout << "You send me a weight of " << gram.count() << " gram" << std::endl;
}


int main()
{
	metric::kilogram _12kg(12);
	printMass(_12kg);	// Print: You send me a weight of 12000 gram

	if (_12kg == metric::milligram(12000000))
	{
		std::cout << "Great, this library can make some sort of math." << std::endl;
	}

	return 0;
}
```




