#include "src/__support/FPUtil/FPBits.h"
#include "src/__support/FPUtil/SoftFloat128.h"
#include "src/__support/uint128.h"
#include "src/math/fabsf128.h"
#include "test/UnitTest/LibcTest.h"

using LIBC_NAMESPACE::Sign;
using LIBC_NAMESPACE::fputil::FPBits;
using LIBC_NAMESPACE::fputil::SoftFloat128;

// Helper: construct SoftFloat128 from raw FPBits
static SoftFloat128 make(Sign s, int exp, UInt128 mantissa) {
  return FPBits<SoftFloat128>::create_value(s, exp, mantissa).get_val();
}

// ── Existing construction tests
// ───────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, BasicConstruction) {
  SoftFloat128 x;
  x.bits = UInt128(0);
  FPBits<SoftFloat128> bits(x);
  EXPECT_TRUE(bits.is_zero());
  EXPECT_TRUE(bits.is_pos());
}

TEST(LlvmLibcSoftFloat128Test, OneValue) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  Bits bits(one);
  EXPECT_FALSE(bits.is_zero());
  EXPECT_TRUE(bits.is_pos());
  EXPECT_EQ(bits.get_exponent(), 0);
}

TEST(LlvmLibcSoftFloat128Test, Infinity) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 inf = Bits::inf(Sign::POS).get_val();
  Bits bits(inf);
  EXPECT_TRUE(bits.is_inf());
  EXPECT_FALSE(bits.is_nan());
}

TEST(LlvmLibcSoftFloat128Test, NaN) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 qnan = Bits::quiet_nan().get_val();
  Bits bits(qnan);
  EXPECT_TRUE(bits.is_nan());
  EXPECT_TRUE(bits.is_quiet_nan());
}

// ── Negation
// ──────────────────────────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, NegationOfOne) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 neg_one = -one;
  Bits bits(neg_one);
  EXPECT_TRUE(bits.is_neg());
  EXPECT_EQ(bits.get_exponent(), 0);
}

TEST(LlvmLibcSoftFloat128Test, NegationOfZero) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 pzero = Bits::zero(Sign::POS).get_val();
  SoftFloat128 nzero = -pzero;
  Bits bits(nzero);
  EXPECT_TRUE(bits.is_zero());
  EXPECT_TRUE(bits.is_neg());
}

// ── Addition
// ──────────────────────────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, OnePlusOne) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 two = one + one;
  Bits bits(two);
  // 1.0 + 1.0 = 2.0 → exponent = 1, mantissa implicit leading bit only
  EXPECT_FALSE(bits.is_nan());
  EXPECT_FALSE(bits.is_inf());
  EXPECT_TRUE(bits.is_pos());
  EXPECT_EQ(bits.get_exponent(), 1);
}

TEST(LlvmLibcSoftFloat128Test, AdditionWithZero) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 zero = Bits::zero(Sign::POS).get_val();
  SoftFloat128 result = one + zero;
  Bits bits(result);
  EXPECT_EQ(bits.get_exponent(), 0);
  EXPECT_TRUE(bits.is_pos());
}

TEST(LlvmLibcSoftFloat128Test, AdditionProducesInfOnOverflow) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 max_normal = Bits::max_normal(Sign::POS).get_val();
  SoftFloat128 result = max_normal + max_normal;
  Bits bits(result);
  EXPECT_TRUE(bits.is_inf());
  EXPECT_TRUE(bits.is_pos());
}

TEST(LlvmLibcSoftFloat128Test, AddNaNPropagates) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 qnan = Bits::quiet_nan().get_val();
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 result = qnan + one;
  EXPECT_TRUE(FPBits<SoftFloat128>(result).is_nan());
}

// ── Subtraction
// ───────────────────────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, OneMinusOne) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 result = one - one;
  Bits bits(result);
  EXPECT_TRUE(bits.is_zero());
}

TEST(LlvmLibcSoftFloat128Test, SubtractionGivesNegative) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 two = one + one;
  SoftFloat128 result = one - two; // 1 - 2 = -1
  Bits bits(result);
  EXPECT_TRUE(bits.is_neg());
  EXPECT_EQ(bits.get_exponent(), 0);
}

