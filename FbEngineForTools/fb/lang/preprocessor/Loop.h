#pragma once

#include "fb/lang/preprocessor/Halve.h"

// This macro allows preprocessor loops. You give the macro a number, and it
// returns either LOOP or DONE. DONE is returned, if the parameter was 1.
// Otherwise LOOP is returned. Useful with FB_PP_NARG macro and __VA_ARGS__.

// Define allowed loop states (max 255, increase if needed).
#define FB_PP_LOOP_AT_1   DONE
#define FB_PP_LOOP_AT_2   LOOP
#define FB_PP_LOOP_AT_3   LOOP
#define FB_PP_LOOP_AT_4   LOOP
#define FB_PP_LOOP_AT_5   LOOP
#define FB_PP_LOOP_AT_6   LOOP
#define FB_PP_LOOP_AT_7   LOOP
#define FB_PP_LOOP_AT_8   LOOP
#define FB_PP_LOOP_AT_9   LOOP
#define FB_PP_LOOP_AT_10  LOOP
#define FB_PP_LOOP_AT_11  LOOP
#define FB_PP_LOOP_AT_12  LOOP
#define FB_PP_LOOP_AT_13  LOOP
#define FB_PP_LOOP_AT_14  LOOP
#define FB_PP_LOOP_AT_15  LOOP
#define FB_PP_LOOP_AT_16  LOOP
#define FB_PP_LOOP_AT_17  LOOP
#define FB_PP_LOOP_AT_18  LOOP
#define FB_PP_LOOP_AT_19  LOOP
#define FB_PP_LOOP_AT_20  LOOP
#define FB_PP_LOOP_AT_21  LOOP
#define FB_PP_LOOP_AT_22  LOOP
#define FB_PP_LOOP_AT_23  LOOP
#define FB_PP_LOOP_AT_24  LOOP
#define FB_PP_LOOP_AT_25  LOOP
#define FB_PP_LOOP_AT_26  LOOP
#define FB_PP_LOOP_AT_27  LOOP
#define FB_PP_LOOP_AT_28  LOOP
#define FB_PP_LOOP_AT_29  LOOP
#define FB_PP_LOOP_AT_30  LOOP
#define FB_PP_LOOP_AT_31  LOOP
#define FB_PP_LOOP_AT_32  LOOP
#define FB_PP_LOOP_AT_33  LOOP
#define FB_PP_LOOP_AT_34  LOOP
#define FB_PP_LOOP_AT_35  LOOP
#define FB_PP_LOOP_AT_36  LOOP
#define FB_PP_LOOP_AT_37  LOOP
#define FB_PP_LOOP_AT_38  LOOP
#define FB_PP_LOOP_AT_39  LOOP
#define FB_PP_LOOP_AT_40  LOOP
#define FB_PP_LOOP_AT_41  LOOP
#define FB_PP_LOOP_AT_42  LOOP
#define FB_PP_LOOP_AT_43  LOOP
#define FB_PP_LOOP_AT_44  LOOP
#define FB_PP_LOOP_AT_45  LOOP
#define FB_PP_LOOP_AT_46  LOOP
#define FB_PP_LOOP_AT_47  LOOP
#define FB_PP_LOOP_AT_48  LOOP
#define FB_PP_LOOP_AT_49  LOOP
#define FB_PP_LOOP_AT_50  LOOP
#define FB_PP_LOOP_AT_51  LOOP
#define FB_PP_LOOP_AT_52  LOOP
#define FB_PP_LOOP_AT_53  LOOP
#define FB_PP_LOOP_AT_54  LOOP
#define FB_PP_LOOP_AT_55  LOOP
#define FB_PP_LOOP_AT_56  LOOP
#define FB_PP_LOOP_AT_57  LOOP
#define FB_PP_LOOP_AT_58  LOOP
#define FB_PP_LOOP_AT_59  LOOP
#define FB_PP_LOOP_AT_60  LOOP
#define FB_PP_LOOP_AT_61  LOOP
#define FB_PP_LOOP_AT_62  LOOP
#define FB_PP_LOOP_AT_63  LOOP
#define FB_PP_LOOP_AT_64  LOOP
#define FB_PP_LOOP_AT_65  LOOP
#define FB_PP_LOOP_AT_66  LOOP
#define FB_PP_LOOP_AT_67  LOOP
#define FB_PP_LOOP_AT_68  LOOP
#define FB_PP_LOOP_AT_69  LOOP
#define FB_PP_LOOP_AT_70  LOOP
#define FB_PP_LOOP_AT_71  LOOP
#define FB_PP_LOOP_AT_72  LOOP
#define FB_PP_LOOP_AT_73  LOOP
#define FB_PP_LOOP_AT_74  LOOP
#define FB_PP_LOOP_AT_75  LOOP
#define FB_PP_LOOP_AT_76  LOOP
#define FB_PP_LOOP_AT_77  LOOP
#define FB_PP_LOOP_AT_78  LOOP
#define FB_PP_LOOP_AT_79  LOOP
#define FB_PP_LOOP_AT_80  LOOP
#define FB_PP_LOOP_AT_81  LOOP
#define FB_PP_LOOP_AT_82  LOOP
#define FB_PP_LOOP_AT_83  LOOP
#define FB_PP_LOOP_AT_84  LOOP
#define FB_PP_LOOP_AT_85  LOOP
#define FB_PP_LOOP_AT_86  LOOP
#define FB_PP_LOOP_AT_87  LOOP
#define FB_PP_LOOP_AT_88  LOOP
#define FB_PP_LOOP_AT_89  LOOP
#define FB_PP_LOOP_AT_90  LOOP
#define FB_PP_LOOP_AT_91  LOOP
#define FB_PP_LOOP_AT_92  LOOP
#define FB_PP_LOOP_AT_93  LOOP
#define FB_PP_LOOP_AT_94  LOOP
#define FB_PP_LOOP_AT_95  LOOP
#define FB_PP_LOOP_AT_96  LOOP
#define FB_PP_LOOP_AT_97  LOOP
#define FB_PP_LOOP_AT_98  LOOP
#define FB_PP_LOOP_AT_99  LOOP
#define FB_PP_LOOP_AT_100 LOOP
#define FB_PP_LOOP_AT_101 LOOP
#define FB_PP_LOOP_AT_102 LOOP
#define FB_PP_LOOP_AT_103 LOOP
#define FB_PP_LOOP_AT_104 LOOP
#define FB_PP_LOOP_AT_105 LOOP
#define FB_PP_LOOP_AT_106 LOOP
#define FB_PP_LOOP_AT_107 LOOP
#define FB_PP_LOOP_AT_108 LOOP
#define FB_PP_LOOP_AT_109 LOOP
#define FB_PP_LOOP_AT_110 LOOP
#define FB_PP_LOOP_AT_111 LOOP
#define FB_PP_LOOP_AT_112 LOOP
#define FB_PP_LOOP_AT_113 LOOP
#define FB_PP_LOOP_AT_114 LOOP
#define FB_PP_LOOP_AT_115 LOOP
#define FB_PP_LOOP_AT_116 LOOP
#define FB_PP_LOOP_AT_117 LOOP
#define FB_PP_LOOP_AT_118 LOOP
#define FB_PP_LOOP_AT_119 LOOP
#define FB_PP_LOOP_AT_120 LOOP
#define FB_PP_LOOP_AT_121 LOOP
#define FB_PP_LOOP_AT_122 LOOP
#define FB_PP_LOOP_AT_123 LOOP
#define FB_PP_LOOP_AT_124 LOOP
#define FB_PP_LOOP_AT_125 LOOP
#define FB_PP_LOOP_AT_126 LOOP
#define FB_PP_LOOP_AT_127 LOOP
#define FB_PP_LOOP_AT_128 LOOP
#define FB_PP_LOOP_AT_129 LOOP
#define FB_PP_LOOP_AT_130 LOOP
#define FB_PP_LOOP_AT_131 LOOP
#define FB_PP_LOOP_AT_132 LOOP
#define FB_PP_LOOP_AT_133 LOOP
#define FB_PP_LOOP_AT_134 LOOP
#define FB_PP_LOOP_AT_135 LOOP
#define FB_PP_LOOP_AT_136 LOOP
#define FB_PP_LOOP_AT_137 LOOP
#define FB_PP_LOOP_AT_138 LOOP
#define FB_PP_LOOP_AT_139 LOOP
#define FB_PP_LOOP_AT_140 LOOP
#define FB_PP_LOOP_AT_141 LOOP
#define FB_PP_LOOP_AT_142 LOOP
#define FB_PP_LOOP_AT_143 LOOP
#define FB_PP_LOOP_AT_144 LOOP
#define FB_PP_LOOP_AT_145 LOOP
#define FB_PP_LOOP_AT_146 LOOP
#define FB_PP_LOOP_AT_147 LOOP
#define FB_PP_LOOP_AT_148 LOOP
#define FB_PP_LOOP_AT_149 LOOP
#define FB_PP_LOOP_AT_150 LOOP
#define FB_PP_LOOP_AT_151 LOOP
#define FB_PP_LOOP_AT_152 LOOP
#define FB_PP_LOOP_AT_153 LOOP
#define FB_PP_LOOP_AT_154 LOOP
#define FB_PP_LOOP_AT_155 LOOP
#define FB_PP_LOOP_AT_156 LOOP
#define FB_PP_LOOP_AT_157 LOOP
#define FB_PP_LOOP_AT_158 LOOP
#define FB_PP_LOOP_AT_159 LOOP
#define FB_PP_LOOP_AT_160 LOOP
#define FB_PP_LOOP_AT_161 LOOP
#define FB_PP_LOOP_AT_162 LOOP
#define FB_PP_LOOP_AT_163 LOOP
#define FB_PP_LOOP_AT_164 LOOP
#define FB_PP_LOOP_AT_165 LOOP
#define FB_PP_LOOP_AT_166 LOOP
#define FB_PP_LOOP_AT_167 LOOP
#define FB_PP_LOOP_AT_168 LOOP
#define FB_PP_LOOP_AT_169 LOOP
#define FB_PP_LOOP_AT_170 LOOP
#define FB_PP_LOOP_AT_171 LOOP
#define FB_PP_LOOP_AT_172 LOOP
#define FB_PP_LOOP_AT_173 LOOP
#define FB_PP_LOOP_AT_174 LOOP
#define FB_PP_LOOP_AT_175 LOOP
#define FB_PP_LOOP_AT_176 LOOP
#define FB_PP_LOOP_AT_177 LOOP
#define FB_PP_LOOP_AT_178 LOOP
#define FB_PP_LOOP_AT_179 LOOP
#define FB_PP_LOOP_AT_180 LOOP
#define FB_PP_LOOP_AT_181 LOOP
#define FB_PP_LOOP_AT_182 LOOP
#define FB_PP_LOOP_AT_183 LOOP
#define FB_PP_LOOP_AT_184 LOOP
#define FB_PP_LOOP_AT_185 LOOP
#define FB_PP_LOOP_AT_186 LOOP
#define FB_PP_LOOP_AT_187 LOOP
#define FB_PP_LOOP_AT_188 LOOP
#define FB_PP_LOOP_AT_189 LOOP
#define FB_PP_LOOP_AT_190 LOOP
#define FB_PP_LOOP_AT_191 LOOP
#define FB_PP_LOOP_AT_192 LOOP
#define FB_PP_LOOP_AT_193 LOOP
#define FB_PP_LOOP_AT_194 LOOP
#define FB_PP_LOOP_AT_195 LOOP
#define FB_PP_LOOP_AT_196 LOOP
#define FB_PP_LOOP_AT_197 LOOP
#define FB_PP_LOOP_AT_198 LOOP
#define FB_PP_LOOP_AT_199 LOOP
#define FB_PP_LOOP_AT_200 LOOP
#define FB_PP_LOOP_AT_201 LOOP
#define FB_PP_LOOP_AT_202 LOOP
#define FB_PP_LOOP_AT_203 LOOP
#define FB_PP_LOOP_AT_204 LOOP
#define FB_PP_LOOP_AT_205 LOOP
#define FB_PP_LOOP_AT_206 LOOP
#define FB_PP_LOOP_AT_207 LOOP
#define FB_PP_LOOP_AT_208 LOOP
#define FB_PP_LOOP_AT_209 LOOP
#define FB_PP_LOOP_AT_210 LOOP
#define FB_PP_LOOP_AT_211 LOOP
#define FB_PP_LOOP_AT_212 LOOP
#define FB_PP_LOOP_AT_213 LOOP
#define FB_PP_LOOP_AT_214 LOOP
#define FB_PP_LOOP_AT_215 LOOP
#define FB_PP_LOOP_AT_216 LOOP
#define FB_PP_LOOP_AT_217 LOOP
#define FB_PP_LOOP_AT_218 LOOP
#define FB_PP_LOOP_AT_219 LOOP
#define FB_PP_LOOP_AT_220 LOOP
#define FB_PP_LOOP_AT_221 LOOP
#define FB_PP_LOOP_AT_222 LOOP
#define FB_PP_LOOP_AT_223 LOOP
#define FB_PP_LOOP_AT_224 LOOP
#define FB_PP_LOOP_AT_225 LOOP
#define FB_PP_LOOP_AT_226 LOOP
#define FB_PP_LOOP_AT_227 LOOP
#define FB_PP_LOOP_AT_228 LOOP
#define FB_PP_LOOP_AT_229 LOOP
#define FB_PP_LOOP_AT_230 LOOP
#define FB_PP_LOOP_AT_231 LOOP
#define FB_PP_LOOP_AT_232 LOOP
#define FB_PP_LOOP_AT_233 LOOP
#define FB_PP_LOOP_AT_234 LOOP
#define FB_PP_LOOP_AT_235 LOOP
#define FB_PP_LOOP_AT_236 LOOP
#define FB_PP_LOOP_AT_237 LOOP
#define FB_PP_LOOP_AT_238 LOOP
#define FB_PP_LOOP_AT_239 LOOP
#define FB_PP_LOOP_AT_240 LOOP
#define FB_PP_LOOP_AT_241 LOOP
#define FB_PP_LOOP_AT_242 LOOP
#define FB_PP_LOOP_AT_243 LOOP
#define FB_PP_LOOP_AT_244 LOOP
#define FB_PP_LOOP_AT_245 LOOP
#define FB_PP_LOOP_AT_246 LOOP
#define FB_PP_LOOP_AT_247 LOOP
#define FB_PP_LOOP_AT_248 LOOP
#define FB_PP_LOOP_AT_249 LOOP
#define FB_PP_LOOP_AT_250 LOOP
#define FB_PP_LOOP_AT_251 LOOP
#define FB_PP_LOOP_AT_252 LOOP
#define FB_PP_LOOP_AT_253 LOOP
#define FB_PP_LOOP_AT_254 LOOP
#define FB_PP_LOOP_AT_255 LOOP


