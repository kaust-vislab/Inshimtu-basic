/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef PIPELINE_HXXDER
#define PIPELINE_HXXDER

#include "pipeline.h"

#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


//----------------------------------------------

// Example: templates
// http://www.baryudin.com/blog/boost-variant-and-lambda-functions-c-11.html
template <typename ReturnT, typename... Lambdas>
struct lambda_visitor;

template <typename ReturnT, typename L1, typename... Lambdas>
struct lambda_visitor< ReturnT, L1 , Lambdas...> : public L1, public lambda_visitor<ReturnT, Lambdas...>
{
  using L1::operator();
  using lambda_visitor< ReturnT , Lambdas...>::operator();
  lambda_visitor(L1 l1, Lambdas... lambdas) : L1(l1), lambda_visitor< ReturnT , Lambdas...> (lambdas...) {}
};

template <typename ReturnT, typename L1>
struct lambda_visitor<ReturnT, L1> : public L1, public boost::static_visitor<ReturnT>
{
  using L1::operator();
  lambda_visitor(L1 l1) : L1(l1), boost::static_visitor<ReturnT>() {}
};

template <typename ReturnT>
struct lambda_visitor<ReturnT> : public boost::static_visitor<ReturnT>
{
  lambda_visitor() : boost::static_visitor<ReturnT>() {}
};

template <typename ReturnT, typename... Lambdas>
lambda_visitor<ReturnT, Lambdas...> make_lambda_visitor(Lambdas... lambdas)
{
  return { lambdas... };
}


#endif