// ── Multiplication
// ────────────────────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, OneTimesOne) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 result = one * one;
  Bits bits(result);
  EXPECT_FALSE(bits.is_nan());
  EXPECT_FALSE(bits.is_inf());
  EXPECT_EQ(bits.get_exponent(), 0);
  EXPECT_TRUE(bits.is_pos());
}

TEST(LlvmLibcSoftFloat128Test, MultiplyByZero) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 zero = Bits::zero(Sign::POS).get_val();
  SoftFloat128 result = one * zero;
  Bits bits(result);
  EXPECT_TRUE(bits.is_zero());
}

TEST(LlvmLibcSoftFloat128Test, MultiplySignNegTimesNeg) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 neg_one = Bits::one(Sign::NEG).get_val();
  SoftFloat128 result = neg_one * neg_one; // (-1) * (-1) = +1
  Bits bits(result);
  EXPECT_TRUE(bits.is_pos());
  EXPECT_EQ(bits.get_exponent(), 0);
}

TEST(LlvmLibcSoftFloat128Test, MultiplyInfByZeroIsNaN) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 inf = Bits::inf(Sign::POS).get_val();
  SoftFloat128 zero = Bits::zero(Sign::POS).get_val();
  SoftFloat128 result = inf * zero;
  EXPECT_TRUE(FPBits<SoftFloat128>(result).is_nan());
}

// ── Division
// ──────────────────────────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, OneDividedByOne) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 result = one / one;
  Bits bits(result);
  EXPECT_FALSE(bits.is_nan());
  EXPECT_FALSE(bits.is_inf());
  EXPECT_EQ(bits.get_exponent(), 0);
  EXPECT_TRUE(bits.is_pos());
}

TEST(LlvmLibcSoftFloat128Test, DivideByZeroIsInf) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 zero = Bits::zero(Sign::POS).get_val();
  SoftFloat128 result = one / zero;
  Bits bits(result);
  EXPECT_TRUE(bits.is_inf());
  EXPECT_TRUE(bits.is_pos());
}

TEST(LlvmLibcSoftFloat128Test, DivideZeroByOne) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 zero = Bits::zero(Sign::POS).get_val();
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 result = zero / one;
  EXPECT_TRUE(FPBits<SoftFloat128>(result).is_zero());
}

// ── Comparison operators
// ──────────────────────────────────────────────────────

TEST(LlvmLibcSoftFloat128Test, EqualityOfOnes) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 a = Bits::one(Sign::POS).get_val();
  SoftFloat128 b = Bits::one(Sign::POS).get_val();
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(LlvmLibcSoftFloat128Test, OrderingOnePosNeg) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 pos = Bits::one(Sign::POS).get_val();
  SoftFloat128 neg = Bits::one(Sign::NEG).get_val();
  EXPECT_TRUE(neg < pos);
  EXPECT_TRUE(pos > neg);
  EXPECT_TRUE(neg <= pos);
  EXPECT_TRUE(pos >= neg);
}

TEST(LlvmLibcSoftFloat128Test, ConversionToDouble) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  double d = static_cast<double>(one);
  EXPECT_TRUE(d == 1.0);
}

TEST(LlvmLibcSoftFloat128Test, ConversionToFloat) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  float f = static_cast<float>(one);
  EXPECT_TRUE(f == 1.0f);
}

TEST(LlvmLibcSoftFloat128Test, ConversionToInt) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 one = Bits::one(Sign::POS).get_val();
  SoftFloat128 two = one + one;
  int i = static_cast<int>(two);
  EXPECT_EQ(i, 2);
}

TEST(LlvmLibcSoftFloat128Test, NegativeConversionToDouble) {
  using Bits = FPBits<SoftFloat128>;
  SoftFloat128 neg_one = Bits::one(Sign::NEG).get_val();
  double d = static_cast<double>(neg_one);
  EXPECT_TRUE(d == -1.0);
}