// Figure out the looping state
#define FB_PP_LOOP_STATE_4(p_number) p_number
#define FB_PP_LOOP_STATE_3(p_number)  FB_PP_LOOP_STATE_4(p_number)
#define FB_PP_LOOP_STATE_2(p_number)  FB_PP_LOOP_STATE_3(p_number)
#define FB_PP_LOOP_STATE_1(p_number)  FB_PP_LOOP_STATE_2(p_number)
#define FB_PP_LOOP_STATE(p_number) \
	FB_PP_LOOP_STATE_1(FB_PP_CONCAT(FB_PP_LOOP_AT_, p_number))


// For loop cases (max 255, increase if needed)
#define FB_PP_FOR_1(p_oper, p_args)   FB_PP_UNPACK(FB_PP_EXPAND((p_oper p_args)))
#define FB_PP_FOR_2(p_oper, p_args)   FB_PP_FOR_1(p_oper, (p_oper p_args))
#define FB_PP_FOR_3(p_oper, p_args)   FB_PP_FOR_2(p_oper, (p_oper p_args))
#define FB_PP_FOR_4(p_oper, p_args)   FB_PP_FOR_3(p_oper, (p_oper p_args))
#define FB_PP_FOR_5(p_oper, p_args)   FB_PP_FOR_4(p_oper, (p_oper p_args))
#define FB_PP_FOR_6(p_oper, p_args)   FB_PP_FOR_5(p_oper, (p_oper p_args))
#define FB_PP_FOR_7(p_oper, p_args)   FB_PP_FOR_6(p_oper, (p_oper p_args))
#define FB_PP_FOR_8(p_oper, p_args)   FB_PP_FOR_7(p_oper, (p_oper p_args))
#define FB_PP_FOR_9(p_oper, p_args)   FB_PP_FOR_8(p_oper, (p_oper p_args))
#define FB_PP_FOR_10(p_oper, p_args)  FB_PP_FOR_9(p_oper, (p_oper p_args))
#define FB_PP_FOR_11(p_oper, p_args)  FB_PP_FOR_10(p_oper, (p_oper p_args))
#define FB_PP_FOR_12(p_oper, p_args)  FB_PP_FOR_11(p_oper, (p_oper p_args))
#define FB_PP_FOR_13(p_oper, p_args)  FB_PP_FOR_12(p_oper, (p_oper p_args))
#define FB_PP_FOR_14(p_oper, p_args)  FB_PP_FOR_13(p_oper, (p_oper p_args))
#define FB_PP_FOR_15(p_oper, p_args)  FB_PP_FOR_14(p_oper, (p_oper p_args))
#define FB_PP_FOR_16(p_oper, p_args)  FB_PP_FOR_15(p_oper, (p_oper p_args))
#define FB_PP_FOR_17(p_oper, p_args)  FB_PP_FOR_16(p_oper, (p_oper p_args))
#define FB_PP_FOR_18(p_oper, p_args)  FB_PP_FOR_17(p_oper, (p_oper p_args))
#define FB_PP_FOR_19(p_oper, p_args)  FB_PP_FOR_18(p_oper, (p_oper p_args))
#define FB_PP_FOR_20(p_oper, p_args)  FB_PP_FOR_19(p_oper, (p_oper p_args))
#define FB_PP_FOR_21(p_oper, p_args)  FB_PP_FOR_20(p_oper, (p_oper p_args))
#define FB_PP_FOR_22(p_oper, p_args)  FB_PP_FOR_21(p_oper, (p_oper p_args))
#define FB_PP_FOR_23(p_oper, p_args)  FB_PP_FOR_22(p_oper, (p_oper p_args))
#define FB_PP_FOR_24(p_oper, p_args)  FB_PP_FOR_23(p_oper, (p_oper p_args))
#define FB_PP_FOR_25(p_oper, p_args)  FB_PP_FOR_24(p_oper, (p_oper p_args))
#define FB_PP_FOR_26(p_oper, p_args)  FB_PP_FOR_25(p_oper, (p_oper p_args))
#define FB_PP_FOR_27(p_oper, p_args)  FB_PP_FOR_26(p_oper, (p_oper p_args))
#define FB_PP_FOR_28(p_oper, p_args)  FB_PP_FOR_27(p_oper, (p_oper p_args))
#define FB_PP_FOR_29(p_oper, p_args)  FB_PP_FOR_28(p_oper, (p_oper p_args))
#define FB_PP_FOR_30(p_oper, p_args)  FB_PP_FOR_29(p_oper, (p_oper p_args))
#define FB_PP_FOR_31(p_oper, p_args)  FB_PP_FOR_30(p_oper, (p_oper p_args))
#define FB_PP_FOR_32(p_oper, p_args)  FB_PP_FOR_31(p_oper, (p_oper p_args))
#define FB_PP_FOR_33(p_oper, p_args)  FB_PP_FOR_32(p_oper, (p_oper p_args))
#define FB_PP_FOR_34(p_oper, p_args)  FB_PP_FOR_33(p_oper, (p_oper p_args))
#define FB_PP_FOR_35(p_oper, p_args)  FB_PP_FOR_34(p_oper, (p_oper p_args))
#define FB_PP_FOR_36(p_oper, p_args)  FB_PP_FOR_35(p_oper, (p_oper p_args))
#define FB_PP_FOR_37(p_oper, p_args)  FB_PP_FOR_36(p_oper, (p_oper p_args))
#define FB_PP_FOR_38(p_oper, p_args)  FB_PP_FOR_37(p_oper, (p_oper p_args))
#define FB_PP_FOR_39(p_oper, p_args)  FB_PP_FOR_38(p_oper, (p_oper p_args))
#define FB_PP_FOR_40(p_oper, p_args)  FB_PP_FOR_39(p_oper, (p_oper p_args))
#define FB_PP_FOR_41(p_oper, p_args)  FB_PP_FOR_40(p_oper, (p_oper p_args))
#define FB_PP_FOR_42(p_oper, p_args)  FB_PP_FOR_41(p_oper, (p_oper p_args))
#define FB_PP_FOR_43(p_oper, p_args)  FB_PP_FOR_42(p_oper, (p_oper p_args))
#define FB_PP_FOR_44(p_oper, p_args)  FB_PP_FOR_43(p_oper, (p_oper p_args))
#define FB_PP_FOR_45(p_oper, p_args)  FB_PP_FOR_44(p_oper, (p_oper p_args))
#define FB_PP_FOR_46(p_oper, p_args)  FB_PP_FOR_45(p_oper, (p_oper p_args))
#define FB_PP_FOR_47(p_oper, p_args)  FB_PP_FOR_46(p_oper, (p_oper p_args))
#define FB_PP_FOR_48(p_oper, p_args)  FB_PP_FOR_47(p_oper, (p_oper p_args))
#define FB_PP_FOR_49(p_oper, p_args)  FB_PP_FOR_48(p_oper, (p_oper p_args))
#define FB_PP_FOR_50(p_oper, p_args)  FB_PP_FOR_49(p_oper, (p_oper p_args))
#define FB_PP_FOR_51(p_oper, p_args)  FB_PP_FOR_50(p_oper, (p_oper p_args))
#define FB_PP_FOR_52(p_oper, p_args)  FB_PP_FOR_51(p_oper, (p_oper p_args))
#define FB_PP_FOR_53(p_oper, p_args)  FB_PP_FOR_52(p_oper, (p_oper p_args))
#define FB_PP_FOR_54(p_oper, p_args)  FB_PP_FOR_53(p_oper, (p_oper p_args))
#define FB_PP_FOR_55(p_oper, p_args)  FB_PP_FOR_54(p_oper, (p_oper p_args))
#define FB_PP_FOR_56(p_oper, p_args)  FB_PP_FOR_55(p_oper, (p_oper p_args))
#define FB_PP_FOR_57(p_oper, p_args)  FB_PP_FOR_56(p_oper, (p_oper p_args))
#define FB_PP_FOR_58(p_oper, p_args)  FB_PP_FOR_57(p_oper, (p_oper p_args))
#define FB_PP_FOR_59(p_oper, p_args)  FB_PP_FOR_58(p_oper, (p_oper p_args))
#define FB_PP_FOR_60(p_oper, p_args)  FB_PP_FOR_59(p_oper, (p_oper p_args))
#define FB_PP_FOR_61(p_oper, p_args)  FB_PP_FOR_60(p_oper, (p_oper p_args))
#define FB_PP_FOR_62(p_oper, p_args)  FB_PP_FOR_61(p_oper, (p_oper p_args))
#define FB_PP_FOR_63(p_oper, p_args)  FB_PP_FOR_62(p_oper, (p_oper p_args))
#define FB_PP_FOR_64(p_oper, p_args)  FB_PP_FOR_63(p_oper, (p_oper p_args))
#define FB_PP_FOR_65(p_oper, p_args)  FB_PP_FOR_64(p_oper, (p_oper p_args))
#define FB_PP_FOR_66(p_oper, p_args)  FB_PP_FOR_65(p_oper, (p_oper p_args))
#define FB_PP_FOR_67(p_oper, p_args)  FB_PP_FOR_66(p_oper, (p_oper p_args))
#define FB_PP_FOR_68(p_oper, p_args)  FB_PP_FOR_67(p_oper, (p_oper p_args))
#define FB_PP_FOR_69(p_oper, p_args)  FB_PP_FOR_68(p_oper, (p_oper p_args))
#define FB_PP_FOR_70(p_oper, p_args)  FB_PP_FOR_69(p_oper, (p_oper p_args))
#define FB_PP_FOR_71(p_oper, p_args)  FB_PP_FOR_70(p_oper, (p_oper p_args))
#define FB_PP_FOR_72(p_oper, p_args)  FB_PP_FOR_71(p_oper, (p_oper p_args))
#define FB_PP_FOR_73(p_oper, p_args)  FB_PP_FOR_72(p_oper, (p_oper p_args))
#define FB_PP_FOR_74(p_oper, p_args)  FB_PP_FOR_73(p_oper, (p_oper p_args))
#define FB_PP_FOR_75(p_oper, p_args)  FB_PP_FOR_74(p_oper, (p_oper p_args))
#define FB_PP_FOR_76(p_oper, p_args)  FB_PP_FOR_75(p_oper, (p_oper p_args))
#define FB_PP_FOR_77(p_oper, p_args)  FB_PP_FOR_76(p_oper, (p_oper p_args))
#define FB_PP_FOR_78(p_oper, p_args)  FB_PP_FOR_77(p_oper, (p_oper p_args))
#define FB_PP_FOR_79(p_oper, p_args)  FB_PP_FOR_78(p_oper, (p_oper p_args))
#define FB_PP_FOR_80(p_oper, p_args)  FB_PP_FOR_79(p_oper, (p_oper p_args))
#define FB_PP_FOR_81(p_oper, p_args)  FB_PP_FOR_80(p_oper, (p_oper p_args))
#define FB_PP_FOR_82(p_oper, p_args)  FB_PP_FOR_81(p_oper, (p_oper p_args))
#define FB_PP_FOR_83(p_oper, p_args)  FB_PP_FOR_82(p_oper, (p_oper p_args))
#define FB_PP_FOR_84(p_oper, p_args)  FB_PP_FOR_83(p_oper, (p_oper p_args))
#define FB_PP_FOR_85(p_oper, p_args)  FB_PP_FOR_84(p_oper, (p_oper p_args))
#define FB_PP_FOR_86(p_oper, p_args)  FB_PP_FOR_85(p_oper, (p_oper p_args))
#define FB_PP_FOR_87(p_oper, p_args)  FB_PP_FOR_86(p_oper, (p_oper p_args))
#define FB_PP_FOR_88(p_oper, p_args)  FB_PP_FOR_87(p_oper, (p_oper p_args))
#define FB_PP_FOR_89(p_oper, p_args)  FB_PP_FOR_88(p_oper, (p_oper p_args))
#define FB_PP_FOR_90(p_oper, p_args)  FB_PP_FOR_89(p_oper, (p_oper p_args))
#define FB_PP_FOR_91(p_oper, p_args)  FB_PP_FOR_90(p_oper, (p_oper p_args))
#define FB_PP_FOR_92(p_oper, p_args)  FB_PP_FOR_91(p_oper, (p_oper p_args))
#define FB_PP_FOR_93(p_oper, p_args)  FB_PP_FOR_92(p_oper, (p_oper p_args))
#define FB_PP_FOR_94(p_oper, p_args)  FB_PP_FOR_93(p_oper, (p_oper p_args))
#define FB_PP_FOR_95(p_oper, p_args)  FB_PP_FOR_94(p_oper, (p_oper p_args))
#define FB_PP_FOR_96(p_oper, p_args)  FB_PP_FOR_95(p_oper, (p_oper p_args))
#define FB_PP_FOR_97(p_oper, p_args)  FB_PP_FOR_96(p_oper, (p_oper p_args))
#define FB_PP_FOR_98(p_oper, p_args)  FB_PP_FOR_97(p_oper, (p_oper p_args))
#define FB_PP_FOR_99(p_oper, p_args)  FB_PP_FOR_98(p_oper, (p_oper p_args))
#define FB_PP_FOR_100(p_oper, p_args) FB_PP_FOR_99(p_oper, (p_oper p_args))
#define FB_PP_FOR_101(p_oper, p_args) FB_PP_FOR_100(p_oper, (p_oper p_args))
#define FB_PP_FOR_102(p_oper, p_args) FB_PP_FOR_101(p_oper, (p_oper p_args))
#define FB_PP_FOR_103(p_oper, p_args) FB_PP_FOR_102(p_oper, (p_oper p_args))
#define FB_PP_FOR_104(p_oper, p_args) FB_PP_FOR_103(p_oper, (p_oper p_args))
#define FB_PP_FOR_105(p_oper, p_args) FB_PP_FOR_104(p_oper, (p_oper p_args))
#define FB_PP_FOR_106(p_oper, p_args) FB_PP_FOR_105(p_oper, (p_oper p_args))
#define FB_PP_FOR_107(p_oper, p_args) FB_PP_FOR_106(p_oper, (p_oper p_args))
#define FB_PP_FOR_108(p_oper, p_args) FB_PP_FOR_107(p_oper, (p_oper p_args))
#define FB_PP_FOR_109(p_oper, p_args) FB_PP_FOR_108(p_oper, (p_oper p_args))
#define FB_PP_FOR_110(p_oper, p_args) FB_PP_FOR_109(p_oper, (p_oper p_args))
#define FB_PP_FOR_111(p_oper, p_args) FB_PP_FOR_110(p_oper, (p_oper p_args))
#define FB_PP_FOR_112(p_oper, p_args) FB_PP_FOR_111(p_oper, (p_oper p_args))
#define FB_PP_FOR_113(p_oper, p_args) FB_PP_FOR_112(p_oper, (p_oper p_args))
#define FB_PP_FOR_114(p_oper, p_args) FB_PP_FOR_113(p_oper, (p_oper p_args))
#define FB_PP_FOR_115(p_oper, p_args) FB_PP_FOR_114(p_oper, (p_oper p_args))
#define FB_PP_FOR_116(p_oper, p_args) FB_PP_FOR_115(p_oper, (p_oper p_args))
#define FB_PP_FOR_117(p_oper, p_args) FB_PP_FOR_116(p_oper, (p_oper p_args))
#define FB_PP_FOR_118(p_oper, p_args) FB_PP_FOR_117(p_oper, (p_oper p_args))
#define FB_PP_FOR_119(p_oper, p_args) FB_PP_FOR_118(p_oper, (p_oper p_args))
#define FB_PP_FOR_120(p_oper, p_args) FB_PP_FOR_119(p_oper, (p_oper p_args))
#define FB_PP_FOR_121(p_oper, p_args) FB_PP_FOR_120(p_oper, (p_oper p_args))
#define FB_PP_FOR_122(p_oper, p_args) FB_PP_FOR_121(p_oper, (p_oper p_args))
#define FB_PP_FOR_123(p_oper, p_args) FB_PP_FOR_122(p_oper, (p_oper p_args))
#define FB_PP_FOR_124(p_oper, p_args) FB_PP_FOR_123(p_oper, (p_oper p_args))
#define FB_PP_FOR_125(p_oper, p_args) FB_PP_FOR_124(p_oper, (p_oper p_args))
#define FB_PP_FOR_126(p_oper, p_args) FB_PP_FOR_125(p_oper, (p_oper p_args))
#define FB_PP_FOR_127(p_oper, p_args) FB_PP_FOR_126(p_oper, (p_oper p_args))
#define FB_PP_FOR_128(p_oper, p_args) FB_PP_FOR_127(p_oper, (p_oper p_args))
#define FB_PP_FOR_129(p_oper, p_args) FB_PP_FOR_128(p_oper, (p_oper p_args))
#define FB_PP_FOR_130(p_oper, p_args) FB_PP_FOR_129(p_oper, (p_oper p_args))
#define FB_PP_FOR_131(p_oper, p_args) FB_PP_FOR_130(p_oper, (p_oper p_args))
#define FB_PP_FOR_132(p_oper, p_args) FB_PP_FOR_131(p_oper, (p_oper p_args))
#define FB_PP_FOR_133(p_oper, p_args) FB_PP_FOR_132(p_oper, (p_oper p_args))
#define FB_PP_FOR_134(p_oper, p_args) FB_PP_FOR_133(p_oper, (p_oper p_args))
#define FB_PP_FOR_135(p_oper, p_args) FB_PP_FOR_134(p_oper, (p_oper p_args))
#define FB_PP_FOR_136(p_oper, p_args) FB_PP_FOR_135(p_oper, (p_oper p_args))
#define FB_PP_FOR_137(p_oper, p_args) FB_PP_FOR_136(p_oper, (p_oper p_args))
#define FB_PP_FOR_138(p_oper, p_args) FB_PP_FOR_137(p_oper, (p_oper p_args))
#define FB_PP_FOR_139(p_oper, p_args) FB_PP_FOR_138(p_oper, (p_oper p_args))
#define FB_PP_FOR_140(p_oper, p_args) FB_PP_FOR_139(p_oper, (p_oper p_args))
#define FB_PP_FOR_141(p_oper, p_args) FB_PP_FOR_140(p_oper, (p_oper p_args))
#define FB_PP_FOR_142(p_oper, p_args) FB_PP_FOR_141(p_oper, (p_oper p_args))
#define FB_PP_FOR_143(p_oper, p_args) FB_PP_FOR_142(p_oper, (p_oper p_args))
#define FB_PP_FOR_144(p_oper, p_args) FB_PP_FOR_143(p_oper, (p_oper p_args))
#define FB_PP_FOR_145(p_oper, p_args) FB_PP_FOR_144(p_oper, (p_oper p_args))
#define FB_PP_FOR_146(p_oper, p_args) FB_PP_FOR_145(p_oper, (p_oper p_args))
#define FB_PP_FOR_147(p_oper, p_args) FB_PP_FOR_146(p_oper, (p_oper p_args))
#define FB_PP_FOR_148(p_oper, p_args) FB_PP_FOR_147(p_oper, (p_oper p_args))
#define FB_PP_FOR_149(p_oper, p_args) FB_PP_FOR_148(p_oper, (p_oper p_args))
#define FB_PP_FOR_150(p_oper, p_args) FB_PP_FOR_149(p_oper, (p_oper p_args))
#define FB_PP_FOR_151(p_oper, p_args) FB_PP_FOR_150(p_oper, (p_oper p_args))
#define FB_PP_FOR_152(p_oper, p_args) FB_PP_FOR_151(p_oper, (p_oper p_args))
#define FB_PP_FOR_153(p_oper, p_args) FB_PP_FOR_152(p_oper, (p_oper p_args))
#define FB_PP_FOR_154(p_oper, p_args) FB_PP_FOR_153(p_oper, (p_oper p_args))
#define FB_PP_FOR_155(p_oper, p_args) FB_PP_FOR_154(p_oper, (p_oper p_args))
#define FB_PP_FOR_156(p_oper, p_args) FB_PP_FOR_155(p_oper, (p_oper p_args))
#define FB_PP_FOR_157(p_oper, p_args) FB_PP_FOR_156(p_oper, (p_oper p_args))
#define FB_PP_FOR_158(p_oper, p_args) FB_PP_FOR_157(p_oper, (p_oper p_args))
#define FB_PP_FOR_159(p_oper, p_args) FB_PP_FOR_158(p_oper, (p_oper p_args))
#define FB_PP_FOR_160(p_oper, p_args) FB_PP_FOR_159(p_oper, (p_oper p_args))
#define FB_PP_FOR_161(p_oper, p_args) FB_PP_FOR_160(p_oper, (p_oper p_args))
#define FB_PP_FOR_162(p_oper, p_args) FB_PP_FOR_161(p_oper, (p_oper p_args))
#define FB_PP_FOR_163(p_oper, p_args) FB_PP_FOR_162(p_oper, (p_oper p_args))
#define FB_PP_FOR_164(p_oper, p_args) FB_PP_FOR_163(p_oper, (p_oper p_args))
#define FB_PP_FOR_165(p_oper, p_args) FB_PP_FOR_164(p_oper, (p_oper p_args))
#define FB_PP_FOR_166(p_oper, p_args) FB_PP_FOR_165(p_oper, (p_oper p_args))
#define FB_PP_FOR_167(p_oper, p_args) FB_PP_FOR_166(p_oper, (p_oper p_args))
#define FB_PP_FOR_168(p_oper, p_args) FB_PP_FOR_167(p_oper, (p_oper p_args))
#define FB_PP_FOR_169(p_oper, p_args) FB_PP_FOR_168(p_oper, (p_oper p_args))
#define FB_PP_FOR_170(p_oper, p_args) FB_PP_FOR_169(p_oper, (p_oper p_args))
#define FB_PP_FOR_171(p_oper, p_args) FB_PP_FOR_170(p_oper, (p_oper p_args))
#define FB_PP_FOR_172(p_oper, p_args) FB_PP_FOR_171(p_oper, (p_oper p_args))
#define FB_PP_FOR_173(p_oper, p_args) FB_PP_FOR_172(p_oper, (p_oper p_args))
#define FB_PP_FOR_174(p_oper, p_args) FB_PP_FOR_173(p_oper, (p_oper p_args))
#define FB_PP_FOR_175(p_oper, p_args) FB_PP_FOR_174(p_oper, (p_oper p_args))
#define FB_PP_FOR_176(p_oper, p_args) FB_PP_FOR_175(p_oper, (p_oper p_args))
#define FB_PP_FOR_177(p_oper, p_args) FB_PP_FOR_176(p_oper, (p_oper p_args))
#define FB_PP_FOR_178(p_oper, p_args) FB_PP_FOR_177(p_oper, (p_oper p_args))
#define FB_PP_FOR_179(p_oper, p_args) FB_PP_FOR_178(p_oper, (p_oper p_args))
#define FB_PP_FOR_180(p_oper, p_args) FB_PP_FOR_179(p_oper, (p_oper p_args))
#define FB_PP_FOR_181(p_oper, p_args) FB_PP_FOR_180(p_oper, (p_oper p_args))
#define FB_PP_FOR_182(p_oper, p_args) FB_PP_FOR_181(p_oper, (p_oper p_args))
#define FB_PP_FOR_183(p_oper, p_args) FB_PP_FOR_182(p_oper, (p_oper p_args))
#define FB_PP_FOR_184(p_oper, p_args) FB_PP_FOR_183(p_oper, (p_oper p_args))
#define FB_PP_FOR_185(p_oper, p_args) FB_PP_FOR_184(p_oper, (p_oper p_args))
#define FB_PP_FOR_186(p_oper, p_args) FB_PP_FOR_185(p_oper, (p_oper p_args))
#define FB_PP_FOR_187(p_oper, p_args) FB_PP_FOR_186(p_oper, (p_oper p_args))
#define FB_PP_FOR_188(p_oper, p_args) FB_PP_FOR_187(p_oper, (p_oper p_args))
#define FB_PP_FOR_189(p_oper, p_args) FB_PP_FOR_188(p_oper, (p_oper p_args))
#define FB_PP_FOR_190(p_oper, p_args) FB_PP_FOR_189(p_oper, (p_oper p_args))
#define FB_PP_FOR_191(p_oper, p_args) FB_PP_FOR_190(p_oper, (p_oper p_args))
#define FB_PP_FOR_192(p_oper, p_args) FB_PP_FOR_191(p_oper, (p_oper p_args))
#define FB_PP_FOR_193(p_oper, p_args) FB_PP_FOR_192(p_oper, (p_oper p_args))
#define FB_PP_FOR_194(p_oper, p_args) FB_PP_FOR_193(p_oper, (p_oper p_args))
#define FB_PP_FOR_195(p_oper, p_args) FB_PP_FOR_194(p_oper, (p_oper p_args))
#define FB_PP_FOR_196(p_oper, p_args) FB_PP_FOR_195(p_oper, (p_oper p_args))
#define FB_PP_FOR_197(p_oper, p_args) FB_PP_FOR_196(p_oper, (p_oper p_args))
#define FB_PP_FOR_198(p_oper, p_args) FB_PP_FOR_197(p_oper, (p_oper p_args))
#define FB_PP_FOR_199(p_oper, p_args) FB_PP_FOR_198(p_oper, (p_oper p_args))
#define FB_PP_FOR_200(p_oper, p_args) FB_PP_FOR_199(p_oper, (p_oper p_args))
#define FB_PP_FOR_201(p_oper, p_args) FB_PP_FOR_200(p_oper, (p_oper p_args))
#define FB_PP_FOR_202(p_oper, p_args) FB_PP_FOR_201(p_oper, (p_oper p_args))
#define FB_PP_FOR_203(p_oper, p_args) FB_PP_FOR_202(p_oper, (p_oper p_args))
#define FB_PP_FOR_204(p_oper, p_args) FB_PP_FOR_203(p_oper, (p_oper p_args))
#define FB_PP_FOR_205(p_oper, p_args) FB_PP_FOR_204(p_oper, (p_oper p_args))
#define FB_PP_FOR_206(p_oper, p_args) FB_PP_FOR_205(p_oper, (p_oper p_args))
#define FB_PP_FOR_207(p_oper, p_args) FB_PP_FOR_206(p_oper, (p_oper p_args))
#define FB_PP_FOR_208(p_oper, p_args) FB_PP_FOR_207(p_oper, (p_oper p_args))
#define FB_PP_FOR_209(p_oper, p_args) FB_PP_FOR_208(p_oper, (p_oper p_args))
#define FB_PP_FOR_210(p_oper, p_args) FB_PP_FOR_209(p_oper, (p_oper p_args))
#define FB_PP_FOR_211(p_oper, p_args) FB_PP_FOR_210(p_oper, (p_oper p_args))
#define FB_PP_FOR_212(p_oper, p_args) FB_PP_FOR_211(p_oper, (p_oper p_args))
#define FB_PP_FOR_213(p_oper, p_args) FB_PP_FOR_212(p_oper, (p_oper p_args))
#define FB_PP_FOR_214(p_oper, p_args) FB_PP_FOR_213(p_oper, (p_oper p_args))
#define FB_PP_FOR_215(p_oper, p_args) FB_PP_FOR_214(p_oper, (p_oper p_args))
#define FB_PP_FOR_216(p_oper, p_args) FB_PP_FOR_215(p_oper, (p_oper p_args))
#define FB_PP_FOR_217(p_oper, p_args) FB_PP_FOR_216(p_oper, (p_oper p_args))
#define FB_PP_FOR_218(p_oper, p_args) FB_PP_FOR_217(p_oper, (p_oper p_args))
#define FB_PP_FOR_219(p_oper, p_args) FB_PP_FOR_218(p_oper, (p_oper p_args))
#define FB_PP_FOR_220(p_oper, p_args) FB_PP_FOR_219(p_oper, (p_oper p_args))
#define FB_PP_FOR_221(p_oper, p_args) FB_PP_FOR_220(p_oper, (p_oper p_args))
#define FB_PP_FOR_222(p_oper, p_args) FB_PP_FOR_221(p_oper, (p_oper p_args))
#define FB_PP_FOR_223(p_oper, p_args) FB_PP_FOR_222(p_oper, (p_oper p_args))
#define FB_PP_FOR_224(p_oper, p_args) FB_PP_FOR_223(p_oper, (p_oper p_args))
#define FB_PP_FOR_225(p_oper, p_args) FB_PP_FOR_224(p_oper, (p_oper p_args))
#define FB_PP_FOR_226(p_oper, p_args) FB_PP_FOR_225(p_oper, (p_oper p_args))
#define FB_PP_FOR_227(p_oper, p_args) FB_PP_FOR_226(p_oper, (p_oper p_args))
#define FB_PP_FOR_228(p_oper, p_args) FB_PP_FOR_227(p_oper, (p_oper p_args))
#define FB_PP_FOR_229(p_oper, p_args) FB_PP_FOR_228(p_oper, (p_oper p_args))
#define FB_PP_FOR_230(p_oper, p_args) FB_PP_FOR_229(p_oper, (p_oper p_args))
#define FB_PP_FOR_231(p_oper, p_args) FB_PP_FOR_230(p_oper, (p_oper p_args))
#define FB_PP_FOR_232(p_oper, p_args) FB_PP_FOR_231(p_oper, (p_oper p_args))
#define FB_PP_FOR_233(p_oper, p_args) FB_PP_FOR_232(p_oper, (p_oper p_args))
#define FB_PP_FOR_234(p_oper, p_args) FB_PP_FOR_233(p_oper, (p_oper p_args))
#define FB_PP_FOR_235(p_oper, p_args) FB_PP_FOR_234(p_oper, (p_oper p_args))
#define FB_PP_FOR_236(p_oper, p_args) FB_PP_FOR_235(p_oper, (p_oper p_args))
#define FB_PP_FOR_237(p_oper, p_args) FB_PP_FOR_236(p_oper, (p_oper p_args))
#define FB_PP_FOR_238(p_oper, p_args) FB_PP_FOR_237(p_oper, (p_oper p_args))
#define FB_PP_FOR_239(p_oper, p_args) FB_PP_FOR_238(p_oper, (p_oper p_args))
#define FB_PP_FOR_240(p_oper, p_args) FB_PP_FOR_239(p_oper, (p_oper p_args))
#define FB_PP_FOR_241(p_oper, p_args) FB_PP_FOR_240(p_oper, (p_oper p_args))
#define FB_PP_FOR_242(p_oper, p_args) FB_PP_FOR_241(p_oper, (p_oper p_args))
#define FB_PP_FOR_243(p_oper, p_args) FB_PP_FOR_242(p_oper, (p_oper p_args))
#define FB_PP_FOR_244(p_oper, p_args) FB_PP_FOR_243(p_oper, (p_oper p_args))
#define FB_PP_FOR_245(p_oper, p_args) FB_PP_FOR_244(p_oper, (p_oper p_args))
#define FB_PP_FOR_246(p_oper, p_args) FB_PP_FOR_245(p_oper, (p_oper p_args))
#define FB_PP_FOR_247(p_oper, p_args) FB_PP_FOR_246(p_oper, (p_oper p_args))
#define FB_PP_FOR_248(p_oper, p_args) FB_PP_FOR_247(p_oper, (p_oper p_args))
#define FB_PP_FOR_249(p_oper, p_args) FB_PP_FOR_248(p_oper, (p_oper p_args))
#define FB_PP_FOR_250(p_oper, p_args) FB_PP_FOR_249(p_oper, (p_oper p_args))
#define FB_PP_FOR_251(p_oper, p_args) FB_PP_FOR_250(p_oper, (p_oper p_args))
#define FB_PP_FOR_252(p_oper, p_args) FB_PP_FOR_251(p_oper, (p_oper p_args))
#define FB_PP_FOR_253(p_oper, p_args) FB_PP_FOR_252(p_oper, (p_oper p_args))
#define FB_PP_FOR_254(p_oper, p_args) FB_PP_FOR_253(p_oper, (p_oper p_args))
#define FB_PP_FOR_255(p_oper, p_args) FB_PP_FOR_254(p_oper, (p_oper p_args))

