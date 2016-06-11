/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "geometry.hpp"

namespace ts
{
  namespace graphics
  {
    Geometry::Geometry()
    {
      GLuint buffer{};
      glCreateBuffers(1, &buffer);
      buffer_.reset(buffer);

      dummy_texture_ = Texture({ 1U, 1U });

      std::uint8_t data[] = { 255, 255, 255, 255 };
      dummy_texture_.update({ 0, 0 }, data, { 1U, 1U });
    }

    void Geometry::clear()
    {
      for (auto& component : components_)
      {
        component.vertices.clear();
        component.buffer_offset = 0;
      }
    }

    Geometry::component_hint 
      Geometry::add_vertices(const vertex_type* vertices, std::size_t vertex_count,
                                         const Texture* texture, component_hint hint)
    {
      auto component_ptr = static_cast<Component*>(hint);
      if (!component_ptr || component_ptr->texture != texture)
      {
        // Find the matching component. If there is none, create a new component with the required
        // properties.
        auto it = std::find_if(components_.begin(), components_.end(),
                               [=](const Component& component)
        {
          return component.texture == texture;
        });

        if (it != components_.end())
        {
          component_ptr = &*it;
        }

        else
        {
          Component component;
          component.texture = texture;
          component.buffer_offset = 0;

          components_.push_back(std::move(component));
          component_ptr = &components_.back();
        }
      }

      component_ptr->vertices.insert(component_ptr->vertices.end(), 
                                     vertices, vertices + vertex_count);

      dirty_ = true;
      return static_cast<component_hint>(component_ptr);
    }

    void Geometry::draw() const
    {
      if (dirty_)
      {
        glBindBuffer(GL_ARRAY_BUFFER, buffer_.get());

        auto total_vertex_count = std::accumulate(components_.begin(), components_.end(), std::size_t(0),
                                                  [](std::size_t acc, const Component& component)
        {
          return acc + component.vertices.size();
        });

        buffer_size_ = graphics::next_power_of_two(total_vertex_count * sizeof(vertex_type));
        glBufferData(GL_ARRAY_BUFFER, buffer_size_,
                     nullptr, GL_STATIC_DRAW);

        std::size_t offset = 0;
        for (auto& component : components_)
        {
          component.buffer_offset = offset;
          auto size = component.vertices.size() * sizeof(vertex_type);
          glBufferSubData(GL_ARRAY_BUFFER, offset, size, component.vertices.data());

          offset += size;
        }

        dirty_ = false;
      }

      glBindBuffer(GL_ARRAY_BUFFER, buffer_.get());

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_type),
                            reinterpret_cast<void*>(offsetof(vertex_type, position)));

      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_type),
                            reinterpret_cast<void*>(offsetof(vertex_type, texture_coords)));

      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_type),
                            reinterpret_cast<void*>(offsetof(vertex_type, color)));

      glActiveTexture(GL_TEXTURE0);
      for (const auto& component : components_)
      {
        if (component.vertices.empty()) continue;

        auto texture = component.texture;
        if (texture) glBindTexture(GL_TEXTURE_2D, texture->get());
        else glBindTexture(GL_TEXTURE_2D, dummy_texture_.get());

        glDrawArrays(GL_TRIANGLES, component.buffer_offset, component.vertices.size());
      }

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);
      glDisableVertexAttribArray(2);
    }
  }
}