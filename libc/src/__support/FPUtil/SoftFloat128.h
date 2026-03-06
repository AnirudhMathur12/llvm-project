#ifndef LLVM_LIBC_SRC___SUPPORT_FPUTIL_SOFTFLOAT128_H
#define LLVM_LIBC_SRC___SUPPORT_FPUTIL_SOFTFLOAT128_H

#include "hdr/stdint_proxy.h"
#include "src/__support/CPP/bit.h"
#include "src/__support/CPP/limits.h"
#include "src/__support/CPP/type_traits.h"
#include "src/__support/CPP/type_traits/always_false.h"
#include "src/__support/CPP/type_traits/enable_if.h"
#include "src/__support/CPP/type_traits/is_integral.h"
#include "src/__support/CPP/type_traits/is_signed.h"
#include "src/__support/CPP/type_traits/make_unsigned.h"
#include "src/__support/FPUtil/cast.h"
#include "src/__support/FPUtil/comparison_operations.h"
#include "src/__support/FPUtil/dyadic_float.h"
#include "src/__support/FPUtil/generic/add_sub.h"
#include "src/__support/FPUtil/generic/div.h"
#include "src/__support/FPUtil/generic/mul.h"
#include "src/__support/macros/config.h"
#include "src/__support/macros/properties/types.h"
#include "src/__support/uint128.h"

namespace LIBC_NAMESPACE_DECL {
namespace fputil {

struct SoftFloat128 {
  UInt128 bits;

  LIBC_INLINE SoftFloat128() = default;

  template <typename T>
  LIBC_INLINE constexpr explicit SoftFloat128(T value)
      : bits(static_cast<UInt128>(0)) {
    if constexpr (cpp::is_floating_point_v<T>) {
      bits = fputil::cast<SoftFloat128>(value).bits;
    } else if constexpr (cpp::is_integral_v<T>) {
      Sign sign = Sign::POS;

      if constexpr (cpp::is_signed_v<T>) {
        if (value < 0) {
          sign = Sign::NEG;
          value = -value;
        }
      }

      fputil::DyadicFloat<128> xd(sign, 0,
                                  static_cast<cpp::make_unsigned_t<T>>(value));

      bits =
          xd.template as<SoftFloat128, /*ShouldSignalExceptions=*/true>().bits;
    } else if constexpr (cpp::is_convertible_v<T, SoftFloat128>) {
      bits = value.operator SoftFloat128().bits;
    } else {
      static_assert(cpp::always_false<T>,
                    "SoftFloat128 cannot be contructed from this type");
    }
  }

  LIBC_INLINE constexpr operator double() const {
    return fputil::cast<double>(*this);
  }

  LIBC_INLINE constexpr operator float() const {
    return fputil::cast<float>(*this);
  }

  LIBC_INLINE constexpr operator long double() const {
    return fputil::cast<long double>(*this);
  }

  template <typename T, cpp::enable_if_t<cpp::is_integral_v<T>, int> = 0>
  LIBC_INLINE constexpr explicit operator T() const {
    return static_cast<T>(static_cast<double>(*this));
  }

  LIBC_INLINE bool operator==(SoftFloat128 other) const {
    return fputil::equals(*this, other);
  }

  LIBC_INLINE bool operator!=(SoftFloat128 other) const {
    return !fputil::equals(*this, other);
  }

  LIBC_INLINE bool operator<(SoftFloat128 other) const {
    return fputil::less_than(*this, other);
  }

  LIBC_INLINE bool operator<=(SoftFloat128 other) const {
    return fputil::less_than_or_equals(*this, other);
  }

  LIBC_INLINE bool operator>(SoftFloat128 other) const {
    return fputil::greater_than(*this, other);
  }

  LIBC_INLINE bool operator>=(SoftFloat128 other) const {
    return fputil::greater_than_or_equals(*this, other);
  }

  LIBC_INLINE constexpr SoftFloat128 operator-() const {
    fputil::FPBits<SoftFloat128> result(*this);
    result.set_sign(result.is_pos() ? Sign::NEG : Sign::POS);
    return result.get_val();
  }

  LIBC_INLINE SoftFloat128 operator+(SoftFloat128 other) const {
    return fputil::generic::add<SoftFloat128>(*this, other);
  }

  LIBC_INLINE SoftFloat128 operator-(SoftFloat128 other) const {
    return fputil::generic::sub<SoftFloat128>(*this, other);
  }

  LIBC_INLINE SoftFloat128 operator*(SoftFloat128 other) const {
    return fputil::generic::mul<SoftFloat128>(*this, other);
  }

  LIBC_INLINE SoftFloat128 operator/(SoftFloat128 other) const {
    return fputil::generic::div<SoftFloat128>(*this, other);
  }
}; // struct SoftFloat128

} // namespace fputil
} // namespace LIBC_NAMESPACE_DECL

#endif // LLVM_LIBC_SRC___SUPPORT_FPUTIL_SOFTFLOAT128_H
