/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <memory>

namespace ts
{
  namespace fonts
  {
    class FontLibrary;
  }

  namespace resources
  {
    class CarStore;
    class TrackStore;
    class Settings;

    // The ResourceStore class can be used as a global-ish object, and as the name implies,
    // it stores various resource libraries. Things like loaded cars, tracks and settings come to mind.
    class ResourceStore
    {
    public:
      ResourceStore();
      ~ResourceStore();

      const CarStore& car_store() const;
      CarStore& car_store();

      const TrackStore& track_store() const;
      TrackStore& track_store();

      Settings& settings();
      const Settings& settings() const;

      fonts::FontLibrary& font_library();
      const fonts::FontLibrary& font_library() const;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}