// For loop
#define FB_PP_FOR_WRAPPER8(p_a, p_b) p_a p_b
#define FB_PP_FOR_WRAPPER7(p_a, p_b) FB_PP_FOR_WRAPPER8(p_a, p_b)
#define FB_PP_FOR_WRAPPER6(p_a, p_b) FB_PP_FOR_WRAPPER7(p_a, p_b)
#define FB_PP_FOR_WRAPPER5(p_a, p_b) FB_PP_FOR_WRAPPER6(p_a, p_b)
#define FB_PP_FOR_WRAPPER4(p_a, p_b) FB_PP_FOR_WRAPPER5(p_a, p_b)
#define FB_PP_FOR_WRAPPER3(p_a, p_b) FB_PP_FOR_WRAPPER4(p_a, p_b)
#define FB_PP_FOR_WRAPPER2(p_a, p_b) FB_PP_FOR_WRAPPER3(p_a, p_b)
#define FB_PP_FOR_WRAPPER1(p_a, p_b) FB_PP_FOR_WRAPPER2(p_a, p_b)
#define FB_PP_FOR(p_count, p_oper, p_args) \
	FB_PP_FOR_WRAPPER1(FB_PP_CONCAT(FB_PP_FOR_, p_count), (p_oper, p_args))


// Foreach loop
#define FB_PP_FOREACH_OPER_DONE(p_count, p_oper, p_input, p_output) FB_PP_UNPACK(FB_PP_POP_FIRST(p_output))
#define FB_PP_FOREACH_OPER_LOOP(p_count, p_oper, p_input, p_output) p_count, p_oper, p_input, p_output
#define FB_PP_FOREACH_OPER(p_count, p_oper, p_input, p_output) \
	FB_PP_CONCAT(FB_PP_FOREACH_OPER_, FB_PP_LOOP_STATE(p_count)) (FB_PP_DECREASE(p_count), p_oper, FB_PP_POP_FIRST(p_input), (FB_PP_UNPACK(p_output), p_oper(FB_PP_FIRST(p_input))))

