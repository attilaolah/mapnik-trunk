/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef TEXT_SYMBOLIZER_HPP
#define TEXT_SYMBOLIZER_HPP
// stl
#include <string>
// boost
#include <boost/tuple/tuple.hpp>
// mapnik
#include <mapnik/color.hpp>

namespace mapnik
{
    enum label_placement_e {
        point_placement=1,
        line_placement=2
    };
        
    typedef boost::tuple<double,double> position;
    
    struct MAPNIK_DECL text_symbolizer
    {		
        text_symbolizer(std::string const& name,std::string const& face_name, 
                        unsigned size,Color const& fill);	
        text_symbolizer(text_symbolizer const& rhs);
        text_symbolizer& operator=(text_symbolizer const& rhs);
        std::string const& get_name() const;
        unsigned get_text_ratio() const; // target ratio for text bounding box in pixels
        void set_text_ratio(unsigned ratio);
        unsigned get_wrap_width() const; // target ratio for text bounding box in pixels
        void set_wrap_width(unsigned ratio);
        unsigned get_text_size() const;
        std::string const& get_face_name() const;
        Color const& get_fill() const;
        void set_halo_fill(Color const& fill);
        Color const& get_halo_fill() const;
        void set_halo_radius(unsigned radius);
        unsigned get_halo_radius() const;
        void set_label_placement(label_placement_e label_p);
        label_placement_e get_label_placement() const;
        void set_anchor(double x, double y);	
        position const& get_anchor() const;	
        void set_displacement(double x, double y);
        position const& get_displacement() const;
    private:
        std::string name_;
        std::string face_name_;
        unsigned size_;
        unsigned text_ratio_;
        unsigned wrap_width_;
        Color fill_;
        Color halo_fill_;
        unsigned halo_radius_;
        label_placement_e label_p_;
        position anchor_;
        position displacement_;
    };
}

#endif //TEXT_SYMBOLIZER_HPP
