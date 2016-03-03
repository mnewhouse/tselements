/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef RESOURCE_STORE_HPP_22559283
#define RESOURCE_STORE_HPP_22559283

#include <memory>

namespace ts
{
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

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif