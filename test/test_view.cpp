/***************************************************************************
* Copyright (c) Johan Mabille, Sylvain Corlay and Wolf Vollprecht          *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include "gtest/gtest.h"
#include "zarray/zarray.hpp"
#include "xtensor-io/xchunk_store_manager.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "xtensor-io/xio_disk_handler.hpp"
#include "xtensor/xview.hpp"

namespace xt
{
    namespace fs = ghc::filesystem;

    TEST(view, strided_view)
    {
        xarray<double> a = {{1., 2.}, {3., 4.}};
        xstrided_slice_vector sv1({xt::all(), 1});
        xstrided_slice_vector sv2({1});
        xarray<double> expected1 = xt::strided_view(a, sv1);
        xarray<double> expected2 = xt::strided_view(expected1, sv2);

        zarray za(a);

        zarray zres1 = strided_view(za, sv1);
        zarray zres2 = strided_view(zres1, sv2);

        EXPECT_EQ(zres1.get_array<double>(), expected1);
        EXPECT_EQ(zres2.get_array<double>(), expected2);
    }

    TEST(view, chunked_strided_view)
    {
        std::vector<size_t> shape = {4, 4};
        std::vector<size_t> chunk_shape = {2, 2};
        std::string chunk_dir = "chunk_dir";
        fs::create_directory(chunk_dir);
        double init_value = 123.456;
        auto ca = chunked_file_array<double, xio_disk_handler<xio_binary_config>>(shape, chunk_shape, chunk_dir, init_value);
        xarray<double> a = {{1., 2.}, {3., 4.}};
        view(ca, range(0, 2), range(0, 2)) = a;
        xt::xstrided_slice_vector sv({xt::all(), 1});
        xarray<double> expected = {2, 4, init_value, init_value};

        zarray za(ca);

        zarray zres = strided_view(za, sv);

        EXPECT_EQ(zres.get_array<double>(), expected);
    }
}
