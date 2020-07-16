//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#include "RAJA/RAJA.hpp"
#include "counter.hpp"

#include "gtest/gtest.h"


CounterData* plugin_test_data = nullptr;

struct SetupPluginVars
{
  SetupPluginVars()
  {
    if (plugin_test_data == nullptr) {
      plugin_test_data = new CounterData;
    }

    plugin_test_data->capture_platform_active = RAJA::Platform::undefined;
    plugin_test_data->capture_counter_pre     = 0;
    plugin_test_data->capture_counter_post    = 0;
    plugin_test_data->launch_platform_active = RAJA::Platform::undefined;
    plugin_test_data->launch_counter_pre     = 0;
    plugin_test_data->launch_counter_post    = 0;
  }

  SetupPluginVars(SetupPluginVars const&) = delete;
  SetupPluginVars(SetupPluginVars &&) = delete;
  SetupPluginVars& operator=(SetupPluginVars const&) = delete;
  SetupPluginVars& operator=(SetupPluginVars &&) = delete;

  ~SetupPluginVars()
  {
    if (plugin_test_data != nullptr) {
      delete plugin_test_data; plugin_test_data = nullptr;
    }
  }
};


struct PluginTestCallable
{
  PluginTestCallable(CounterData* data_optr)
    : m_data_optr(data_optr)
    , m_data_iptr(plugin_test_data)
  {
    clear_data();
  }

  PluginTestCallable(PluginTestCallable const& rhs)
    : m_data_optr(rhs.m_data_optr)
    , m_data_iptr(rhs.m_data_iptr)
    , m_data(rhs.m_data)
  {
    CounterData i_data = *m_data_iptr;
    if (m_data.capture_platform_active == RAJA::Platform::undefined &&
        i_data.capture_platform_active != RAJA::Platform::undefined) {
      m_data = i_data;
    }
  }

  PluginTestCallable(PluginTestCallable && rhs)
    : m_data_optr(rhs.m_data_optr)
    , m_data_iptr(rhs.m_data_iptr)
    , m_data(rhs.m_data)
  {
    rhs.clear();
  }

  PluginTestCallable& operator=(PluginTestCallable const&) = default;
  PluginTestCallable& operator=(PluginTestCallable && rhs)
  {
    if (this != &rhs) {
      m_data_optr = rhs.m_data_optr;
      m_data_iptr = rhs.m_data_iptr;
      m_data      = rhs.m_data;

      rhs.clear();
    }

    return *this;
  }

  void operator()(int i) const
  {
    m_data_optr[i].capture_platform_active = m_data.capture_platform_active;
    m_data_optr[i].capture_counter_pre     = m_data.capture_counter_pre;
    m_data_optr[i].capture_counter_post    = m_data.capture_counter_post;
    m_data_optr[i].launch_platform_active = m_data_iptr->launch_platform_active;
    m_data_optr[i].launch_counter_pre     = m_data_iptr->launch_counter_pre;
    m_data_optr[i].launch_counter_post    = m_data_iptr->launch_counter_post;
  }

  void operator()(int count, int i) const
  {
    RAJA_UNUSED_VAR(count);
    operator()(i);
  }

private:
        CounterData* m_data_optr = nullptr;
  const CounterData* m_data_iptr = nullptr;
        CounterData  m_data;


  void clear()
  {
    m_data_optr = nullptr;
    m_data_iptr = nullptr;
    clear_data();
  }

  void clear_data()
  {
    m_data.capture_platform_active = RAJA::Platform::undefined;
    m_data.capture_counter_pre     = -1;
    m_data.capture_counter_post    = -1;
    m_data.launch_platform_active = RAJA::Platform::undefined;
    m_data.launch_counter_pre     = -1;
    m_data.launch_counter_post    = -1;
  }
};


// Check that the plugin is called with the right Platform.
// Check that the plugin is called the correct number of times,
// once before and after each kernel capture for the capture counter,
// once before and after each kernel invocation for the launch counter.

// test with basic forall
TEST(PluginTest, ForAllCounter)
{
  SetupPluginVars spv;

  CounterData* data = new CounterData[10];

  for (int i = 0; i < 10; i++) {

    RAJA::forall<RAJA::seq_exec>(
      RAJA::RangeSegment(i,i+1),
      PluginTestCallable{data}
    );

    ASSERT_EQ(data[i].capture_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].capture_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].capture_counter_post,    i                   );
    ASSERT_EQ(data[i].launch_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].launch_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].launch_counter_post,    i                   );
  }

  ASSERT_EQ(plugin_test_data->capture_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->capture_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->capture_counter_post,    10);
  ASSERT_EQ(plugin_test_data->launch_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->launch_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->launch_counter_post,    10);

  delete[] data;
}

// test with basic forall_Icount
TEST(PluginTest, ForAllICountCounter)
{
  SetupPluginVars spv;

  CounterData* data = new CounterData[10];

  for (int i = 0; i < 10; i++) {

    RAJA::forall_Icount<RAJA::seq_exec>(
      RAJA::RangeSegment(i,i+1), i,
      PluginTestCallable{data}
    );

    ASSERT_EQ(data[i].capture_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].capture_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].capture_counter_post,    i                   );
    ASSERT_EQ(data[i].launch_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].launch_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].launch_counter_post,    i                   );
  }

  ASSERT_EQ(plugin_test_data->capture_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->capture_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->capture_counter_post,    10);
  ASSERT_EQ(plugin_test_data->launch_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->launch_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->launch_counter_post,    10);

  delete[] data;
}

