#pragma once

// Expand two arguments
#define FB_PP_EXPAND2_1(p_a, p_b) p_a p_b
#define FB_PP_EXPAND2_2(p_a, p_b) FB_PP_EXPAND2_1(p_a, p_b)
#define FB_PP_EXPAND2_3(p_a, p_b) FB_PP_EXPAND2_2(p_a, p_b)
#define FB_PP_EXPAND2_4(p_a, p_b) FB_PP_EXPAND2_3(p_a, p_b)
#define FB_PP_EXPAND2_5(p_a, p_b) FB_PP_EXPAND2_4(p_a, p_b)
#define FB_PP_EXPAND2_6(p_a, p_b) FB_PP_EXPAND2_5(p_a, p_b)
#define FB_PP_EXPAND2_7(p_a, p_b) FB_PP_EXPAND2_6(p_a, p_b)
#define FB_PP_EXPAND2_8(p_a, p_b) FB_PP_EXPAND2_7(p_a, p_b)
#define FB_PP_EXPAND2_9(p_a, p_b) FB_PP_EXPAND2_8(p_a, p_b)
#define FB_PP_EXPAND2_10(p_a, p_b) FB_PP_EXPAND2_9(p_a, p_b)
#define FB_PP_EXPAND2_11(p_a, p_b) FB_PP_EXPAND2_10(p_a, p_b)
#define FB_PP_EXPAND2_12(p_a, p_b) FB_PP_EXPAND2_11(p_a, p_b)
#define FB_PP_EXPAND2_13(p_a, p_b) FB_PP_EXPAND2_12(p_a, p_b)
#define FB_PP_EXPAND2_14(p_a, p_b) FB_PP_EXPAND2_13(p_a, p_b)
#define FB_PP_EXPAND2_15(p_a, p_b) FB_PP_EXPAND2_14(p_a, p_b)
#define FB_PP_EXPAND2_16(p_a, p_b) FB_PP_EXPAND2_15(p_a, p_b)
#define FB_PP_EXPAND2_17(p_a, p_b) FB_PP_EXPAND2_16(p_a, p_b)
#define FB_PP_EXPAND2_18(p_a, p_b) FB_PP_EXPAND2_17(p_a, p_b)
#define FB_PP_EXPAND2_19(p_a, p_b) FB_PP_EXPAND2_18(p_a, p_b)
#define FB_PP_EXPAND2_20(p_a, p_b) FB_PP_EXPAND2_19(p_a, p_b)
#define FB_PP_EXPAND2_21(p_a, p_b) FB_PP_EXPAND2_20(p_a, p_b)
#define FB_PP_EXPAND2_22(p_a, p_b) FB_PP_EXPAND2_21(p_a, p_b)
#define FB_PP_EXPAND2_23(p_a, p_b) FB_PP_EXPAND2_22(p_a, p_b)
#define FB_PP_EXPAND2_24(p_a, p_b) FB_PP_EXPAND2_23(p_a, p_b)
#define FB_PP_EXPAND2_25(p_a, p_b) FB_PP_EXPAND2_24(p_a, p_b)
#define FB_PP_EXPAND2_26(p_a, p_b) FB_PP_EXPAND2_25(p_a, p_b)
#define FB_PP_EXPAND2_27(p_a, p_b) FB_PP_EXPAND2_26(p_a, p_b)
#define FB_PP_EXPAND2_28(p_a, p_b) FB_PP_EXPAND2_27(p_a, p_b)
#define FB_PP_EXPAND2_29(p_a, p_b) FB_PP_EXPAND2_28(p_a, p_b)
#define FB_PP_EXPAND2_30(p_a, p_b) FB_PP_EXPAND2_29(p_a, p_b)
#define FB_PP_EXPAND2_31(p_a, p_b) FB_PP_EXPAND2_30(p_a, p_b)
#define FB_PP_EXPAND2_32(p_a, p_b) FB_PP_EXPAND2_31(p_a, p_b)
#define FB_PP_EXPAND2(p_a, p_b)   FB_PP_EXPAND2_32(p_a, p_b)

