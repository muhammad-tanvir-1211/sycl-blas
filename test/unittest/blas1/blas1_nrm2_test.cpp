/***************************************************************************
 *
 *  @license
 *  Nrm2right (C) Codeplay Software Limited
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a nrm2 of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  For your convenience, a nrm2 of the License has been included in this
 *  repository.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  SYCL-BLAS: BLAS implementation using SYCL
 *
 *  @filename blas1_nrm2_test.cpp
 *
 **************************************************************************/

#include "blas_test.hpp"

template <typename scalar_t>
using combination_t = std::tuple<int, int>;

template <typename scalar_t>
void run_test(const combination_t<scalar_t> combi) {
  index_t size;
  index_t incX;
  std::tie(size, incX) = combi;

  using data_t = utils::data_storage_t<scalar_t>;

  // Input vectors
  std::vector<data_t> x_v(size * incX);
  fill_random(x_v);

  // Output vector
  std::vector<data_t> out_s(1, 10.0);

  // Reference implementation
  auto out_cpu_s = reference_blas::nrm2(size, x_v.data(), incX);

  // SYCL implementation
  auto q = make_queue();
  test_executor_t ex(q);

  // Iterators
  auto gpu_x_v = utils::make_quantized_buffer<scalar_t>(ex, x_v);
  auto gpu_out_s = utils::make_quantized_buffer<scalar_t>(ex, out_s);

  _nrm2(ex, size, gpu_x_v, incX, gpu_out_s);
  auto event = utils::quantized_copy_to_host<scalar_t>(ex, gpu_out_s, out_s);
  ex.get_policy_handler().wait(event);

  // Validate the result
  const bool isAlmostEqual =
      utils::almost_equal<data_t, scalar_t>(out_s[0], out_cpu_s);
  ASSERT_TRUE(isAlmostEqual);
}

const auto combi = ::testing::Combine(::testing::Values(11, 1002),  // size
                                      ::testing::Values(1, 4)       // incX
);

template <class T>
static std::string generate_name(
    const ::testing::TestParamInfo<combination_t<T>>& info) {
  int size, incX;
  BLAS_GENERATE_NAME(info.param, size, incX);
}

BLAS_REGISTER_TEST_ALL(Nrm2, combination_t, combi, generate_name);
