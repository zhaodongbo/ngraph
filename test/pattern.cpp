// ----------------------------------------------------------------------------
// Copyright 2017 Nervana Systems Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// ----------------------------------------------------------------------------

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <memory>

#include "gtest/gtest.h"
#include "ngraph/log.hpp"
#include "ngraph/ngraph.hpp"
#include "ngraph/pass/graph_rewrite.hpp"
#include "ngraph/pass/manager.hpp"
#include "ngraph/pass/topological_sort.hpp"
#include "ngraph/pattern/matcher.hpp"
#include "ngraph/pattern/op/any.hpp"
#include "ngraph/pattern/op/label.hpp"

using namespace ngraph;
using namespace std;

//this is for more nuanced testing
class TestMatcher : public pattern::Matcher
{
    using pattern::Matcher::Matcher;
    void virtual match_class(const std::shared_ptr<Node>& pattern_node,
                             const std::shared_ptr<Node>& graph_node) override
    {
        static const auto parameter_type = std::type_index(typeid(::ngraph::op::Parameter));
        const auto pattern_type = std::type_index(typeid(*&*pattern_node));

        if (pattern_type == parameter_type)
        {
            on_match_class(pattern_node,
                           graph_node,
                           pattern_node.get() ==
                               dynamic_cast<::ngraph::op::Parameter*>(graph_node.get()));
            return;
        }

        this->pattern::Matcher::match_class(pattern_node, graph_node);
    }
};

static std::shared_ptr<Node> construct_constant_node(int n)
{
    auto shape = Shape{1};
    auto t = ngraph::runtime::make_tensor<element::Int32>(shape);
    (*t) = std::vector<int>{n};
    return std::make_shared<op::ParameterizedConstant<element::Int32>>(shape, t);
}

class TestGraphRewrite : public ngraph::pass::GraphRewrite
{
public:
    void construct_multiply_by_one()
    {
        //pattern #1 : a * 1 = a
        auto iconst1 = construct_constant_node(1);
		auto pattern = pattern::op::Label::make_from_node(iconst1);

        NGRAPH_DEBUG << "IN TestGraphRewrite";

        ngraph::pattern::gr_callback_fn callback = [pattern](pattern::Matcher& m) {
            NGRAPH_DEBUG << "IN CALLBACK";
            assert(m.match_root()->get_arguments().size() == 2);

            size_t const_node_index =
                m.match_root()->get_arguments().at(0) == pattern->get_bound_node();
            auto const_node = dynamic_pointer_cast<op::ParameterizedConstant<element::Int32>>(
                m.match_root()->get_arguments().at(const_node_index));
            auto second_node = m.match_root()->get_arguments().at(const_node_index);
            NGRAPH_DEBUG << "second_node " << second_node->description() << " , " << second_node;
            NGRAPH_DEBUG << "pattern " << pattern->get_bound_node()->description() << " , "
                         << pattern->get_bound_node();
            assert(const_node);

            auto pattern_value_type = dynamic_pointer_cast<const TensorViewType>(
                pattern->get_bound_node()->get_value_type());
            auto const_node_value_type =
                dynamic_pointer_cast<const TensorViewType>(const_node->get_value_type());
            assert(pattern_value_type && const_node);

            if (pattern_value_type->get_element_type() !=
                    const_node_value_type->get_element_type() ||
                pattern_value_type->get_shape() != const_node_value_type->get_shape())
            {
                NGRAPH_DEBUG << "TYPE/SHAPE";
                return;
            }

            auto const_values = const_node->get_value()->get_vector();
            bool all_ones =
                std::all_of(begin(const_values), end(const_values), [](int e) { return e == 1; });

            if (!all_ones)
            {
                NGRAPH_DEBUG << "ALL_ONES";
                return;
            }

            NGRAPH_DEBUG << "BEFORE REPLACE";
            ngraph::pass::GraphRewrite::replace_node(m.match_root(), pattern->get_bound_node());
        };

        auto m = make_shared<TestMatcher>(pattern * iconst1, callback);
        this->add_matcher(m);
    }