// Expand the argument
#define FB_PP_EXPAND_32(p_arg) p_arg
#define FB_PP_EXPAND_31(p_arg) FB_PP_EXPAND_32(p_arg)
#define FB_PP_EXPAND_30(p_arg) FB_PP_EXPAND_31(p_arg)
#define FB_PP_EXPAND_29(p_arg) FB_PP_EXPAND_30(p_arg)
#define FB_PP_EXPAND_28(p_arg) FB_PP_EXPAND_29(p_arg)
#define FB_PP_EXPAND_27(p_arg) FB_PP_EXPAND_28(p_arg)
#define FB_PP_EXPAND_26(p_arg) FB_PP_EXPAND_27(p_arg)
#define FB_PP_EXPAND_25(p_arg) FB_PP_EXPAND_26(p_arg)
#define FB_PP_EXPAND_24(p_arg) FB_PP_EXPAND_25(p_arg)
#define FB_PP_EXPAND_23(p_arg) FB_PP_EXPAND_24(p_arg)
#define FB_PP_EXPAND_22(p_arg) FB_PP_EXPAND_23(p_arg)
#define FB_PP_EXPAND_21(p_arg) FB_PP_EXPAND_22(p_arg)
#define FB_PP_EXPAND_20(p_arg) FB_PP_EXPAND_21(p_arg)
#define FB_PP_EXPAND_19(p_arg) FB_PP_EXPAND_20(p_arg)
#define FB_PP_EXPAND_18(p_arg) FB_PP_EXPAND_19(p_arg)
#define FB_PP_EXPAND_17(p_arg) FB_PP_EXPAND_18(p_arg)
#define FB_PP_EXPAND_16(p_arg) FB_PP_EXPAND_17(p_arg)
#define FB_PP_EXPAND_15(p_arg) FB_PP_EXPAND_16(p_arg)
#define FB_PP_EXPAND_14(p_arg) FB_PP_EXPAND_15(p_arg)
#define FB_PP_EXPAND_13(p_arg) FB_PP_EXPAND_14(p_arg)
#define FB_PP_EXPAND_12(p_arg) FB_PP_EXPAND_13(p_arg)
#define FB_PP_EXPAND_11(p_arg) FB_PP_EXPAND_12(p_arg)
#define FB_PP_EXPAND_10(p_arg) FB_PP_EXPAND_11(p_arg)
#define FB_PP_EXPAND_9(p_arg) FB_PP_EXPAND_10(p_arg)
#define FB_PP_EXPAND_8(p_arg) FB_PP_EXPAND_9(p_arg)
#define FB_PP_EXPAND_7(p_arg) FB_PP_EXPAND_8(p_arg)
#define FB_PP_EXPAND_6(p_arg) FB_PP_EXPAND_7(p_arg)
#define FB_PP_EXPAND_5(p_arg) FB_PP_EXPAND_6(p_arg)
#define FB_PP_EXPAND_4(p_arg) FB_PP_EXPAND_5(p_arg)
#define FB_PP_EXPAND_3(p_arg) FB_PP_EXPAND_4(p_arg)
#define FB_PP_EXPAND_2(p_arg) FB_PP_EXPAND_3(p_arg)
#define FB_PP_EXPAND_1(p_arg) FB_PP_EXPAND_2(p_arg)
#define FB_PP_EXPAND(p_arg)   FB_PP_EXPAND_1(p_arg)


