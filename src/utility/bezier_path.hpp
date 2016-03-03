/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef BEZIER_PATH_HPP_859812983498
#define BEZIER_PATH_HPP_859812983498

#include "vector2.hpp"

#include <boost/iterator/iterator_facade.hpp>

#include <vector>
#include <type_traits>

namespace ts
{
  namespace utility
  {
    struct BezierCurve
    {
      Vector2<double> start_point, start_control, end_control, end_point;
    };

    class BezierPath;
    
    namespace detail
    {
      template <typename Point>
      struct BezierPathReference
      {
        using point_reference_ = Point&;
        point_reference_ start_point, start_control, end_control, end_point;

        operator BezierCurve() const
        {
          return{ start_point, start_control, end_control, end_point };
        }
      };

      template <typename From, typename To>
      using is_bezier_point_convertible = std::enable_if_t<
        !std::is_same<From, To>::value && std::is_convertible<From, To>::value
      >;

      template <typename Point>
      class BezierPathIterator
        : public boost::iterator_facade<BezierPathIterator<Point>, BezierCurve, 
                                        boost::random_access_traversal_tag, BezierPathReference<Point>>
      {
      public:
        BezierPathIterator() = default;

        template <typename OtherPoint, typename = is_bezier_point_convertible<OtherPoint, Point>>
        BezierPathIterator(const BezierPathIterator<OtherPoint>& other);

        using base_type = boost::iterator_facade<BezierPathIterator<Point>, BezierCurve,
          boost::random_access_traversal_tag, BezierPathReference<Point>>;

        using reference = typename base_type::reference;
        using difference_type = typename base_type::difference_type;
        using value_type = typename base_type::value_type;
        using iterator_category = typename base_type::iterator_category;

      private:
        friend boost::iterator_core_access;
        friend BezierPath;

        template <typename OtherPoint>
        friend class BezierPathIterator;        

        explicit BezierPathIterator(Point* point);

        bool equal(const BezierPathIterator& other) const;

        void increment();
        void decrement();

        typename base_type::reference dereference() const;

        void advance(typename base_type::difference_type d);
        typename base_type::difference_type distance_to(const BezierPathIterator& other) const;

        Point* point_ = nullptr;
      };
    }

    class BezierPath
    {
    public:
      using point = Vector2<double>;
      using iterator = detail::BezierPathIterator<point>;
      using const_iterator = detail::BezierPathIterator<const point>;

      BezierPath() = default;

      template <typename InputIt>
      explicit BezierPath(InputIt it, InputIt end);      

      std::size_t size() const;
      bool empty() const;

      iterator insert(const_iterator pos, const BezierCurve& curve);

      template <typename InputIt>
      iterator insert(const_iterator pos, InputIt begin, InputIt end);

      void push_back(const BezierCurve& curve);
      void reserve(std::size_t new_cap);

      iterator begin();
      const_iterator begin() const;

      iterator end();
      const_iterator end() const;

    private:
      std::vector<point> points_;
    };

    // time_point in range [0.0-1.0]
    template <typename CurveType>
    Vector2<double> bezier_point_at(const CurveType& curve, double time_point);

    std::pair<BezierPath, BezierPath> generate_path_outline(const BezierPath& bezier_path, double width);
  }
}

#endif