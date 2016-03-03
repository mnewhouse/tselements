/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef VERTEX_INTERFACE_HPP_321589
#define VERTEX_INTERFACE_HPP_321589

#include <cstdint>
#include <vector>

namespace ts
{
  namespace utility
  {
    // This is a utility template that stores arrays of vertices in an efficient manner,
    // grouped by a texture handle and a level identifier. These groups are what we call "components",
    // and an interface is provided to examine and modify the list of components as needed.
    template <typename TextureType, typename VertexType>
    struct VertexInterface
    {
      using texture_type = TextureType;
      using vertex_type = VertexType;

      struct Component;
      struct ComponentInterface;

      using component_iterator = typename std::vector<Component>::const_iterator;

      template <typename VertexIt>
      component_iterator insert_vertices(component_iterator pos, TextureType texture,
                                VertexIt vertices, VertexIt vertices_end);

      template <typename VertexIt>
      component_iterator insert_vertices(component_iterator pos, std::size_t vertex_index, 
                                         TextureType texture, VertexIt vertices, VertexIt vertices_end);

      template <typename VertexIt>
      void append_vertices(TextureType texture, VertexIt vertices, VertexIt vertices_end, std::size_t level);

      void erase_vertices(component_iterator it);
      void erase_vertices(component_iterator it, component_iterator end);
      void erase_vertices(component_iterator it, std::size_t vertex_index, std::size_t vertex_count);

      ComponentInterface components() const;

    private:
      std::vector<VertexType> vertices_;
      std::vector<Component> components_;
    };

    template <typename TextureType, typename VertexType>
    struct VertexInterface<TextureType, VertexType>::Component
    {
      TextureType texture;
      std::size_t vertex_index;
      std::size_t vertex_count;
      std::size_t level;
    };

    template <typename TextureType, typename VertexType>
    struct VertexInterface<TextureType, VertexType>::ComponentInterface
    {
    public:
      struct TransformedComponent
      {
        const TextureType texture;
        const VertexType* vertices;
        std::size_t vertex_count;
        std::size_t level;
      };

      struct Transformation
      {
        Transformation(const VertexType* vertices)
          : vertices_(vertices)
        {}

        TransformedComponent operator()(const Component& component) const;

      private:
        const VertexType* vertices_;
      };

      auto begin() const;
      auto end() const;

      bool empty() const;
      std::size_t size() const;

    private:
      friend VertexInterface<TextureType, VertexType>;

      explicit ComponentInterface(const std::vector<Component>* components,
                                  const std::vector<VertexType>* vertices);

      const std::vector<Component>* components_;
      const std::vector<VertexType>* vertices_;
    };

    template <typename TextureType, typename VertexType>
    template <typename VertexIt>
    void VertexInterface<TextureType, VertexType>::append_vertices(TextureType texture, VertexIt vertices, 
                                                                   VertexIt vertices_end, std::size_t level)
    {
      std::size_t old_size = vertices_.size();
      vertices_.insert(vertices_.end(), vertices, vertices_end);
      std::size_t new_size = vertices_.size();
      std::size_t vertex_count = new_size - old_size;

      try
      {
        // If there's an active component with the same texture handle *and* we're not forced to create
        // a new component...
        if (!components_.empty() && components_.back().texture == texture && components_.back().level == level)
        {
          auto& component = components_.back();
          component.vertex_count += vertex_count;
        }

        // Otherwise, just append a new component.
        else
        {
          Component component;
          component.texture = texture;
          component.vertex_index = old_size;
          component.vertex_count = vertex_count;
          component.level = level;
          components_.push_back(component);
        }
      }

      catch (...)
      {
        // Strong exception guarantee.
        vertices_.resize(old_size);
        throw;
      }
    }

    template <typename TextureType, typename VertexType>
    typename VertexInterface<TextureType, VertexType>::ComponentInterface 
      VertexInterface<TextureType, VertexType>::components() const
    {
      return ComponentInterface(&components_, &vertices_);
    }

    template <typename TextureType, typename VertexType>
    typename VertexInterface<TextureType, VertexType>::ComponentInterface::TransformedComponent
      VertexInterface<TextureType, VertexType>::ComponentInterface::Transformation::operator()(const Component& component) const
    {
      return{ component.texture, vertices_ + component.vertex_index, component.vertex_count, component.level };
    }
    
    template <typename TextureType, typename VertexType>
    VertexInterface<TextureType, VertexType>::ComponentInterface::ComponentInterface(const std::vector<Component>* components,
                                                                                     const std::vector<VertexType>* vertices)
      : components_(components),
        vertices_(vertices)
    {
    }

    template <typename TextureType, typename VertexType>
    auto VertexInterface<TextureType, VertexType>::ComponentInterface::begin() const
    {
      return boost::make_transform_iterator(components_->begin(), Transformation(vertices_->data()));
    }

    template <typename TextureType, typename VertexType>
    auto VertexInterface<TextureType, VertexType>::ComponentInterface::end() const
    {
      return boost::make_transform_iterator(components_->end(), Transformation(vertices_->data()));
    }

    template <typename TextureType, typename VertexType>
    bool VertexInterface<TextureType, VertexType>::ComponentInterface::empty() const
    {
      return components_->empty();
    }

    template <typename TextureType, typename VertexType>
    std::size_t VertexInterface<TextureType, VertexType>::ComponentInterface::size() const
    {
      return components_->size();
    }
  }
}

#endif