// test with IndexSet forall
TEST(PluginTest, ForAllIdxSetCounter)
{
  SetupPluginVars spv;

  CounterData* data = new CounterData[10];

  for (int i = 0; i < 10; i++) {

    RAJA::TypedIndexSet< RAJA::RangeSegment > iset;

    for (int j = i; j < 10; j++) {
      iset.push_back(RAJA::RangeSegment(j, j+1));
    }

    RAJA::forall<RAJA::ExecPolicy<RAJA::seq_segit, RAJA::seq_exec>>(
      iset,
      PluginTestCallable{data}
    );

    for (int j = i; j < 10; j++) {
      ASSERT_EQ(data[j].capture_platform_active, RAJA::Platform::host);
      ASSERT_EQ(data[j].capture_counter_pre,     i+1                 );
      ASSERT_EQ(data[j].capture_counter_post,    i                   );
      ASSERT_EQ(data[j].launch_platform_active, RAJA::Platform::host);
      ASSERT_EQ(data[j].launch_counter_pre,     i+1                 );
      ASSERT_EQ(data[j].launch_counter_post,    i                   );
    }
  }

  ASSERT_EQ(plugin_test_data->capture_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->capture_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->capture_counter_post,    10);
  ASSERT_EQ(plugin_test_data->launch_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->launch_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->launch_counter_post,    10);

  delete[] data;
}

// test with IndexSet forall_Icount
TEST(PluginTest, ForAllIcountIdxSetCounter)
{
  SetupPluginVars spv;

  CounterData* data = new CounterData[10];

  for (int i = 0; i < 10; i++) {

    RAJA::TypedIndexSet< RAJA::RangeSegment > iset;

    for (int j = i; j < 10; j++) {
      iset.push_back(RAJA::RangeSegment(j, j+1));
    }

    RAJA::forall_Icount<RAJA::ExecPolicy<RAJA::seq_segit, RAJA::seq_exec>>(
      iset,
      PluginTestCallable{data}
    );

    for (int j = i; j < 10; j++) {
      ASSERT_EQ(data[j].capture_platform_active, RAJA::Platform::host);
      ASSERT_EQ(data[j].capture_counter_pre,     i+1                 );
      ASSERT_EQ(data[j].capture_counter_post,    i                   );
      ASSERT_EQ(data[j].launch_platform_active, RAJA::Platform::host);
      ASSERT_EQ(data[j].launch_counter_pre,     i+1                 );
      ASSERT_EQ(data[j].launch_counter_post,    i                   );
    }
  }

  ASSERT_EQ(plugin_test_data->capture_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->capture_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->capture_counter_post,    10);
  ASSERT_EQ(plugin_test_data->launch_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->launch_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->launch_counter_post,    10);

  delete[] data;
}

// test with multi_policy forall
TEST(PluginTest, ForAllMultiPolicyCounter)
{
  SetupPluginVars spv;

  CounterData* data = new CounterData[10];

  auto mp = RAJA::make_multi_policy<RAJA::seq_exec, RAJA::loop_exec>(
      [](const RAJA::RangeSegment &r) {
        if (*(r.begin()) < 5) {
          return 0;
        } else {
          return 1;
        }
      });

  for (int i = 0; i < 5; i++) {

    RAJA::forall(mp,
      RAJA::RangeSegment(i,i+1),
      PluginTestCallable{data}
    );

    ASSERT_EQ(data[i].capture_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].capture_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].capture_counter_post,    i                   );
    ASSERT_EQ(data[i].launch_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].launch_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].launch_counter_post,    i                   );
  }

  ASSERT_EQ(plugin_test_data->capture_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->capture_counter_pre,     5);
  ASSERT_EQ(plugin_test_data->capture_counter_post,    5);
  ASSERT_EQ(plugin_test_data->launch_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->launch_counter_pre,     5);
  ASSERT_EQ(plugin_test_data->launch_counter_post,    5);

  for (int i = 5; i < 10; i++) {

    RAJA::forall(mp,
      RAJA::RangeSegment(i,i+1),
      PluginTestCallable{data}
    );

    ASSERT_EQ(data[i].capture_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].capture_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].capture_counter_post,    i                   );
    ASSERT_EQ(data[i].launch_platform_active, RAJA::Platform::host);
    ASSERT_EQ(data[i].launch_counter_pre,     i+1                 );
    ASSERT_EQ(data[i].launch_counter_post,    i                   );
  }

  ASSERT_EQ(plugin_test_data->capture_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->capture_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->capture_counter_post,    10);
  ASSERT_EQ(plugin_test_data->launch_platform_active, RAJA::Platform::undefined);
  ASSERT_EQ(plugin_test_data->launch_counter_pre,     10);
  ASSERT_EQ(plugin_test_data->launch_counter_post,    10);

  delete[] data;
}
