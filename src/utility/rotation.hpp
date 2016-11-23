/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

namespace ts
{
  namespace rotation_units
  {
    static const struct radians_t{} radians;
    static const struct degrees_t{} degrees;
  }

  template <typename T>
  class Rotation
  {
  public:
    Rotation() = default;
    explicit Rotation(T radians);
    Rotation(T radians, rotation_units::radians_t);
    Rotation(T degrees, rotation_units::degrees_t);

    T radians() const;
    T degrees() const;

    void normalize();

    static constexpr T pi = static_cast<T>(3.14159265358979323846264338327950L);
    static constexpr T double_pi = static_cast<T>(3.14159265358979323846264338327950L * 2);
    static constexpr T inverse_pi = static_cast<T>(1 / 3.14159265358979323846264338327950L);
    static constexpr T inverse_180 = static_cast<T>(1 / 180.0L);

  private:
    T radians_ = T();
  };

  template <typename T>
  Rotation<T>::Rotation(T radians)
    : radians_(radians)
  {
  }

  template <typename T>
  Rotation<T>::Rotation(T radians, rotation_units::radians_t)
    : Rotation(radians)
  {
  }

  template <typename T>
  Rotation<T>::Rotation(T degrees, rotation_units::degrees_t)
    : radians_(degrees * inverse_180 * pi)
  {
  }

  template <typename T>
  T Rotation<T>::degrees() const
  {
    return radians_ * 180.0 * inverse_pi;
  }

  template <typename T>
  T Rotation<T>::radians() const
  {
    return radians_;
  }

  template <typename T>
  void Rotation<T>::normalize()
  {
    using std::fmod;

    if (radians_ > pi)
    {
      radians_ = fmod(radians_ + pi, double_pi) - pi;
    }

    else if (radians_ < -pi)
    {
      radians_ = fmod(radians_ - pi, double_pi) + pi;
    }
  }

  template <typename T>
  Rotation<T> normalize(Rotation<T> rotation)
  {
    rotation.normalize();
    return rotation;
  }

  template <typename To, typename From>
  Rotation<To> rotation_cast(const Rotation<From>& rotation)
  {
    return radians(static_cast<To>(rotation.radians()));
  }

  template <typename T>
  Rotation<T> radians(T radians)
  {
    return Rotation<T>(radians, rotation_units::radians);
  }

  template <typename T>
  Rotation<T> degrees(T degrees)
  {
    return Rotation<T>(degrees, rotation_units::degrees);
  }

  template <typename T>
  Rotation<T> operator+(const Rotation<T>& a, const Rotation<T>& b)
  {
    return radians(a.radians() + b.radians());
  }

  template <typename T>
  Rotation<T> operator-(const Rotation<T>& a, const Rotation<T>& b)
  {
    return radians(a.radians() - b.radians());
  }

  using Rotationf = Rotation<float>;
  using Rotationd = Rotation<double>;
}