// Convert argument to string
#define FB_PP_TOSTR_32(p_value) #p_value
#define FB_PP_TOSTR_31(p_value) FB_PP_TOSTR_32(p_value)
#define FB_PP_TOSTR_30(p_value) FB_PP_TOSTR_31(p_value)
#define FB_PP_TOSTR_29(p_value) FB_PP_TOSTR_30(p_value)
#define FB_PP_TOSTR_28(p_value) FB_PP_TOSTR_29(p_value)
#define FB_PP_TOSTR_27(p_value) FB_PP_TOSTR_28(p_value)
#define FB_PP_TOSTR_26(p_value) FB_PP_TOSTR_27(p_value)
#define FB_PP_TOSTR_25(p_value) FB_PP_TOSTR_26(p_value)
#define FB_PP_TOSTR_24(p_value) FB_PP_TOSTR_25(p_value)
#define FB_PP_TOSTR_23(p_value) FB_PP_TOSTR_24(p_value)
#define FB_PP_TOSTR_22(p_value) FB_PP_TOSTR_23(p_value)
#define FB_PP_TOSTR_21(p_value) FB_PP_TOSTR_22(p_value)
#define FB_PP_TOSTR_20(p_value) FB_PP_TOSTR_21(p_value)
#define FB_PP_TOSTR_19(p_value) FB_PP_TOSTR_20(p_value)
#define FB_PP_TOSTR_18(p_value) FB_PP_TOSTR_19(p_value)
#define FB_PP_TOSTR_17(p_value) FB_PP_TOSTR_18(p_value)
#define FB_PP_TOSTR_16(p_value) FB_PP_TOSTR_17(p_value)
#define FB_PP_TOSTR_15(p_value) FB_PP_TOSTR_16(p_value)
#define FB_PP_TOSTR_14(p_value) FB_PP_TOSTR_15(p_value)
#define FB_PP_TOSTR_13(p_value) FB_PP_TOSTR_14(p_value)
#define FB_PP_TOSTR_12(p_value) FB_PP_TOSTR_13(p_value)
#define FB_PP_TOSTR_11(p_value) FB_PP_TOSTR_12(p_value)
#define FB_PP_TOSTR_10(p_value) FB_PP_TOSTR_11(p_value)
#define FB_PP_TOSTR_9(p_value) FB_PP_TOSTR_10(p_value)
#define FB_PP_TOSTR_8(p_value) FB_PP_TOSTR_9(p_value)
#define FB_PP_TOSTR_7(p_value) FB_PP_TOSTR_8(p_value)
#define FB_PP_TOSTR_6(p_value) FB_PP_TOSTR_7(p_value)
#define FB_PP_TOSTR_5(p_value) FB_PP_TOSTR_6(p_value)
#define FB_PP_TOSTR_4(p_value) FB_PP_TOSTR_5(p_value)
#define FB_PP_TOSTR_3(p_value) FB_PP_TOSTR_4(p_value)
#define FB_PP_TOSTR_2(p_value) FB_PP_TOSTR_3(p_value)
#define FB_PP_TOSTR_1(p_value) FB_PP_TOSTR_2(p_value)
#define FB_PP_TOSTR(p_value) FB_PP_TOSTR_1(p_value)


