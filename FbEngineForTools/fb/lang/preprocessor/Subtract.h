#ifndef FB_LANG_PLATFORM_SUBTRACT_H
#define FB_LANG_PLATFORM_SUBTRACT_H

// This macro can be used to subtract numbers in preprocessor.
// Useful for loops and such.

// Define allowed cases (max 255, increase if needed).
#define FB_PP_SUBTRACT_AT_1(p_value)   FB_PP_DECREASE(p_value)
#define FB_PP_SUBTRACT_AT_2(p_value)   FB_PP_SUBTRACT_AT_1(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_3(p_value)   FB_PP_SUBTRACT_AT_2(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_4(p_value)   FB_PP_SUBTRACT_AT_3(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_5(p_value)   FB_PP_SUBTRACT_AT_4(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_6(p_value)   FB_PP_SUBTRACT_AT_5(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_7(p_value)   FB_PP_SUBTRACT_AT_6(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_8(p_value)   FB_PP_SUBTRACT_AT_7(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_9(p_value)   FB_PP_SUBTRACT_AT_8(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_10(p_value)  FB_PP_SUBTRACT_AT_9(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_11(p_value)  FB_PP_SUBTRACT_AT_10(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_12(p_value)  FB_PP_SUBTRACT_AT_11(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_13(p_value)  FB_PP_SUBTRACT_AT_12(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_14(p_value)  FB_PP_SUBTRACT_AT_13(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_15(p_value)  FB_PP_SUBTRACT_AT_14(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_16(p_value)  FB_PP_SUBTRACT_AT_15(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_17(p_value)  FB_PP_SUBTRACT_AT_16(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_18(p_value)  FB_PP_SUBTRACT_AT_17(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_19(p_value)  FB_PP_SUBTRACT_AT_18(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_20(p_value)  FB_PP_SUBTRACT_AT_19(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_21(p_value)  FB_PP_SUBTRACT_AT_20(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_22(p_value)  FB_PP_SUBTRACT_AT_21(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_23(p_value)  FB_PP_SUBTRACT_AT_22(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_24(p_value)  FB_PP_SUBTRACT_AT_23(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_25(p_value)  FB_PP_SUBTRACT_AT_24(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_26(p_value)  FB_PP_SUBTRACT_AT_25(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_27(p_value)  FB_PP_SUBTRACT_AT_26(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_28(p_value)  FB_PP_SUBTRACT_AT_27(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_29(p_value)  FB_PP_SUBTRACT_AT_28(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_30(p_value)  FB_PP_SUBTRACT_AT_29(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_31(p_value)  FB_PP_SUBTRACT_AT_30(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_32(p_value)  FB_PP_SUBTRACT_AT_31(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_33(p_value)  FB_PP_SUBTRACT_AT_32(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_34(p_value)  FB_PP_SUBTRACT_AT_33(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_35(p_value)  FB_PP_SUBTRACT_AT_34(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_36(p_value)  FB_PP_SUBTRACT_AT_35(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_37(p_value)  FB_PP_SUBTRACT_AT_36(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_38(p_value)  FB_PP_SUBTRACT_AT_37(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_39(p_value)  FB_PP_SUBTRACT_AT_38(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_40(p_value)  FB_PP_SUBTRACT_AT_39(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_41(p_value)  FB_PP_SUBTRACT_AT_40(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_42(p_value)  FB_PP_SUBTRACT_AT_41(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_43(p_value)  FB_PP_SUBTRACT_AT_42(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_44(p_value)  FB_PP_SUBTRACT_AT_43(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_45(p_value)  FB_PP_SUBTRACT_AT_44(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_46(p_value)  FB_PP_SUBTRACT_AT_45(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_47(p_value)  FB_PP_SUBTRACT_AT_46(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_48(p_value)  FB_PP_SUBTRACT_AT_47(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_49(p_value)  FB_PP_SUBTRACT_AT_48(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_50(p_value)  FB_PP_SUBTRACT_AT_49(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_51(p_value)  FB_PP_SUBTRACT_AT_50(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_52(p_value)  FB_PP_SUBTRACT_AT_51(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_53(p_value)  FB_PP_SUBTRACT_AT_52(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_54(p_value)  FB_PP_SUBTRACT_AT_53(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_55(p_value)  FB_PP_SUBTRACT_AT_54(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_56(p_value)  FB_PP_SUBTRACT_AT_55(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_57(p_value)  FB_PP_SUBTRACT_AT_56(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_58(p_value)  FB_PP_SUBTRACT_AT_57(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_59(p_value)  FB_PP_SUBTRACT_AT_58(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_60(p_value)  FB_PP_SUBTRACT_AT_59(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_61(p_value)  FB_PP_SUBTRACT_AT_60(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_62(p_value)  FB_PP_SUBTRACT_AT_61(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_63(p_value)  FB_PP_SUBTRACT_AT_62(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_64(p_value)  FB_PP_SUBTRACT_AT_63(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_65(p_value)  FB_PP_SUBTRACT_AT_64(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_66(p_value)  FB_PP_SUBTRACT_AT_65(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_67(p_value)  FB_PP_SUBTRACT_AT_66(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_68(p_value)  FB_PP_SUBTRACT_AT_67(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_69(p_value)  FB_PP_SUBTRACT_AT_68(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_70(p_value)  FB_PP_SUBTRACT_AT_69(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_71(p_value)  FB_PP_SUBTRACT_AT_70(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_72(p_value)  FB_PP_SUBTRACT_AT_71(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_73(p_value)  FB_PP_SUBTRACT_AT_72(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_74(p_value)  FB_PP_SUBTRACT_AT_73(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_75(p_value)  FB_PP_SUBTRACT_AT_74(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_76(p_value)  FB_PP_SUBTRACT_AT_75(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_77(p_value)  FB_PP_SUBTRACT_AT_76(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_78(p_value)  FB_PP_SUBTRACT_AT_77(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_79(p_value)  FB_PP_SUBTRACT_AT_78(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_80(p_value)  FB_PP_SUBTRACT_AT_79(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_81(p_value)  FB_PP_SUBTRACT_AT_80(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_82(p_value)  FB_PP_SUBTRACT_AT_81(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_83(p_value)  FB_PP_SUBTRACT_AT_82(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_84(p_value)  FB_PP_SUBTRACT_AT_83(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_85(p_value)  FB_PP_SUBTRACT_AT_84(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_86(p_value)  FB_PP_SUBTRACT_AT_85(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_87(p_value)  FB_PP_SUBTRACT_AT_86(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_88(p_value)  FB_PP_SUBTRACT_AT_87(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_89(p_value)  FB_PP_SUBTRACT_AT_88(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_90(p_value)  FB_PP_SUBTRACT_AT_89(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_91(p_value)  FB_PP_SUBTRACT_AT_90(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_92(p_value)  FB_PP_SUBTRACT_AT_91(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_93(p_value)  FB_PP_SUBTRACT_AT_92(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_94(p_value)  FB_PP_SUBTRACT_AT_93(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_95(p_value)  FB_PP_SUBTRACT_AT_94(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_96(p_value)  FB_PP_SUBTRACT_AT_95(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_97(p_value)  FB_PP_SUBTRACT_AT_96(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_98(p_value)  FB_PP_SUBTRACT_AT_97(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_99(p_value)  FB_PP_SUBTRACT_AT_98(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_100(p_value) FB_PP_SUBTRACT_AT_99(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_101(p_value) FB_PP_SUBTRACT_AT_100(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_102(p_value) FB_PP_SUBTRACT_AT_101(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_103(p_value) FB_PP_SUBTRACT_AT_102(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_104(p_value) FB_PP_SUBTRACT_AT_103(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_105(p_value) FB_PP_SUBTRACT_AT_104(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_106(p_value) FB_PP_SUBTRACT_AT_105(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_107(p_value) FB_PP_SUBTRACT_AT_106(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_108(p_value) FB_PP_SUBTRACT_AT_107(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_109(p_value) FB_PP_SUBTRACT_AT_108(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_110(p_value) FB_PP_SUBTRACT_AT_109(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_111(p_value) FB_PP_SUBTRACT_AT_110(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_112(p_value) FB_PP_SUBTRACT_AT_111(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_113(p_value) FB_PP_SUBTRACT_AT_112(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_114(p_value) FB_PP_SUBTRACT_AT_113(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_115(p_value) FB_PP_SUBTRACT_AT_114(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_116(p_value) FB_PP_SUBTRACT_AT_115(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_117(p_value) FB_PP_SUBTRACT_AT_116(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_118(p_value) FB_PP_SUBTRACT_AT_117(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_119(p_value) FB_PP_SUBTRACT_AT_118(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_120(p_value) FB_PP_SUBTRACT_AT_119(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_121(p_value) FB_PP_SUBTRACT_AT_120(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_122(p_value) FB_PP_SUBTRACT_AT_121(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_123(p_value) FB_PP_SUBTRACT_AT_122(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_124(p_value) FB_PP_SUBTRACT_AT_123(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_125(p_value) FB_PP_SUBTRACT_AT_124(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_126(p_value) FB_PP_SUBTRACT_AT_125(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_127(p_value) FB_PP_SUBTRACT_AT_126(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_128(p_value) FB_PP_SUBTRACT_AT_127(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_129(p_value) FB_PP_SUBTRACT_AT_128(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_130(p_value) FB_PP_SUBTRACT_AT_129(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_131(p_value) FB_PP_SUBTRACT_AT_130(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_132(p_value) FB_PP_SUBTRACT_AT_131(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_133(p_value) FB_PP_SUBTRACT_AT_132(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_134(p_value) FB_PP_SUBTRACT_AT_133(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_135(p_value) FB_PP_SUBTRACT_AT_134(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_136(p_value) FB_PP_SUBTRACT_AT_135(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_137(p_value) FB_PP_SUBTRACT_AT_136(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_138(p_value) FB_PP_SUBTRACT_AT_137(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_139(p_value) FB_PP_SUBTRACT_AT_138(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_140(p_value) FB_PP_SUBTRACT_AT_139(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_141(p_value) FB_PP_SUBTRACT_AT_140(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_142(p_value) FB_PP_SUBTRACT_AT_141(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_143(p_value) FB_PP_SUBTRACT_AT_142(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_144(p_value) FB_PP_SUBTRACT_AT_143(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_145(p_value) FB_PP_SUBTRACT_AT_144(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_146(p_value) FB_PP_SUBTRACT_AT_145(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_147(p_value) FB_PP_SUBTRACT_AT_146(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_148(p_value) FB_PP_SUBTRACT_AT_147(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_149(p_value) FB_PP_SUBTRACT_AT_148(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_150(p_value) FB_PP_SUBTRACT_AT_149(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_151(p_value) FB_PP_SUBTRACT_AT_150(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_152(p_value) FB_PP_SUBTRACT_AT_151(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_153(p_value) FB_PP_SUBTRACT_AT_152(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_154(p_value) FB_PP_SUBTRACT_AT_153(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_155(p_value) FB_PP_SUBTRACT_AT_154(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_156(p_value) FB_PP_SUBTRACT_AT_155(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_157(p_value) FB_PP_SUBTRACT_AT_156(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_158(p_value) FB_PP_SUBTRACT_AT_157(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_159(p_value) FB_PP_SUBTRACT_AT_158(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_160(p_value) FB_PP_SUBTRACT_AT_159(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_161(p_value) FB_PP_SUBTRACT_AT_160(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_162(p_value) FB_PP_SUBTRACT_AT_161(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_163(p_value) FB_PP_SUBTRACT_AT_162(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_164(p_value) FB_PP_SUBTRACT_AT_163(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_165(p_value) FB_PP_SUBTRACT_AT_164(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_166(p_value) FB_PP_SUBTRACT_AT_165(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_167(p_value) FB_PP_SUBTRACT_AT_166(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_168(p_value) FB_PP_SUBTRACT_AT_167(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_169(p_value) FB_PP_SUBTRACT_AT_168(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_170(p_value) FB_PP_SUBTRACT_AT_169(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_171(p_value) FB_PP_SUBTRACT_AT_170(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_172(p_value) FB_PP_SUBTRACT_AT_171(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_173(p_value) FB_PP_SUBTRACT_AT_172(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_174(p_value) FB_PP_SUBTRACT_AT_173(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_175(p_value) FB_PP_SUBTRACT_AT_174(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_176(p_value) FB_PP_SUBTRACT_AT_175(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_177(p_value) FB_PP_SUBTRACT_AT_176(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_178(p_value) FB_PP_SUBTRACT_AT_177(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_179(p_value) FB_PP_SUBTRACT_AT_178(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_180(p_value) FB_PP_SUBTRACT_AT_179(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_181(p_value) FB_PP_SUBTRACT_AT_180(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_182(p_value) FB_PP_SUBTRACT_AT_181(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_183(p_value) FB_PP_SUBTRACT_AT_182(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_184(p_value) FB_PP_SUBTRACT_AT_183(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_185(p_value) FB_PP_SUBTRACT_AT_184(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_186(p_value) FB_PP_SUBTRACT_AT_185(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_187(p_value) FB_PP_SUBTRACT_AT_186(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_188(p_value) FB_PP_SUBTRACT_AT_187(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_189(p_value) FB_PP_SUBTRACT_AT_188(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_190(p_value) FB_PP_SUBTRACT_AT_189(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_191(p_value) FB_PP_SUBTRACT_AT_190(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_192(p_value) FB_PP_SUBTRACT_AT_191(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_193(p_value) FB_PP_SUBTRACT_AT_192(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_194(p_value) FB_PP_SUBTRACT_AT_193(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_195(p_value) FB_PP_SUBTRACT_AT_194(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_196(p_value) FB_PP_SUBTRACT_AT_195(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_197(p_value) FB_PP_SUBTRACT_AT_196(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_198(p_value) FB_PP_SUBTRACT_AT_197(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_199(p_value) FB_PP_SUBTRACT_AT_198(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_200(p_value) FB_PP_SUBTRACT_AT_199(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_201(p_value) FB_PP_SUBTRACT_AT_200(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_202(p_value) FB_PP_SUBTRACT_AT_201(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_203(p_value) FB_PP_SUBTRACT_AT_202(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_204(p_value) FB_PP_SUBTRACT_AT_203(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_205(p_value) FB_PP_SUBTRACT_AT_204(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_206(p_value) FB_PP_SUBTRACT_AT_205(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_207(p_value) FB_PP_SUBTRACT_AT_206(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_208(p_value) FB_PP_SUBTRACT_AT_207(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_209(p_value) FB_PP_SUBTRACT_AT_208(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_210(p_value) FB_PP_SUBTRACT_AT_209(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_211(p_value) FB_PP_SUBTRACT_AT_210(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_212(p_value) FB_PP_SUBTRACT_AT_211(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_213(p_value) FB_PP_SUBTRACT_AT_212(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_214(p_value) FB_PP_SUBTRACT_AT_213(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_215(p_value) FB_PP_SUBTRACT_AT_214(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_216(p_value) FB_PP_SUBTRACT_AT_215(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_217(p_value) FB_PP_SUBTRACT_AT_216(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_218(p_value) FB_PP_SUBTRACT_AT_217(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_219(p_value) FB_PP_SUBTRACT_AT_218(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_220(p_value) FB_PP_SUBTRACT_AT_219(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_221(p_value) FB_PP_SUBTRACT_AT_220(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_222(p_value) FB_PP_SUBTRACT_AT_221(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_223(p_value) FB_PP_SUBTRACT_AT_222(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_224(p_value) FB_PP_SUBTRACT_AT_223(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_225(p_value) FB_PP_SUBTRACT_AT_224(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_226(p_value) FB_PP_SUBTRACT_AT_225(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_227(p_value) FB_PP_SUBTRACT_AT_226(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_228(p_value) FB_PP_SUBTRACT_AT_227(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_229(p_value) FB_PP_SUBTRACT_AT_228(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_230(p_value) FB_PP_SUBTRACT_AT_229(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_231(p_value) FB_PP_SUBTRACT_AT_230(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_232(p_value) FB_PP_SUBTRACT_AT_231(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_233(p_value) FB_PP_SUBTRACT_AT_232(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_234(p_value) FB_PP_SUBTRACT_AT_233(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_235(p_value) FB_PP_SUBTRACT_AT_234(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_236(p_value) FB_PP_SUBTRACT_AT_235(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_237(p_value) FB_PP_SUBTRACT_AT_236(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_238(p_value) FB_PP_SUBTRACT_AT_237(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_239(p_value) FB_PP_SUBTRACT_AT_238(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_240(p_value) FB_PP_SUBTRACT_AT_239(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_241(p_value) FB_PP_SUBTRACT_AT_240(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_242(p_value) FB_PP_SUBTRACT_AT_241(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_243(p_value) FB_PP_SUBTRACT_AT_242(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_244(p_value) FB_PP_SUBTRACT_AT_243(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_245(p_value) FB_PP_SUBTRACT_AT_244(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_246(p_value) FB_PP_SUBTRACT_AT_245(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_247(p_value) FB_PP_SUBTRACT_AT_246(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_248(p_value) FB_PP_SUBTRACT_AT_247(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_249(p_value) FB_PP_SUBTRACT_AT_248(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_250(p_value) FB_PP_SUBTRACT_AT_249(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_251(p_value) FB_PP_SUBTRACT_AT_250(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_252(p_value) FB_PP_SUBTRACT_AT_251(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_253(p_value) FB_PP_SUBTRACT_AT_252(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_254(p_value) FB_PP_SUBTRACT_AT_253(FB_PP_DECREASE(p_value))
#define FB_PP_SUBTRACT_AT_255(p_value) FB_PP_SUBTRACT_AT_254(FB_PP_DECREASE(p_value))


// Subtract the number
#define FB_PP_SUBTRACT_4(p_dest, p_oper) p_oper(p_dest)
#define FB_PP_SUBTRACT_3(p_dest, p_oper) FB_PP_SUBTRACT_4(p_dest, p_oper)
#define FB_PP_SUBTRACT_2(p_dest, p_oper) FB_PP_SUBTRACT_3(p_dest, p_oper)
#define FB_PP_SUBTRACT_1(p_dest, p_oper) FB_PP_SUBTRACT_2(p_dest, p_oper)
#define FB_PP_SUBTRACT(p_dest, p_src) \
	FB_PP_SUBTRACT_1(p_dest, FB_PP_CONCAT(FB_PP_SUBTRACT_AT_, p_src))

#endif

