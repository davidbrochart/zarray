/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_ZASSIGN_HPP
#define XTENSOR_ZASSIGN_HPP

#include "xtensor/xassign.hpp"
#include "zarray_impl.hpp"

namespace xt
{
    namespace detail
    {
        template <class Tag>
        struct zexpression_assigner
        {
            // Both E1 and E2 are zarray expressions
            template <class E1, class E2>
            static void assign_data(E1& e1, const E2& e2)
            {
                e2.assign_to(e1.get_implementation());
            }
        };

        template <>
        struct zexpression_assigner<xtensor_expression_tag>
        {
            // E1 is a zarray_expression, E2 is an xtensor_expression
            template <class E1, class E2>
            static void assign_data(E1& e1, const E2& e2)
            {
                using value_type = typename E2::value_type;
                e1.template get_array<value_type>() = e2;
            }
        };
    }

    template <>
    class xexpression_assigner<zarray_expression_tag>
    {
    public:

        template <class E1, class E2>
        static void assign_xexpression(xexpression<E1>& e1, const xexpression<E2>& e2)
        {
            E1& lhs = e1.derived_cast();
            const E2& rhs = e2.derived_cast();
            std::size_t size = rhs.dimension();
            auto shape = uninitialized_shape<dynamic_shape<std::size_t>>(size);
            rhs.broadcast_shape(shape, true);
            lhs.resize(std::move(shape));
            detail::zexpression_assigner<xexpression_tag_t<E2>>::assign_data(lhs, rhs);
        }
    };

    /******************************
     * zassign_wrapped_expression *
     ******************************/

    namespace detail
    {
        // The shape of lhs (and thus whether the assign is linear)
        // is computed before the dynamic dispatch, thus the information
        // is lost. However, since xtensor wrapped expressions are precomputed
        // in xarray objects with default layout, comparing the shapes of the
        // different operands of an expression is enough to decide whether
        // the assign should be linear or not.
        template <class LHS, class RHS>
        struct xis_linear_assign
        {
            static bool run(const LHS&, const RHS&)
            {
                return false;
            }
        };

        template <class LHS, class T>
        struct xis_linear_assign<LHS, xarray<T>>
        {
            static bool run(const LHS& lhs, const xarray<T>& rhs)
            {
                return lhs.shape() == rhs.shape();
            }
        };

        template <class LHS, class F, class... CT>
        struct xis_linear_assign<LHS, xfunction<F, CT...>>
        {
            static bool run(const LHS& lhs, const xfunction<F, CT...>& rhs)
            {
                const auto& shape = lhs.shape();
                auto f = [&shape](bool b, const auto& e) { return b && shape == e.shape(); };
                return accumulate(f, true, rhs.arguments());
            }
        };
    }

    template <class E1, class E2>
    inline void zassign_wrapped_expression(xexpression<E1>& e1, const xexpression<E2>& e2)
    {
        bool linear_assign = detail::xis_linear_assign<E1, E2>::run(e1.derived_cast(), e2.derived_cast());
        assign_data(e1, e2, linear_assign);
    }
}

#endif
