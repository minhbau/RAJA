/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file containing RAJA simd policy definitions.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_policy_simd_register_HPP
#define RAJA_policy_simd_register_HPP

#include<RAJA/pattern/register.hpp>
#include<RAJA/policy/simd/policy.hpp>

#ifdef __AVX__
#include<RAJA/policy/simd/register/avx.hpp>
#endif

namespace RAJA
{
namespace policy
{
  namespace simd
  {

    // This sets the default SIMD register that will be used
    // Individual registers can
    using simd_register = simd_avx_register;
  }
}



  using policy::simd::simd_register;

}

#endif