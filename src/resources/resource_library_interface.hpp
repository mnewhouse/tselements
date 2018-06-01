/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace resources
  {
    // The following class can be used to to expose resources in id-based resource libraries.
    // Examples include the texture and tile libraries.
    // The ContainerType must be a container that exposes an "iterator" member type.
    // The SearchPolicy must implement the following member functions:
    // * find(ContainerType, T)
    // * lower_bound(ContainerType, T)
    // * upper_bound(ContainerType, T)
    template <typename ContainerType, typename SearchPolicy>
    struct ResourceLibraryInterface
      : private SearchPolicy
    {
    public:
      explicit ResourceLibraryInterface(const ContainerType* container, SearchPolicy search_policy = {})
        : SearchPolicy(search_policy),
        container_(container)
      {}

      // Exposing the const_iterator type is not ideal but it's still quite alright. 
      using iterator = typename ContainerType::iterator;

      auto begin() const
      {
        return container_->begin();
      }

      auto end() const
      {
        return container_->end();
      }

      template <typename T>
      auto lower_bound(const T& k) const
      {
        return SearchPolicy::lower_bound(*container_, k);
      }

      template <typename T>
      auto upper_bound(const T& k) const
      {
        return SearchPolicy::upper_bound(*container_, k);
      }

      template <typename T>
      auto equal_range(const T& k) const
      {
        return SearchPolicy::equal_range(*container_, k);
      }

      template <typename T>
      auto find(const T& k) const
      {
        return SearchPolicy::find(*container_, k);
      }

      std::size_t size() const
      {
        return container_->size();
      }

    private:
      const ContainerType* container_;
    };

    struct SetSearchPolicy
    {
      template <typename ContainerType, typename T>
      auto lower_bound(const ContainerType& container, const T& value) const
      {
        return container.lower_bound(value);
      }

      template <typename ContainerType, typename T>
      auto upper_bound(const ContainerType& container, const T& value) const
      {
        return container.lower_bound(value);
      }

      template <typename ContainerType, typename T>
      auto equal_range(const ContainerType& container, const T& value) const
      {
        return container.equal_range(value);
      }

      template <typename ContainerType, typename T>
      auto find(const ContainerType& container, const T& value) const
      {
        return container.find(value);
      }
    };
  }
}