#define FB_PP_FOREACH_WRAPPER3(p_oper, p_input) FB_PP_FOR(FB_PP_NARG p_input, FB_PP_FOREACH_OPER, (FB_PP_NARG p_input, p_oper, (FB_PP_UNPACK(p_input), input), (output)))
#define FB_PP_FOREACH_WRAPPER2(p_oper, p_input) FB_PP_FOREACH_WRAPPER3(p_oper, p_input)
#define FB_PP_FOREACH_WRAPPER1(p_oper, p_input) FB_PP_FOREACH_WRAPPER2(p_oper, p_input)
#define FB_PP_FOREACH(p_oper, ...) \
	FB_PP_FOREACH_WRAPPER1(p_oper, (__VA_ARGS__))


// Foreach loop with count
#define FB_PP_FOREACH_WITH_COUNT_OPER_DONE(p_count, p_oper, p_input, p_output) FB_PP_UNPACK(FB_PP_POP_FIRST(p_output))
#define FB_PP_FOREACH_WITH_COUNT_OPER_LOOP(p_count, p_oper, p_input, p_output) p_count, p_oper, p_input, p_output
#define FB_PP_FOREACH_WITH_COUNT_OPER(p_count, p_oper, p_input, p_output) \
	FB_PP_CONCAT(FB_PP_FOREACH_WITH_COUNT_OPER_, FB_PP_LOOP_STATE(p_count)) (FB_PP_DECREASE(p_count), p_oper, FB_PP_POP_FIRST(p_input), (FB_PP_UNPACK(p_output), p_oper(FB_PP_DECREASE(p_count), FB_PP_FIRST(p_input))))

