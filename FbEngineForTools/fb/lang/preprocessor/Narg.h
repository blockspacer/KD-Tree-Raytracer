#ifndef FB_LANG_PLATFORM_NARG_H
#define FB_LANG_PLATFORM_NARG_H

// This macro figures out the number of parameters that is given to it.
// Useful for __VA_ARGS__ macro trickery and such.
//
// Max number of parameters is currently 127.

// Parameter list
#define FB_PP_NARG_PARAMS \
	          125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, \
	111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,  96, \
	 95,  94,  93,  92,  91,  90,  89,  88,  87,  86,  85,  84,  83,  82,  81,  80, \
	 79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,  68,  67,  66,  65,  64, \
	 63,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48, \
	 47,  46,  45,  44,  43,  42,  41,  40,  39,  38,  37,  36,  35,  34,  33,  32, \
	 31,  30,  29,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,  17,  16, \
	 15,  14,  13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0

// Implementation
#define FB_PP_NARG_IMPL( \
	              p_125, p_124, p_123, p_122, p_121, p_120, p_119, p_118, p_117, p_116, p_115, p_114, p_113, p_112, \
	p_111, p_110, p_109, p_108, p_107, p_106, p_105, p_104, p_103, p_102, p_101, p_100,  p_99,  p_98,  p_97,  p_96, \
	 p_95,  p_94,  p_93,  p_92,  p_91,  p_90,  p_89,  p_88,  p_87,  p_86,  p_85,  p_84,  p_83,  p_82,  p_81,  p_80, \
	 p_79,  p_78,  p_77,  p_76,  p_75,  p_74,  p_73,  p_72,  p_71,  p_70,  p_69,  p_68,  p_67,  p_66,  p_65,  p_64, \
	 p_63,  p_62,  p_61,  p_60,  p_59,  p_58,  p_57,  p_56,  p_55,  p_54,  p_53,  p_52,  p_51,  p_50,  p_49,  p_48, \
	 p_47,  p_46,  p_45,  p_44,  p_43,  p_42,  p_41,  p_40,  p_39,  p_38,  p_37,  p_36,  p_35,  p_34,  p_33,  p_32, \
	 p_31,  p_30,  p_29,  p_28,  p_27,  p_26,  p_25,  p_24,  p_23,  p_22,  p_21,  p_20,  p_19,  p_18,  p_17,  p_16, \
	 p_15,  p_14,  p_13,  p_12,  p_11,  p_10,   p_9,   p_8,   p_7,   p_6,   p_5,   p_4,   p_3,   p_2,   p_1,   p_n, \
	...) p_n

// Wrappers
#define FB_PP_NARG_(p_args) FB_PP_NARG_IMPL p_args
#define FB_PP_NARG(...) FB_PP_NARG_((__VA_ARGS__, FB_PP_NARG_PARAMS))

#endif
