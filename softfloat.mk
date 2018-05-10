softfloat_subproject_deps =

softfloat_hdrs = \
  internals.h \
  primitives.h \
  primitiveTypes.h \
  softfloat.h \
  softfloat_types.h \
  specialize.h \

softfloat_c_srcs = \
	f128_add.c \
	f128_classify.c \
	f128_div.c \
	f128_eq.c \
	f128_eq_signaling.c \
	f128_isSignalingNaN.c \
	f128_le.c \
	f128_le_quiet.c \
	f128_lt.c \
	f128_lt_quiet.c \
	f128_mulAdd.c \
	f128_mul.c \
	f128_rem.c \
	f128_roundToInt.c \
	f128_sqrt.c \
	f128_sub.c \
	f128_to_f16.c \
	f128_to_f32.c \
	f128_to_f64.c \
	f128_to_i32.c \
	f128_to_i32_r_minMag.c \
	f128_to_i64.c \
	f128_to_i64_r_minMag.c \
	f128_to_ui32.c \
	f128_to_ui32_r_minMag.c \
	f128_to_ui64.c \
	f128_to_ui64_r_minMag.c \
	f16_add.c \
	f16_div.c \
	f16_eq.c \
	f16_eq_signaling.c \
	f16_isSignalingNaN.c \
	f16_le.c \
	f16_le_quiet.c \
	f16_lt.c \
	f16_lt_quiet.c \
	f16_mulAdd.c \
	f16_mul.c \
	f16_rem.c \
	f16_roundToInt.c \
	f16_sqrt.c \
	f16_sub.c \
	f16_to_f128.c \
	f16_to_f32.c \
	f16_to_f64.c \
	f16_to_i32.c \
	f16_to_i32_r_minMag.c \
	f16_to_i64.c \
	f16_to_i64_r_minMag.c \
	f16_to_ui32.c \
	f16_to_ui32_r_minMag.c \
	f16_to_ui64.c \
	f16_to_ui64_r_minMag.c \
	f32_add.c \
	f32_classify.c \
	f32_div.c \
	f32_eq.c \
	f32_eq_signaling.c \
	f32_isSignalingNaN.c \
	f32_le.c \
	f32_le_quiet.c \
	f32_lt.c \
	f32_lt_quiet.c \
	f32_mulAdd.c \
	f32_mul.c \
	f32_rem.c \
	f32_roundToInt.c \
	f32_sqrt.c \
	f32_sub.c \
	f32_to_f128.c \
	f32_to_f16.c \
	f32_to_f64.c \
	f32_to_i32.c \
	f32_to_i32_r_minMag.c \
	f32_to_i64.c \
	f32_to_i64_r_minMag.c \
	f32_to_ui32.c \
	f32_to_ui32_r_minMag.c \
	f32_to_ui64.c \
	f32_to_ui64_r_minMag.c \
	f64_add.c \
	f64_classify.c \
	f64_div.c \
	f64_eq.c \
	f64_eq_signaling.c \
	f64_isSignalingNaN.c \
	f64_le.c \
	f64_le_quiet.c \
	f64_lt.c \
	f64_lt_quiet.c \
	f64_mulAdd.c \
	f64_mul.c \
	f64_rem.c \
	f64_roundToInt.c \
	f64_sqrt.c \
	f64_sub.c \
	f64_to_f128.c \
	f64_to_f16.c \
	f64_to_f32.c \
	f64_to_i32.c \
	f64_to_i32_r_minMag.c \
	f64_to_i64.c \
	f64_to_i64_r_minMag.c \
	f64_to_ui32.c \
	f64_to_ui32_r_minMag.c \
	f64_to_ui64.c \
	f64_to_ui64_r_minMag.c \
	i32_to_f128.c \
	i32_to_f16.c \
	i32_to_f32.c \
	i32_to_f64.c \
	i64_to_f128.c \
	i64_to_f16.c \
	i64_to_f32.c \
	i64_to_f64.c \
	s_add128.c \
	s_add256M.c \
	s_addCarryM.c \
	s_addComplCarryM.c \
	s_addMagsF128.c \
	s_addMagsF16.c \
	s_addMagsF32.c \
	s_addMagsF64.c \
	s_addM.c \
	s_approxRecip_1Ks.c \
	s_approxRecip32_1.c \
	s_approxRecipSqrt_1Ks.c \
	s_approxRecipSqrt32_1.c \
	s_commonNaNToF32UI.c \
	s_commonNaNToF64UI.c \
	s_compare128M.c \
	s_compare96M.c \
	s_countLeadingZeros16.c \
	s_countLeadingZeros32.c \
	s_countLeadingZeros64.c \
	s_countLeadingZeros8.c \
	s_eq128.c \
	s_f32UIToCommonNaN.c \
	s_f64UIToCommonNaN.c \
	s_le128.c \
	s_lt128.c \
	s_mul128By32.c \
	s_mul128MTo256M.c \
	s_mul128To256M.c \
	s_mul64ByShifted32To128.c \
	s_mul64To128.c \
	s_mul64To128M.c \
	s_mulAddF128.c \
	s_mulAddF16.c \
	s_mulAddF32.c \
	s_mulAddF64.c \
	s_negXM.c \
	s_normRoundPackToF128.c \
	s_normRoundPackToF16.c \
	s_normRoundPackToF32.c \
	s_normRoundPackToF64.c \
	s_normSubnormalF128Sig.c \
	s_normSubnormalF16Sig.c \
	s_normSubnormalF32Sig.c \
	s_normSubnormalF64Sig.c \
	softfloat_raiseFlags.c \
	softfloat_state.c \
	s_propagateNaNF16UI.c \
	s_propagateNaNF32UI.c \
	s_propagateNaNF64UI.c \
	s_propagateNaNF128UI.c \
	s_remStepMBy32.c \
	s_roundMToI64.c \
	s_roundMToUI64.c \
	s_roundPackMToI64.c \
	s_roundPackMToUI64.c \
	s_roundPackToF128.c \
	s_roundPackToF16.c \
	s_roundPackToF32.c \
	s_roundPackToF64.c \
	s_roundPackToI32.c \
	s_roundPackToI64.c \
	s_roundPackToUI32.c \
	s_roundPackToUI64.c \
	s_roundToI32.c \
	s_roundToI64.c \
	s_roundToUI32.c \
	s_roundToUI64.c \
	s_shiftRightJam128.c \
	s_shiftRightJam128Extra.c \
	s_shiftRightJam256M.c \
	s_shiftRightJam32.c \
	s_shiftRightJam64.c \
	s_shiftRightJam64Extra.c \
	s_shortShiftLeft128.c \
	s_shortShiftLeft64To96M.c \
	s_shortShiftRight128.c \
	s_shortShiftRightExtendM.c \
	s_shortShiftRightJam128.c \
	s_shortShiftRightJam128Extra.c \
	s_shortShiftRightJam64.c \
	s_shortShiftRightJam64Extra.c \
	s_shortShiftRightM.c \
	s_sub128.c \
	s_sub1XM.c \
	s_sub256M.c \
	s_subMagsF128.c \
	s_subMagsF16.c \
	s_subMagsF32.c \
	s_subMagsF64.c \
	s_subM.c \
	ui32_to_f128.c \
	ui32_to_f16.c \
	ui32_to_f32.c \
	ui32_to_f64.c \
	ui64_to_f128.c \
	ui64_to_f16.c \
	ui64_to_f32.c \
	ui64_to_f64.c \

softfloat_test_srcs =

softfloat_install_prog_srcs =
