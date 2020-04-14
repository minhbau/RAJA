/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   Header file for CUDA tiled executors.
 *
 ******************************************************************************
 */

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#ifndef RAJA_policy_cuda_kernel_TileTCount_HPP
#define RAJA_policy_cuda_kernel_TileTCount_HPP

#include "RAJA/config.hpp"

#if defined(RAJA_ENABLE_CUDA)

#include <iostream>
#include <type_traits>

#include "camp/camp.hpp"
#include "camp/concepts.hpp"
#include "camp/tuple.hpp"

#include "RAJA/util/macros.hpp"
#include "RAJA/util/types.hpp"

#include "RAJA/pattern/kernel/Tile.hpp"
#include "RAJA/pattern/kernel/internal.hpp"

namespace RAJA
{
namespace internal
{

/*!
 * A specialized RAJA::kernel cuda_impl executor for statement::TileTCount
 * Assigns the tile segment to segment ArgumentId
 * Assigns the tile index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          typename TPol,
          typename... EnclosedStmts,
          typename Types>
struct CudaStatementExecutor<
    Data,
    statement::TileTCount<ArgumentId, ParamId, TPol, seq_exec, EnclosedStmts...>, Types>
    : public CudaStatementExecutor<
        Data,
        statement::Tile<ArgumentId, TPol, seq_exec, EnclosedStmts...>, Types> {

  using Base = CudaStatementExecutor<
      Data,
      statement::Tile<ArgumentId, TPol, seq_exec, EnclosedStmts...>, Types>;

  using typename Base::enclosed_stmts_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active){
    // Get the segment referenced by this Tile statement
    auto &segment = camp::get<ArgumentId>(data.segment_tuple);

    // Keep copy of original segment, so we can restore it
    using segment_t = camp::decay<decltype(segment)>;
    segment_t orig_segment = segment;

    int chunk_size = TPol::chunk_size;

    // compute trip count
    int len = segment.end() - segment.begin();

    // Iterate through tiles
    for (int i = 0, t = 0; i < len; i += chunk_size, ++t) {

      // Assign our new tiled segment
      segment = orig_segment.slice(i, chunk_size);
      data.template assign_param<ParamId>(t);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active);
    }

    // Set range back to original values
    segment = orig_segment;
  }
};


/*!
 * A specialized RAJA::kernel cuda_impl executor for statement::TileTCount
 * Assigns the tile segment to segment ArgumentId
 * Assigns the tile index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          camp::idx_t chunk_size,
          int BlockDim,
          typename... EnclosedStmts,
          typename Types>
struct CudaStatementExecutor<
    Data,
    statement::TileTCount<ArgumentId, ParamId,
                    RAJA::tile_fixed<chunk_size>,
                    cuda_block_xyz_direct<BlockDim>,
                    EnclosedStmts...>,
                    Types>
    : public CudaStatementExecutor<
        Data,
        statement::Tile<ArgumentId,
                        RAJA::tile_fixed<chunk_size>,
                        cuda_block_xyz_direct<BlockDim>,
                        EnclosedStmts...>,
                        Types> {

  using Base = CudaStatementExecutor<
      Data,
      statement::Tile<ArgumentId,
                      RAJA::tile_fixed<chunk_size>,
                      cuda_block_xyz_direct<BlockDim>,
                      EnclosedStmts...>,
                      Types>;

  using typename Base::enclosed_stmts_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // Get the segment referenced by this Tile statement
    auto &segment = camp::get<ArgumentId>(data.segment_tuple);

    using segment_t = camp::decay<decltype(segment)>;

    // compute trip count
    int len = segment.end() - segment.begin();
    int t = get_cuda_dim<BlockDim>(blockIdx);
    int i = t * chunk_size;

    // Iterate through grid stride of chunks
    if (i < len) {

      // Keep copy of original segment, so we can restore it
      segment_t orig_segment = segment;

      // Assign our new tiled segment
      segment = orig_segment.slice(i, chunk_size);
      data.template assign_param<ParamId>(t);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active);

      // Set range back to original values
      segment = orig_segment;
    }
  }
};

/*!
 * A specialized RAJA::kernel cuda_impl executor for statement::TileTCount
 * Assigns the tile segment to segment ArgumentId
 * Assigns the tile index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          camp::idx_t chunk_size,
          int BlockDim,
          typename... EnclosedStmts,
          typename Types>
struct CudaStatementExecutor<
    Data,
    statement::TileTCount<ArgumentId, ParamId,
                    RAJA::tile_fixed<chunk_size>,
                    cuda_block_xyz_loop<BlockDim>,
                    EnclosedStmts...>,
                    Types>
    : public CudaStatementExecutor<
        Data,
        statement::Tile<ArgumentId,
                        RAJA::tile_fixed<chunk_size>,
                        cuda_block_xyz_loop<BlockDim>,
                        EnclosedStmts...>,
                        Types> {

  using Base = CudaStatementExecutor<
      Data,
      statement::Tile<ArgumentId,
                      RAJA::tile_fixed<chunk_size>,
                      cuda_block_xyz_loop<BlockDim>,
                      EnclosedStmts...>,
                      Types>;

  using typename Base::enclosed_stmts_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // Get the segment referenced by this Tile statement
    auto &segment = camp::get<ArgumentId>(data.segment_tuple);

    // Keep copy of original segment, so we can restore it
    using segment_t = camp::decay<decltype(segment)>;
    segment_t orig_segment = segment;

    // compute trip count
    int len = segment.end() - segment.begin();
    int t0 = get_cuda_dim<BlockDim>(blockIdx);
    int i0 = t0 * chunk_size;

    // Get our stride from the dimension
    int t_stride = get_cuda_dim<BlockDim>(gridDim);
    int i_stride = t_stride * chunk_size;

    // Iterate through grid stride of chunks
    for (int i = i0, t = t0; i < len; i += i_stride, t += t_stride) {

      // Assign our new tiled segment
      segment = orig_segment.slice(i, chunk_size);
      data.template assign_param<ParamId>(t);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active);
    }

    // Set range back to original values
    segment = orig_segment;
  }
};



/*!
 * A specialized RAJA::kernel cuda_impl executor for statement::TileTCount
 * Assigns the tile segment to segment ArgumentId
 * Assigns the tile index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          camp::idx_t chunk_size,
          int ThreadDim,
          typename ... EnclosedStmts,
          typename Types>
struct CudaStatementExecutor<
  Data,
  statement::TileTCount<ArgumentId, ParamId,
                        RAJA::tile_fixed<chunk_size>,
                        cuda_thread_xyz_direct<ThreadDim>,
                        EnclosedStmts ...>,
                        Types>
  : public CudaStatementExecutor<
    Data,
    statement::Tile<ArgumentId,
                    RAJA::tile_fixed<chunk_size>,
                    cuda_thread_xyz_direct<ThreadDim>,
                    EnclosedStmts ...>,
                    Types> {

  using Base = CudaStatementExecutor<
          Data,
          statement::Tile<ArgumentId,
                          RAJA::tile_fixed<chunk_size>,
                          cuda_thread_xyz_direct<ThreadDim>,
                          EnclosedStmts ...>,
                          Types>;

  using typename Base::enclosed_stmts_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // Get the segment referenced by this Tile statement
    auto &segment = camp::get<ArgumentId>(data.segment_tuple);

    // Keep copy of original segment, so we can restore it
    using segment_t = camp::decay<decltype(segment)>;
    segment_t orig_segment = segment;

    // compute trip count
    int len = segment.end() - segment.begin();
    int t = get_cuda_dim<ThreadDim>(threadIdx);
    int i = t * chunk_size;

    // execute enclosed statements if any thread will
    // but mask off threads without work
    bool have_work = i < len;

    // Assign our new tiled segment
    int slice_size = have_work ? chunk_size : 0;
    segment = orig_segment.slice(i, slice_size);
    data.template assign_param<ParamId>(t);

    // execute enclosed statements
    enclosed_stmts_t::exec(data, thread_active && have_work);

    // Set range back to original values
    segment = orig_segment;
  }
};


/*!
 * A specialized RAJA::kernel cuda_impl executor for statement::TileTCount
 * Assigns the tile segment to segment ArgumentId
 * Assigns the tile index to param ParamId
 */
template <typename Data,
          camp::idx_t ArgumentId,
          typename ParamId,
          camp::idx_t chunk_size,
          int ThreadDim,
          int MinThreads,
          typename ... EnclosedStmts>
struct CudaStatementExecutor<
  Data,
  statement::TileTCount<ArgumentId, ParamId,
                        RAJA::statement::tile_fixed<chunk_size>,
                        cuda_thread_xyz_loop<ThreadDim, MinThreads>,
                        EnclosedStmts ...> >
  : public CudaStatementExecutor<
    Data,
    statement::Tile<ArgumentId,
                    RAJA::statement::tile_fixed<chunk_size>,
                    cuda_thread_xyz_loop<ThreadDim, MinThreads>,
                    EnclosedStmts ...> > {

  using Base = CudaStatementExecutor<
          Data,
          statement::Tile<ArgumentId,
                          RAJA::statement::tile_fixed<chunk_size>,
                          cuda_thread_xyz_loop<ThreadDim, MinThreads>,
                          EnclosedStmts ...> >;

  using typename Base::enclosed_stmts_t;

  static
  inline
  RAJA_DEVICE
  void exec(Data &data, bool thread_active)
  {
    // Get the segment referenced by this Tile statement
    auto &segment = camp::get<ArgumentId>(data.segment_tuple);

    // Keep copy of original segment, so we can restore it
    using segment_t = camp::decay<decltype(segment)>;
    segment_t orig_segment = segment;

    // compute trip count
    int len = segment_length<ArgumentId>(data);
    int t0 = get_cuda_dim<ThreadDim>(threadIdx);
    int i0 = t0 * chunk_size;

    // Get our stride from the dimension
    int t_stride = get_cuda_dim<ThreadDim>(blockDim);
    int i_stride = t_stride * chunk_size;

    // Iterate through grid stride of chunks
    for(int ii = 0, t = t0; ii < len; ii += i_stride, t += t_stride) {
      int i = ii + i0;

      // execute enclosed statements if any thread will
      // but mask off threads without work
      bool have_work = i < len;

      // Assign our new tiled segment
      int slice_size = have_work ? chunk_size : 0;
      segment = orig_segment.slice(i, slice_size);
      data.template assign_param<ParamId>(t);

      // execute enclosed statements
      enclosed_stmts_t::exec(data, thread_active && have_work);
    }

    // Set range back to original values
    segment = orig_segment;
  }
};

}  // end namespace internal
}  // end namespace RAJA

#endif  // RAJA_ENABLE_CUDA
#endif  /* RAJA_pattern_kernel_HPP */
