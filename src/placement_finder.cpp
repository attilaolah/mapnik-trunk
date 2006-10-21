/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 * Copyright (C) 2006 10East Corp.
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

//$Id$

//stl
#include <string>
#include <vector>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/mutex.hpp>

//mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/text_path.hpp>

namespace mapnik
{
  placement::placement(string_info *info_, CoordTransform *ctrans_, const proj_transform *proj_trans_, geometry_ptr geom_, std::pair<double, double> dimensions_)
    : info(info_), ctrans(ctrans_), proj_trans(proj_trans_), geom(geom_), label_placement(point_placement), dimensions(dimensions_), has_dimensions(true), shape_path(*ctrans_, *geom_, *proj_trans_), total_distance_(-1.0)
  {
  }
  
  //For text
  placement::placement(string_info *info_, CoordTransform *ctrans_, const proj_transform *proj_trans_, geometry_ptr geom_, label_placement_e placement_)
    : info(info_), ctrans(ctrans_), proj_trans(proj_trans_), geom(geom_), label_placement(placement_), has_dimensions(false), shape_path(*ctrans_, *geom_, *proj_trans_), total_distance_(-1.0)
  {
  }
  
  placement::~placement()
  {
  }

  std::pair<double, double> placement::get_position_at_distance(double target_distance)
  {
    double old_x, old_y, new_x, new_y;
    double x, y;
    x = y = 0.0;
    
    double distance = 0.0;
    
    shape_path.rewind(0);
    shape_path.vertex(&new_x,&new_y);
    for (unsigned i = 0; i < geom->num_points() - 1; i++)
    {
      double dx, dy;

      old_x = new_x;
      old_y = new_y;

      shape_path.vertex(&new_x,&new_y);
      
      dx = new_x - old_x;
      dy = new_y - old_y;
      
      double segment_length = sqrt(dx*dx + dy*dy);
      
      distance += segment_length;
      if (distance > target_distance)
      {
          x = new_x - dx*(distance - target_distance)/segment_length;
          y = new_y - dy*(distance - target_distance)/segment_length;

          break;
      }
    }
    
    return std::pair<double, double>(x, y);
  }
  
  double placement::get_total_distance()
  {
    if (total_distance_ < 0.0)
    {
      double old_x, old_y, new_x, new_y;
      
      shape_path.rewind(0);
     
      shape_path.vertex(&old_x,&old_y);

      total_distance_ = 0.0;
      
      for (unsigned i = 0; i < geom->num_points() - 1; i++)
      {
          double dx, dy;
          
          shape_path.vertex(&new_x,&new_y);
          
          dx = new_x - old_x;
          dy = new_y - old_y;
          
          total_distance_ += sqrt(dx*dx + dy*dy);
          
          old_x = new_x;
          old_y = new_y;
      }
    }
    
    return total_distance_;
  }
  
  void placement::clear_envelopes()
  {
    while (!envelopes.empty())
      envelopes.pop();
  }
  
  
  
  placement_finder::placement_finder(Envelope<double> e)
    : detector_(e)
  {
  }

  bool placement_finder::find_placement(placement *p)
  {
    if (p->label_placement == point_placement)
    {
      return find_placement_horizontal(p);
    }
    else if (p->label_placement == line_placement)
    {
      return find_placement_follow(p);
    }
    
    return false;
  }

  bool placement_finder::find_placement_follow(placement *p)
  {
    std::pair<double, double> string_dimensions = p->info->get_dimensions();
    double string_width = string_dimensions.first;
//    double string_height = string_dimensions.second;
    
    double distance = p->get_total_distance();
    
    //~ double delta = string_width/distance;
    double delta = distance/100.0;
    
    for (double i = 0; i < (distance - string_width)/2.0; i += delta)
    {
      p->clear_envelopes();
      
      if ( build_path_follow(p, (distance - string_width)/2.0 + i) ) {
        update_detector(p);
        return true;
      }
      
      p->clear_envelopes();
      
      if ( build_path_follow(p, (distance - string_width)/2.0 - i) ) {
        update_detector(p);
        return true;
      }
    }
    
    p->starting_x = 0;
    p->starting_y = 0;
    
    return false;
  }
  
  bool placement_finder::find_placement_horizontal(placement *p)
  {
    double distance = p->get_total_distance();
    //~ double delta = string_width/distance;
    double delta = distance/100.0;
    
    for (double i = 0; i < distance/2.0; i += delta)
    {
      p->clear_envelopes();
      
      if ( build_path_horizontal(p, distance/2.0 + i) ) {
        update_detector(p);
        return true;
      }
      
      p->clear_envelopes();
      
      if ( build_path_horizontal(p, distance/2.0 - i) ) {
        update_detector(p);
        return true;
      }
    }
    
    p->starting_x = 0;
    p->starting_y = 0;
    
    return false;
  }
  
  void placement_finder::update_detector(placement *p)
  {
    while (!p->envelopes.empty())
    {
      Envelope<double> e = p->envelopes.front();

      detector_.insert(e);

      p->envelopes.pop();
    }
  }

