/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "resource_store.hpp"
#include "settings.hpp"
#include "car_store.hpp"

namespace ts
{
  namespace resources
  {
    struct ResourceStore::Impl
    {
      CarStore car_store;

      Settings settings;
    };

    ResourceStore::ResourceStore()
      : impl_(std::make_unique<Impl>())
    {
    }

    ResourceStore::~ResourceStore()
    {
    }

    CarStore& ResourceStore::car_store()
    {
      return impl_->car_store;
    }

    const CarStore& ResourceStore::car_store() const
    {
      return impl_->car_store;
    }

    Settings& ResourceStore::settings()
    {
      return impl_->settings;
    }

    const Settings& ResourceStore::settings() const
    {
      return impl_->settings;
    }
  }
}