    void construct_add_zero()
    {
        //pattern #2 : a + 0 = a
        auto iconst0 = construct_constant_node(0);
		auto pattern = pattern::op::Label::make_from_node(iconst0);

        NGRAPH_DEBUG << "IN TestGraphRewrite";

        ngraph::pattern::gr_callback_fn callback = [pattern](pattern::Matcher& m) {
            NGRAPH_DEBUG << "IN CALLBACK";
            assert(m.match_root()->get_arguments().size() == 2);

            size_t const_node_index =
                m.match_root()->get_arguments().at(0) == pattern->get_bound_node();
            auto const_node = dynamic_pointer_cast<op::ParameterizedConstant<element::Int32>>(
                m.match_root()->get_arguments().at(const_node_index));
            auto second_node = m.match_root()->get_arguments().at(const_node_index);
            NGRAPH_DEBUG << "second_node " << second_node->description() << " , " << second_node;
            NGRAPH_DEBUG << "pattern " << pattern->get_bound_node()->description() << " , "
                         << pattern->get_bound_node();
            assert(const_node);

            auto pattern_value_type = dynamic_pointer_cast<const TensorViewType>(
                pattern->get_bound_node()->get_value_type());
            auto const_node_value_type =
                dynamic_pointer_cast<const TensorViewType>(const_node->get_value_type());
            assert(pattern_value_type && const_node);

            if (pattern_value_type->get_element_type() !=
                    const_node_value_type->get_element_type() ||
                pattern_value_type->get_shape() != const_node_value_type->get_shape())
            {
                NGRAPH_DEBUG << "TYPE/SHAPE";
                return;
            }

            auto const_values = const_node->get_value()->get_vector();
            bool all_zeros =
                std::all_of(begin(const_values), end(const_values), [](int e) { return e == 0; });

            if (!all_zeros)
            {
                NGRAPH_DEBUG << "ALL_ZEROS";
                return;
            }

            NGRAPH_DEBUG << "BEFORE REPLACE";
            ngraph::pass::GraphRewrite::replace_node(m.match_root(), pattern->get_bound_node());
        };

        auto m = make_shared<TestMatcher>(pattern + iconst0, callback);
        this->add_matcher(m);
    }

    TestGraphRewrite()
        : GraphRewrite()
    {
        construct_multiply_by_one();
        construct_add_zero();
    }
};

static void run_passes(pass::Manager& pass_manager,
                       shared_ptr<Node> graph,
                       std::vector<shared_ptr<op::Parameter>> parms)
{
    auto shape = Shape{1};
    auto rt = make_shared<TensorViewType>(element::Int32::element_type(), shape);
    auto func = make_shared<Function>(graph, rt, op::Parameters{parms});
    pass_manager.run_passes(func);
}

TEST(pattern, graph_rewrite)
{
    auto shape = Shape{1};
    pass::Manager pass_manager;

    pass_manager.register_pass<pass::TopologicalSort>();
    pass_manager.register_pass<TestGraphRewrite>();

    {
        auto a = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto b = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto iconst0 = construct_constant_node(0);
        auto graph = b + (a + iconst0);
        run_passes(pass_manager, graph, {a, b});
        ASSERT_EQ(graph->get_arguments().at(1), a);
		//ASSERT_EQ(graph->get_inputs().at(1).get_output(), a->get_outputs().at(0));
    }

    {
        auto a = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto b = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto iconst1 = construct_constant_node(1);
        auto graph = b + (a * iconst1);
        run_passes(pass_manager, graph, {a, b});
        ASSERT_EQ(graph->get_arguments().at(1), a);
		//ASSERT_EQ(graph->get_inputs().at(1).get_output(), a->get_outputs().at(0));
    }

    {
        auto a = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto b = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto iconst1 = construct_constant_node(1);
        auto graph = ((((a * iconst1) * iconst1) * iconst1) * iconst1) + b;
        run_passes(pass_manager, graph, {a, b});
        ASSERT_EQ(graph->get_arguments().at(0), a);
		//ASSERT_EQ(graph->get_inputs().at(0).get_output(), a->get_outputs().at(0));
    }

    {
        auto a = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto b = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto iconst0 = construct_constant_node(0);
        auto iconst1 = construct_constant_node(1);
        auto graph = b + (iconst0 + ((a + iconst0) * iconst1));
        run_passes(pass_manager, graph, {a, b});
        ASSERT_EQ(graph->get_arguments().at(1), a);
		//ASSERT_EQ(graph->get_inputs().at(1).get_output(), a->get_outputs().at(0));
    }

    {
        auto a = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto b = make_shared<op::Parameter>(element::Int32::element_type(), shape);
        auto iconst1 = construct_constant_node(1);
        auto graph = b + (iconst1 * (iconst1 * (iconst1 * (iconst1 * a))));
        run_passes(pass_manager, graph, {a, b});
        ASSERT_EQ(graph->get_arguments().at(1), a);
		//ASSERT_EQ(graph->get_inputs().at(1).get_output(), a->get_outputs().at(0));
    }
}

TEST(pattern, matcher)
{
    auto shape = Shape{1};

    auto a = make_shared<op::Parameter>(element::Int32::element_type(), shape);
    TestMatcher n(nullptr);
    ASSERT_TRUE(n.match(a, a));

    auto abs = make_shared<op::Abs>(a);
    auto any = std::make_shared<pattern::op::Any>(a);
    ASSERT_TRUE(n.match(any, abs));

    auto any_false =
        std::make_shared<pattern::op::Any>(a, [](std::shared_ptr<Node> n) { return false; });
    ASSERT_TRUE(n.match(any_false, a));

    auto pattern = pattern::op::Label::make_from_node(a);
    ASSERT_TRUE(n.match(pattern, a));
    ASSERT_EQ(pattern->get_bound_node(), a);

    auto pattern_false =
		pattern::op::Label::make_from_node(a, [](std::shared_ptr<Node> n) { return false; });
    ASSERT_FALSE(n.match(pattern_false, a));

    auto b = make_shared<op::Parameter>(element::Int32::element_type(), shape);
    ASSERT_FALSE(n.match(a, b));
    ASSERT_FALSE(n.match(abs + b, b + b));
    ASSERT_TRUE(n.match(any + b, abs + b));

    ASSERT_TRUE(n.match(pattern + b, abs + b));
    ASSERT_EQ(pattern->get_bound_node(), abs);

    ASSERT_TRUE(n.match(b + pattern, abs + b));
    ASSERT_EQ(pattern->get_bound_node(), abs);

    auto c = make_shared<op::Parameter>(element::Int32::element_type(), shape);
    ASSERT_TRUE(n.match(c * (b + pattern), c * (abs + b)));
    ASSERT_EQ(pattern->get_bound_node(), abs);

    ASSERT_TRUE(n.match(c * (any + b), c * (abs + b)));     //nested any
    ASSERT_TRUE(n.match(c * (any + b), (b + abs) * c));     //permutations w/ any
    ASSERT_TRUE(n.match(c * (any_false + b), c * (a + b))); //nested any
    ASSERT_TRUE(n.match(c * (any_false + b), (b + a) * c)); //permutations w/ any_false

    auto t = ngraph::runtime::make_tensor<element::Int32>(shape);
    auto f = ngraph::runtime::make_tensor<element::Float32>(shape);
    (*t) = std::vector<int>{1};
    (*f) = std::vector<float>{1};
    auto iconst1_0 = make_shared<op::ParameterizedConstant<element::Int32>>(shape, t);
    auto iconst1_1 = make_shared<op::ParameterizedConstant<element::Int32>>(shape, t);
    ASSERT_TRUE(n.match(pattern * iconst1_0, a * iconst1_1)); //different iconst
    ASSERT_EQ(pattern->get_bound_node(), a);

    auto fconst1_0 = make_shared<op::ParameterizedConstant<element::Float32>>(shape, f);
    ASSERT_FALSE(n.match(pattern * fconst1_0, a * iconst1_1)); //different iconst
}