  bool placement_finder::build_path_follow(placement *p, double target_distance)
  {
    double new_x, new_y, old_x, old_y;
    unsigned cur_node = 0;

    double angle = 0.0;
    int orientation = 0;
    
    p->path.clear();
    
    double x, y;
    x = y = 0.0;
    
    double distance = 0.0;

    std::pair<double, double> string_dimensions = p->info->get_dimensions();
//    double string_width = string_dimensions.first;
    double string_height = string_dimensions.second;
    
    p->shape_path.rewind(0);
    p->shape_path.vertex(&new_x,&new_y);
    for (unsigned i = 0; i < p->geom->num_points() - 1; i++)
    {
        double dx, dy;

        cur_node++;
        
        old_x = new_x;
        old_y = new_y;

        p->shape_path.vertex(&new_x,&new_y);
        
        dx = new_x - old_x;
        dy = new_y - old_y;
        
        double segment_length = sqrt(dx*dx + dy*dy);
        
        distance += segment_length;
        if (distance > target_distance)
        {
            p->starting_x = new_x - dx*(distance - target_distance)/segment_length;
            p->starting_y = new_y - dy*(distance - target_distance)/segment_length;

            angle = atan2(-dy, dx);

            if (angle > M_PI/2 || angle <= -M_PI/2) {
              orientation = -1;
            }
            else {
              orientation = 1;
            }

            distance -= target_distance;
            
            break;
        }
    }

    for (unsigned i = 0; i < p->info->num_characters(); i++)
    {
        character_info ci;
        unsigned c;
      
        while (distance <= 0) {
            double dx, dy;

            cur_node++;
            
            if (cur_node >= p->geom->num_points()) {
              break;
            }
            
            old_x = new_x;
            old_y = new_y;

            p->shape_path.vertex(&new_x,&new_y);

            dx = new_x - old_x;
            dy = new_y - old_y;

            angle = atan2(-dy, dx );
            
            distance += sqrt(dx*dx+dy*dy);
        }

        if (orientation == -1) {
            ci = p->info->at(p->info->num_characters() - i - 1);
        }
        else
        {
            ci = p->info->at(i);
        }
        c = ci.character;

        Envelope<double> e;
        if (p->has_dimensions)
        {
            e.init(x, y, x + p->dimensions.first, y + p->dimensions.second);
        }

        if (orientation == -1) {
            x = new_x - (distance - ci.width)*cos(angle);
            y = new_y + (distance - ci.width)*sin(angle);

            //Center the text on the line.
            x += (((double)string_height/2.0) - 1.0)*cos(angle+M_PI/2);
            y -= (((double)string_height/2.0) - 1.0)*sin(angle+M_PI/2);
          
            if (!p->has_dimensions)
            {
              e.init(x, y, x + ci.width*cos(angle+M_PI), y - ci.width*sin(angle+M_PI));
              e.expand_to_include(x - ci.height*sin(angle+M_PI), y - ci.height*cos(angle+M_PI));
              e.expand_to_include(x + (ci.width*cos(angle+M_PI) - ci.height*sin(angle+M_PI)), y - (ci.width*sin(angle+M_PI) + ci.height*cos(angle+M_PI)));
            }
        }
        else
        {
            x = new_x - distance*cos(angle);
            y = new_y + distance*sin(angle);

            //Center the text on the line.
            x += (((double)string_height/2.0) - 1.0)*cos(angle-M_PI/2);
            y -= (((double)string_height/2.0) - 1.0)*sin(angle-M_PI/2);

            if (!p->has_dimensions)
            {
              e.init(x, y, x + ci.width*cos(angle), y - ci.width*sin(angle));
              e.expand_to_include(x - ci.height*sin(angle), y - ci.height*cos(angle));
              e.expand_to_include(x + (ci.width*cos(angle) - ci.height*sin(angle)), y - (ci.width*sin(angle) + ci.height*cos(angle)));
            }
        }
        
        if (!detector_.has_placement(e))
        {
          return false;
        }
        
        p->envelopes.push(e);
        
        p->path.add_node(c, x - p->starting_x, -y + p->starting_y, (orientation == -1 ? angle + M_PI : angle));
        
        distance -= ci.width;
    }
    
    return true;
  }
  
  bool placement_finder::build_path_horizontal(placement *p, double target_distance)
  {
    double x, y;
  
    p->path.clear();
    
    std::pair<double, double> string_dimensions = p->info->get_dimensions();
    double string_width = string_dimensions.first;
    double string_height = string_dimensions.second;
    
    x = -string_width/2.0;
    y = -string_height/2.0 + 1.0;
    
    if (p->geom->type() == LineString)
    {
      std::pair<double, double> starting_pos = p->get_position_at_distance(target_distance);
      
      p->starting_x = starting_pos.first;
      p->starting_y = starting_pos.second;
    }
    else
    {
      p->geom->label_position(&p->starting_x, &p->starting_y);
      //  TODO: 
      //  We would only want label position in final 'paper' coords.
      //  Move view and proj transforms to e.g. label_position(x,y,proj_trans,ctrans)?
      double z=0;  
      p->proj_trans->backward(p->starting_x, p->starting_y, z);
      p->ctrans->forward(&p->starting_x, &p->starting_y);
    }
    
    for (unsigned i = 0; i < p->info->num_characters(); i++)
    {
        character_info ci;;
        ci = p->info->at(i);
        
        unsigned c = ci.character;
      
        p->path.add_node(c, x, y, 0.0);

        Envelope<double> e;
        if (p->has_dimensions)
        {
            e.init(p->starting_x - (p->dimensions.first/2.0), p->starting_y - (p->dimensions.second/2.0), p->starting_x + (p->dimensions.first/2.0), p->starting_y + (p->dimensions.second/2.0));
        }
        else
        {
          e.init(p->starting_x + x, p->starting_y - y, p->starting_x + x + ci.width, p->starting_y - y - ci.height);
        }
        
        if (!detector_.has_placement(e))
        {
          return false;
        }
        
        p->envelopes.push(e);
      
        x += ci.width;
    }
    return true;
  }

  
}

