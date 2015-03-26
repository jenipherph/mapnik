/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP

#include <mapnik/segment.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/vertex_adapters.hpp>
#include <mapnik/path.hpp>

#include <algorithm>
#include <deque>

namespace mapnik {

namespace detail {

template <typename F1, typename F2, typename F3>
void make_building(geometry::polygon const& poly, double height, F1 const& face_func, F2 const& frame_func, F3 const& roof_func)
{
    const std::unique_ptr<path_type> frame(new path_type(path_type::types::LineString));
    const std::unique_ptr<path_type> roof(new path_type(path_type::types::Polygon));
    std::deque<segment_t> face_segments;
    double x0 = 0;
    double y0 = 0;
    double x,y;
    geometry::polygon_vertex_adapter va(poly);
    va.rewind(0);
    for (unsigned cm = va.vertex(&x, &y); cm != SEG_END;
         cm = va.vertex(&x, &y))
    {
        if (cm == SEG_MOVETO)
        {
            frame->move_to(x,y);
        }
        else if (cm == SEG_LINETO)
        {
            frame->line_to(x,y);
            face_segments.push_back(segment_t(x0,y0,x,y));
        }
        else if (cm == SEG_CLOSE)
        {
            frame->close_path();
        }
        x0 = x;
        y0 = y;
    }

    std::sort(face_segments.begin(),face_segments.end(), y_order);
    for (auto const& seg : face_segments)
    {
        const std::unique_ptr<path_type> faces(new path_type(path_type::types::Polygon));
        faces->move_to(std::get<0>(seg),std::get<1>(seg));
        faces->line_to(std::get<2>(seg),std::get<3>(seg));
        faces->line_to(std::get<2>(seg),std::get<3>(seg) + height);
        faces->line_to(std::get<0>(seg),std::get<1>(seg) + height);

        face_func(*faces);
        //
        frame->move_to(std::get<0>(seg),std::get<1>(seg));
        frame->line_to(std::get<0>(seg),std::get<1>(seg)+height);
    }

    va.rewind(0);
    for (unsigned cm = va.vertex(&x, &y); cm != SEG_END;
         cm = va.vertex(&x, &y))
    {
        if (cm == SEG_MOVETO)
        {
            frame->move_to(x,y+height);
            roof->move_to(x,y+height);
        }
        else if (cm == SEG_LINETO)
        {
            frame->line_to(x,y+height);
            roof->line_to(x,y+height);
        }
        else if (cm == SEG_CLOSE)
        {
            frame->close_path();
            roof->close_path();
        }
    }

    frame_func(*frame);
    roof_func(*roof);
}

} // ns detail

template <typename F1, typename F2, typename F3>
void render_building_symbolizer(mapnik::feature_impl const& feature,
                                double height,
                                F1 face_func, F2 frame_func, F3 roof_func)
{

    auto const& geom = feature.get_geometry();
    if (geom.is<geometry::polygon>())
    {
        auto const& poly = geom.get<geometry::polygon>();
        detail::make_building(poly, height, face_func, frame_func, roof_func);
    }
    else if (geom.is<geometry::multi_polygon>())
    {
        auto const& multi_poly = geom.get<geometry::multi_polygon>();
        for (auto const& poly : multi_poly)
        {
            detail::make_building(poly, height, face_func, frame_func, roof_func);
        }
    }
}

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_BUILDING_SYMBOLIZER_HPP