#define FB_PP_FOREACH_WITH_COUNT_WRAPPER3(p_oper, p_input) FB_PP_FOR(FB_PP_NARG p_input, FB_PP_FOREACH_WITH_COUNT_OPER, (FB_PP_NARG p_input, p_oper, (FB_PP_UNPACK(p_input), input), (output)))
#define FB_PP_FOREACH_WITH_COUNT_WRAPPER2(p_oper, p_input) FB_PP_FOREACH_WITH_COUNT_WRAPPER3(p_oper, p_input)
#define FB_PP_FOREACH_WITH_COUNT_WRAPPER1(p_oper, p_input) FB_PP_FOREACH_WITH_COUNT_WRAPPER2(p_oper, p_input)
#define FB_PP_FOREACH_WITH_COUNT(p_oper, ...) \
	FB_PP_FOREACH_WITH_COUNT_WRAPPER1(p_oper, (__VA_ARGS__))


// Foreach pair loop (item count must be even)
#define FB_PP_FOREACH_PAIR_OPER_DONE(p_count, p_oper, p_input, p_output) FB_PP_UNPACK(FB_PP_POP_FIRST(p_output))
#define FB_PP_FOREACH_PAIR_OPER_LOOP(p_count, p_oper, p_input, p_output) p_count, p_oper, p_input, p_output
#define FB_PP_FOREACH_PAIR_OPER(p_count, p_oper, p_input, p_output) \
	FB_PP_CONCAT(FB_PP_FOREACH_PAIR_OPER_, FB_PP_LOOP_STATE(p_count)) (FB_PP_DECREASE(p_count), p_oper, FB_PP_POP_FIRST_TWO(p_input), (FB_PP_UNPACK(p_output), p_oper(FB_PP_FIRST(p_input), FB_PP_SECOND(p_input))))