// Concatenate pair
#define FB_PP_CONCAT_32(p_a, p_b) p_a ## p_b
#define FB_PP_CONCAT_31(p_a, p_b) FB_PP_CONCAT_32(p_a, p_b)
#define FB_PP_CONCAT_30(p_a, p_b) FB_PP_CONCAT_31(p_a, p_b)
#define FB_PP_CONCAT_29(p_a, p_b) FB_PP_CONCAT_30(p_a, p_b)
#define FB_PP_CONCAT_28(p_a, p_b) FB_PP_CONCAT_29(p_a, p_b)
#define FB_PP_CONCAT_27(p_a, p_b) FB_PP_CONCAT_28(p_a, p_b)
#define FB_PP_CONCAT_26(p_a, p_b) FB_PP_CONCAT_27(p_a, p_b)
#define FB_PP_CONCAT_25(p_a, p_b) FB_PP_CONCAT_26(p_a, p_b)
#define FB_PP_CONCAT_24(p_a, p_b) FB_PP_CONCAT_25(p_a, p_b)
#define FB_PP_CONCAT_23(p_a, p_b) FB_PP_CONCAT_24(p_a, p_b)
#define FB_PP_CONCAT_22(p_a, p_b) FB_PP_CONCAT_23(p_a, p_b)
#define FB_PP_CONCAT_21(p_a, p_b) FB_PP_CONCAT_22(p_a, p_b)
#define FB_PP_CONCAT_20(p_a, p_b) FB_PP_CONCAT_21(p_a, p_b)
#define FB_PP_CONCAT_19(p_a, p_b) FB_PP_CONCAT_20(p_a, p_b)
#define FB_PP_CONCAT_18(p_a, p_b) FB_PP_CONCAT_19(p_a, p_b)
#define FB_PP_CONCAT_17(p_a, p_b) FB_PP_CONCAT_18(p_a, p_b)
#define FB_PP_CONCAT_16(p_a, p_b) FB_PP_CONCAT_17(p_a, p_b)
#define FB_PP_CONCAT_15(p_a, p_b) FB_PP_CONCAT_16(p_a, p_b)
#define FB_PP_CONCAT_14(p_a, p_b) FB_PP_CONCAT_15(p_a, p_b)
#define FB_PP_CONCAT_13(p_a, p_b) FB_PP_CONCAT_14(p_a, p_b)
#define FB_PP_CONCAT_12(p_a, p_b) FB_PP_CONCAT_13(p_a, p_b)
#define FB_PP_CONCAT_11(p_a, p_b) FB_PP_CONCAT_12(p_a, p_b)
#define FB_PP_CONCAT_10(p_a, p_b) FB_PP_CONCAT_11(p_a, p_b)
#define FB_PP_CONCAT_9(p_a, p_b) FB_PP_CONCAT_10(p_a, p_b)
#define FB_PP_CONCAT_8(p_a, p_b) FB_PP_CONCAT_9(p_a, p_b)
#define FB_PP_CONCAT_7(p_a, p_b) FB_PP_CONCAT_8(p_a, p_b)
#define FB_PP_CONCAT_6(p_a, p_b) FB_PP_CONCAT_7(p_a, p_b)
#define FB_PP_CONCAT_5(p_a, p_b) FB_PP_CONCAT_6(p_a, p_b)
#define FB_PP_CONCAT_4(p_a, p_b) FB_PP_CONCAT_5(p_a, p_b)
#define FB_PP_CONCAT_3(p_a, p_b) FB_PP_CONCAT_4(p_a, p_b)
#define FB_PP_CONCAT_2(p_a, p_b) FB_PP_CONCAT_3(p_a, p_b)
#define FB_PP_CONCAT_1(p_a, p_b) FB_PP_CONCAT_2(p_a, p_b)
#define FB_PP_CONCAT(p_a, p_b)   FB_PP_CONCAT_1(p_a, p_b)


// Get first argument from the parameter list
#define FB_PP_FIRST_5(p_first, ...) p_first
#define FB_PP_FIRST_4(p_args) FB_PP_FIRST_5 p_args
#define FB_PP_FIRST_3(p_args) FB_PP_FIRST_4(p_args)
#define FB_PP_FIRST_2(p_args) FB_PP_FIRST_3(p_args)
#define FB_PP_FIRST_1(p_args) FB_PP_FIRST_2(p_args)
#define FB_PP_FIRST(p_args)   FB_PP_FIRST_1(p_args)

// Get second argument from the parameter list
#define FB_PP_SECOND_5(p_first, p_second, ...) p_second
#define FB_PP_SECOND_4(p_args) FB_PP_SECOND_5 p_args
#define FB_PP_SECOND_3(p_args) FB_PP_SECOND_4(p_args)
#define FB_PP_SECOND_2(p_args) FB_PP_SECOND_3(p_args)
#define FB_PP_SECOND_1(p_args) FB_PP_SECOND_2(p_args)
#define FB_PP_SECOND(p_args)   FB_PP_SECOND_1(p_args)


// Get first and second argument from the parameter list
#define FB_PP_FIRST_TWO_3(p_first, p_second, ...) p_first, p_second
#define FB_PP_FIRST_TWO_2(p_args) FB_PP_FIRST_TWO_3 p_args
#define FB_PP_FIRST_TWO_1(p_args) FB_PP_FIRST_TWO_2(p_args)
#define FB_PP_FIRST_TWO(p_args)   FB_PP_FIRST_TWO_1(p_args)


