/*
 * File: SL_Random.h
 * Version: 1.0
 * Last modified on Fri Jul 22 16:44:36 1994 by eroberts
 * -----------------------------------------------------
 * This interface provides several functions for generating
 * pseudo-random numbers.
 */

#ifndef SOCKETLITE_RANDOM_H
#define SOCKETLITE_RANDOM_H

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	#pragma once
#endif
#include "SL_Config.h"

#ifndef SL_RAND_MAX
#define SL_RAND_MAX ((int) ((unsigned) ~0 >> 1))
#endif

class SOCKETLITE_API SL_Random
{
private:
    SL_Random()
    {
    }

    ~SL_Random()
    {
    }

public:

	/*
	* Function: randomize
	* Usage: randomize();
	* -------------------
	* This function sets the random seed so that the random sequence
	* is unpredictable.  During the debugging phase, it is best not
	* to call this function, so that program behavior is repeatable.
	*/
	static void    randomize(int seed = 0);

	/*
	* Function: randomInteger
	* Usage: n = randomInteger(low, high);
	* ------------------------------------
	* This function returns a random integer in the range low to high,
	* inclusive.
	*/
	static int	   randomInteger(); //random number range [0,RAND_MAX]
	static int	   randomInteger(int low, int high);

	/*
	* Function: randomReal
	* Usage: d = randomReal(low, high);
	* ---------------------------------
	* This function returns a random real number in the half-open
	* interval [low .. high), meaning that the result is always
	* greater than or equal to low but strictly less than high.
	*/
	static double  randomReal(); //random number range [0,1]
	static double  randomReal(double low, double high);

	/*
	* Function: randomChance
	* Usage: if (randomChance(p)) . . .
	* ---------------------------------
	* The randomChance function returns TRUE with the probability
	* indicated by p, which should be a floating-point number between
	* 0 (meaning never) and 1 (meaning always).  For example, calling
	* randomChance(.30) returns TRUE 30 percent of the time.
	*/
	static bool    randomChance(double p);

};

#endif
