/*! \file ran.hpp
    \brief Simplifying wrapper to Boost random number facilities

    Copyright 2009 Tuomas Tonteri. This file is licensed as public domain.

    Usage example: 

	Ran01<float> gen; // Call constructor
        gen(); // Generate a random number

        Ran<double> gen(seed,min,max) // Call constructor
        gen(); // Generate a random number
*/

#ifndef RAN_H
#define RAN_H

#include <boost/random.hpp>

#ifdef unix
#include <fstream>
#else
#include <ctime>
#endif

#ifdef _MSC_VER
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#endif

typedef boost::mt19937 base_generator_type;
//typedef boost::minstd_rand base_generator_type;
//typedef boost::rand48 base_generator_type;
//typedef boost::lagged_fibonacci607 base_generator_type;

#ifdef unix
uint64_t GetSeed()
{
        uint64_t seed;
        std::ifstream urandom;
        urandom.open("/dev/urandom");
        urandom.read(reinterpret_cast<char*> (&seed), sizeof (seed));
        urandom.close();
	return seed;
}
#else
// TODO: Implement a proper GetSeed() on other platforms
uint64_t GetSeed()
{
	return time(NULL);
}
#endif

/// Uniform distribution random numbers between 0 .. 1
template <class realtype>
class Ran01 {
        base_generator_type generator;
        boost::uniform_01<base_generator_type,realtype> gen;
public:
        Ran01(uint64_t seed) : generator((uint64_t)seed), gen(generator) { }
        Ran01(void) : generator((uint64_t)GetSeed()), gen(generator) { }

        realtype operator () (void) { return gen(); } ///< Generate a random number
};

/// Binomial distribution random numbers, with given n and p.
// See http://stackoverflow.com/questions/2791477/random-numbers-from-binomial-distribution
template <class realtype>
class RanBinomial {
    base_generator_type generator;
    
public:
    RanBinomial(uint64_t seed) : generator((uint64_t)seed) {}
    RanBinomial(void) : generator((uint64_t)GetSeed()) {}

    realtype operator()(int n, double p) {
        boost::binomial_distribution<> my_binomial = boost::binomial_distribution<>(n, p);
        boost::variate_generator<base_generator_type &, boost::binomial_distribution<> >
            gen(generator, my_binomial);
        return gen();
    }
};


/// Uniform distribution integer random numbers
template <class inttype>
class IRan {
        base_generator_type generator;
        boost::uniform_int<inttype> uni_dist;
        boost::variate_generator<base_generator_type, boost::uniform_int<inttype> > gen;
public:
	IRan(uint64_t seed, inttype min, inttype max) : generator((uint64_t)seed), uni_dist(min,max), gen(generator,uni_dist) { }
	IRan(inttype min, inttype max) : generator((uint64_t)GetSeed()), uni_dist(min,max), gen(generator,uni_dist) { }

        inttype operator () (void) { return gen(); } ///< Generate a random number
};

/// Uniform distribution random numbers
template <class realtype>
class Ran {
	base_generator_type generator;
	boost::uniform_real<realtype> uni_dist;
	boost::variate_generator<base_generator_type, boost::uniform_real<realtype> > gen;
public:
	Ran(uint64_t seed, int min, int max) : generator((uint64_t)seed), uni_dist(min,max), gen(generator,uni_dist) { }
	Ran(int min, int max) : generator((uint64_t)GetSeed()), uni_dist(min,max), gen(generator,uni_dist) { }

	realtype operator () (void) { return gen(); } ///< Generate a random number
};


/// Normal distribution random numbers
template <class realtype>
class Nran {
	base_generator_type generator;
	boost::normal_distribution<realtype> norm_dist;
	boost::variate_generator<base_generator_type, boost::normal_distribution<realtype> > gen;
public:
	Nran(uint64_t seed, int mean, int sigma) : generator((uint64_t)seed), norm_dist(mean,sigma), gen(generator,norm_dist) { }
	Nran(int mean, int sigma) : generator((uint64_t)GetSeed()), norm_dist(mean,sigma), gen(generator,norm_dist) { }
	Nran(uint64_t seed) : generator((uint64_t)seed), norm_dist(0,1), gen(generator,norm_dist) { }
	Nran() : generator((uint64_t)GetSeed()), norm_dist(0,1), gen(generator,norm_dist) { }

	realtype operator () (void) { return gen(); } ///< Generate a random number
};

#endif