// Removes the first argument from the argument list
#define FB_PP_POP_FIRST_6(p_args) p_args
#define FB_PP_POP_FIRST_5(p_args) FB_PP_POP_FIRST_6(p_args)
#define FB_PP_POP_FIRST_4(p_args) FB_PP_POP_FIRST_5(p_args)
#define FB_PP_POP_FIRST_3(p_first, ...) FB_PP_POP_FIRST_4((__VA_ARGS__))
#define FB_PP_POP_FIRST_2(p_args) FB_PP_POP_FIRST_3 p_args
#define FB_PP_POP_FIRST_1(p_args) FB_PP_POP_FIRST_2(p_args)
#define FB_PP_POP_FIRST(p_args)   FB_PP_POP_FIRST_1(p_args)

// Removes first and second argument from the argument list
#define FB_PP_POP_FIRST_TWO_6(p_args) p_args
#define FB_PP_POP_FIRST_TWO_5(p_args) FB_PP_POP_FIRST_TWO_6(p_args)
#define FB_PP_POP_FIRST_TWO_4(p_args) FB_PP_POP_FIRST_TWO_5(p_args)
#define FB_PP_POP_FIRST_TWO_3(p_first, p_second, ...) FB_PP_POP_FIRST_TWO_4((__VA_ARGS__))
#define FB_PP_POP_FIRST_TWO_2(p_args) FB_PP_POP_FIRST_TWO_3 p_args
#define FB_PP_POP_FIRST_TWO_1(p_args) FB_PP_POP_FIRST_TWO_2(p_args)
#define FB_PP_POP_FIRST_TWO(p_args)   FB_PP_POP_FIRST_TWO_1(p_args)

/* Get nth argument */

#define FB_PP_GET_ARG_1(p_input) FB_PP_FIRST(p_input)
#define FB_PP_GET_ARG_2(p_input) FB_PP_GET_ARG_1(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_3(p_input) FB_PP_GET_ARG_2(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_4(p_input) FB_PP_GET_ARG_3(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_5(p_input) FB_PP_GET_ARG_4(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_6(p_input) FB_PP_GET_ARG_5(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_7(p_input) FB_PP_GET_ARG_6(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_8(p_input) FB_PP_GET_ARG_7(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_9(p_input) FB_PP_GET_ARG_8(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_10(p_input) FB_PP_GET_ARG_9(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_11(p_input) FB_PP_GET_ARG_10(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_12(p_input) FB_PP_GET_ARG_11(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_13(p_input) FB_PP_GET_ARG_12(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_14(p_input) FB_PP_GET_ARG_13(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_15(p_input) FB_PP_GET_ARG_14(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_16(p_input) FB_PP_GET_ARG_15(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_17(p_input) FB_PP_GET_ARG_16(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_18(p_input) FB_PP_GET_ARG_17(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_19(p_input) FB_PP_GET_ARG_18(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_20(p_input) FB_PP_GET_ARG_19(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_21(p_input) FB_PP_GET_ARG_20(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_22(p_input) FB_PP_GET_ARG_21(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_23(p_input) FB_PP_GET_ARG_22(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_24(p_input) FB_PP_GET_ARG_23(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_25(p_input) FB_PP_GET_ARG_24(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_26(p_input) FB_PP_GET_ARG_25(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_27(p_input) FB_PP_GET_ARG_26(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_28(p_input) FB_PP_GET_ARG_27(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_29(p_input) FB_PP_GET_ARG_28(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_30(p_input) FB_PP_GET_ARG_29(FB_PP_POP_FIRST(p_input))
#define FB_PP_GET_ARG_31(p_input) FB_PP_GET_ARG_30(FB_PP_POP_FIRST(p_input))

#define FB_PP_GET_ARG_N(p_n, ...) FB_PP_CONCAT(FB_PP_GET_ARG_, p_n)((__VA_ARGS__))
#define FB_PP_GET_LAST_ARG(...) FB_PP_GET_ARG_N(FB_PP_NARG(__VA_ARGS__), __VA_ARGS__)
