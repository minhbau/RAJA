//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-19, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For details about use and distribution, please read RAJA/LICENSE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

///
/// Source file containing tests for arithmetic atomic operations
///

#include "tests/test-forall-atomic-ref-math.hpp"

#include "../test-forall-atomic-utils.hpp"

#if defined(RAJA_ENABLE_OPENMP)
using OmpAtomicForallRefMathTypes = Test< camp::cartesian_product<
                                                                  AtomicOmpExecs,
                                                                  AtomicOmpPols,
                                                                  HostResourceList,
                                                                  AtomicTypeList >
                                        >::Types;

INSTANTIATE_TYPED_TEST_SUITE_P( OmpTest,
                                SeqForallAtomicRefMathFunctionalTest,
                                OmpAtomicForallRefMathTypes );
#endif
