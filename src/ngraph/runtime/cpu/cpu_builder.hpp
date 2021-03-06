/*******************************************************************************
* Copyright 2017-2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "ngraph/node.hpp"
#include "ngraph/runtime/cpu/cpu_external_function.hpp"
#include "ngraph/runtime/cpu/cpu_tensor_view_wrapper.hpp"

#define BUILDER_DECL(op_name)                                                                      \
    build<op_name>(CPU_ExternalFunction * external_function,                                       \
                   const ngraph::Node* node,                                                       \
                   const std::vector<TensorViewWrapper>& args,                                     \
                   const std::vector<TensorViewWrapper>& out)

// Per-type kernel macro
#define SELECT_KERNEL(KV, ET, K)                                                                   \
    if (ET == element::boolean)                                                                    \
    {                                                                                              \
        KV = K<char>;                                                                              \
    }                                                                                              \
    else if (ET == element::f32)                                                                   \
    {                                                                                              \
        KV = K<float>;                                                                             \
    }                                                                                              \
    else if (ET == element::f64)                                                                   \
    {                                                                                              \
        KV = K<double>;                                                                            \
    }                                                                                              \
    else if (ET == element::i8)                                                                    \
    {                                                                                              \
        KV = K<int8_t>;                                                                            \
    }                                                                                              \
    else if (ET == element::i16)                                                                   \
    {                                                                                              \
        KV = K<int16_t>;                                                                           \
    }                                                                                              \
    else if (ET == element::i32)                                                                   \
    {                                                                                              \
        KV = K<int32_t>;                                                                           \
    }                                                                                              \
    else if (ET == element::i64)                                                                   \
    {                                                                                              \
        KV = K<int64_t>;                                                                           \
    }                                                                                              \
    else if (ET == element::u8)                                                                    \
    {                                                                                              \
        KV = K<uint8_t>;                                                                           \
    }                                                                                              \
    else if (ET == element::u16)                                                                   \
    {                                                                                              \
        KV = K<uint16_t>;                                                                          \
    }                                                                                              \
    else if (ET == element::u32)                                                                   \
    {                                                                                              \
        KV = K<uint32_t>;                                                                          \
    }                                                                                              \
    else if (ET == element::u64)                                                                   \
    {                                                                                              \
        KV = K<uint64_t>;                                                                          \
    }

#define SELECT_RANK(KV, ET, R, K)                                                                  \
    if (R == 1)                                                                                    \
        KV = K<ET, 1>;                                                                             \
    else if (R == 2)                                                                               \
        KV = K<ET, 2>;                                                                             \
    else if (R == 3)                                                                               \
        KV = K<ET, 3>;                                                                             \
    else if (R == 4)                                                                               \
        KV = K<ET, 4>;                                                                             \
    else if (R == 5)                                                                               \
        KV = K<ET, 5>;                                                                             \
    else if (R == 6)                                                                               \
        KV = K<ET, 6>;                                                                             \
    else if (R == 7)                                                                               \
        KV = K<ET, 7>;                                                                             \
    else if (R == 8)                                                                               \
        KV = K<ET, 8>;                                                                             \
    else if (R == 9)                                                                               \
        KV = K<ET, 9>;                                                                             \
    else if (R == 10)                                                                              \
        KV = K<ET, 10>;                                                                            \
    else if (R == 11)                                                                              \
        KV = K<ET, 11>;                                                                            \
    else if (R == 12)                                                                              \
        KV = K<ET, 12>;                                                                            \
    else if (R == 13)                                                                              \
        KV = K<ET, 13>;                                                                            \
    else if (R == 14)                                                                              \
        KV = K<ET, 14>;                                                                            \
    else if (R == 15)                                                                              \
        KV = K<ET, 15>;                                                                            \
    else if (R == 16)                                                                              \
        KV = K<ET, 16>;

// Per-type and rank kernel macro
#define SELECT_KERNEL_BY_RANK(KV, ET, R, K)                                                        \
    if (ET == element::boolean)                                                                    \
    {                                                                                              \
        SELECT_RANK(KV, char, R, K);                                                               \
    }                                                                                              \
    else if (ET == element::f32)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, float, R, K);                                                              \
    }                                                                                              \
    else if (ET == element::f64)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, double, R, K);                                                             \
    }                                                                                              \
    else if (ET == element::i8)                                                                    \
    {                                                                                              \
        SELECT_RANK(KV, int8_t, R, K);                                                             \
    }                                                                                              \
    else if (ET == element::i16)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, int16_t, R, K);                                                            \
    }                                                                                              \
    else if (ET == element::i32)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, int32_t, R, K);                                                            \
    }                                                                                              \
    else if (ET == element::i64)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, int64_t, R, K);                                                            \
    }                                                                                              \
    else if (ET == element::u8)                                                                    \
    {                                                                                              \
        SELECT_RANK(KV, uint8_t, R, K);                                                            \
    }                                                                                              \
    else if (ET == element::u16)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, uint16_t, R, K);                                                           \
    }                                                                                              \
    else if (ET == element::u32)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, uint32_t, R, K);                                                           \
    }                                                                                              \
    else if (ET == element::u64)                                                                   \
    {                                                                                              \
        SELECT_RANK(KV, uint64_t, R, K);                                                           \
    }

namespace ngraph
{
    namespace runtime
    {
        namespace cpu
        {
            using BuildOpFunction =
                std::function<void(CPU_ExternalFunction* external_function,
                                   const ngraph::Node*,
                                   const std::vector<TensorViewWrapper>& inputs,
                                   const std::vector<TensorViewWrapper>& outputs)>;

            using BuildOpMap = std::unordered_map<std::type_index, BuildOpFunction>;

            extern const BuildOpMap build_dispatcher;

            class Builder
            {
            public:
                template <typename OP>
                static void build(CPU_ExternalFunction* external_function,
                                  const ngraph::Node* node,
                                  const std::vector<TensorViewWrapper>& args,
                                  const std::vector<TensorViewWrapper>& out)
                {
                    throw std::runtime_error("Unimplemented op in CPU builder");
                }

                static void nop(CPU_ExternalFunction* external_function,
                                const ngraph::Node* node,
                                const std::vector<TensorViewWrapper>& args,
                                const std::vector<TensorViewWrapper>& out)
                {
                }
            };
        }
    }
}