#define FB_PP_FOREACH_PAIR_WRAPPER5(p_oper, p_input) FB_PP_FOR(FB_PP_HALVE(FB_PP_NARG p_input), FB_PP_FOREACH_PAIR_OPER, (FB_PP_HALVE(FB_PP_NARG p_input), p_oper, (FB_PP_UNPACK(p_input), input), (output)))
#define FB_PP_FOREACH_PAIR_WRAPPER4(p_oper, p_input) FB_PP_FOREACH_PAIR_WRAPPER5(p_oper, p_input)
#define FB_PP_FOREACH_PAIR_WRAPPER3(p_oper, p_input) FB_PP_FOREACH_PAIR_WRAPPER4(p_oper, p_input)
#define FB_PP_FOREACH_PAIR_WRAPPER2(p_oper, p_input) FB_PP_FOREACH_PAIR_WRAPPER3(p_oper, p_input)
#define FB_PP_FOREACH_PAIR_WRAPPER1(p_oper, p_input) FB_PP_FOREACH_PAIR_WRAPPER2(p_oper, p_input)
#define FB_PP_FOREACH_PAIR(p_oper, ...) \
	FB_PP_FOREACH_PAIR_WRAPPER1(p_oper, (__VA_ARGS__))


// Foreach pair, but ignore last item (item count must be odd)
#define FB_PP_FOREACH_PAIR_IGNORE_LAST_OPER(p_count, p_oper, p_input, p_output) \
	FB_PP_CONCAT(FB_PP_FOREACH_PAIR_OPER_, FB_PP_LOOP_STATE(p_count)) (FB_PP_DECREASE(p_count), p_oper, FB_PP_POP_FIRST_TWO(p_input), (FB_PP_UNPACK(p_output), p_oper(FB_PP_FIRST(p_input), FB_PP_SECOND(p_input))))

#define FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER5(p_oper, p_input) FB_PP_FOR(FB_PP_HALVE(FB_PP_DECREASE(FB_PP_NARG p_input)), FB_PP_FOREACH_PAIR_IGNORE_LAST_OPER, (FB_PP_HALVE(FB_PP_DECREASE(FB_PP_NARG p_input)), p_oper, (FB_PP_UNPACK(p_input), input), (output)))
#define FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER4(p_oper, p_input) FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER5(p_oper, p_input)
#define FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER3(p_oper, p_input) FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER4(p_oper, p_input)
#define FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER2(p_oper, p_input) FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER3(p_oper, p_input)
#define FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER1(p_oper, p_input) FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER2(p_oper, p_input)

#define FB_PP_FOREACH_PAIR_IGNORE_LAST(p_oper, ...) \
	FB_PP_FOREACH_PAIR_IGNORE_LAST_WRAPPER1(p_oper, (__VA_ARGS__))

