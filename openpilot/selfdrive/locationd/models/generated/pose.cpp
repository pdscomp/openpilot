#include "pose.h"

namespace {
#define DIM 18
#define EDIM 18
#define MEDIM 18
typedef void (*Hfun)(double *, double *, double *);
const static double MAHA_THRESH_4 = 7.814727903251177;
const static double MAHA_THRESH_10 = 7.814727903251177;
const static double MAHA_THRESH_13 = 7.814727903251177;
const static double MAHA_THRESH_14 = 7.814727903251177;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_3549856774171141684) {
   out_3549856774171141684[0] = delta_x[0] + nom_x[0];
   out_3549856774171141684[1] = delta_x[1] + nom_x[1];
   out_3549856774171141684[2] = delta_x[2] + nom_x[2];
   out_3549856774171141684[3] = delta_x[3] + nom_x[3];
   out_3549856774171141684[4] = delta_x[4] + nom_x[4];
   out_3549856774171141684[5] = delta_x[5] + nom_x[5];
   out_3549856774171141684[6] = delta_x[6] + nom_x[6];
   out_3549856774171141684[7] = delta_x[7] + nom_x[7];
   out_3549856774171141684[8] = delta_x[8] + nom_x[8];
   out_3549856774171141684[9] = delta_x[9] + nom_x[9];
   out_3549856774171141684[10] = delta_x[10] + nom_x[10];
   out_3549856774171141684[11] = delta_x[11] + nom_x[11];
   out_3549856774171141684[12] = delta_x[12] + nom_x[12];
   out_3549856774171141684[13] = delta_x[13] + nom_x[13];
   out_3549856774171141684[14] = delta_x[14] + nom_x[14];
   out_3549856774171141684[15] = delta_x[15] + nom_x[15];
   out_3549856774171141684[16] = delta_x[16] + nom_x[16];
   out_3549856774171141684[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2925455247541166295) {
   out_2925455247541166295[0] = -nom_x[0] + true_x[0];
   out_2925455247541166295[1] = -nom_x[1] + true_x[1];
   out_2925455247541166295[2] = -nom_x[2] + true_x[2];
   out_2925455247541166295[3] = -nom_x[3] + true_x[3];
   out_2925455247541166295[4] = -nom_x[4] + true_x[4];
   out_2925455247541166295[5] = -nom_x[5] + true_x[5];
   out_2925455247541166295[6] = -nom_x[6] + true_x[6];
   out_2925455247541166295[7] = -nom_x[7] + true_x[7];
   out_2925455247541166295[8] = -nom_x[8] + true_x[8];
   out_2925455247541166295[9] = -nom_x[9] + true_x[9];
   out_2925455247541166295[10] = -nom_x[10] + true_x[10];
   out_2925455247541166295[11] = -nom_x[11] + true_x[11];
   out_2925455247541166295[12] = -nom_x[12] + true_x[12];
   out_2925455247541166295[13] = -nom_x[13] + true_x[13];
   out_2925455247541166295[14] = -nom_x[14] + true_x[14];
   out_2925455247541166295[15] = -nom_x[15] + true_x[15];
   out_2925455247541166295[16] = -nom_x[16] + true_x[16];
   out_2925455247541166295[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_210273699827536735) {
   out_210273699827536735[0] = 1.0;
   out_210273699827536735[1] = 0.0;
   out_210273699827536735[2] = 0.0;
   out_210273699827536735[3] = 0.0;
   out_210273699827536735[4] = 0.0;
   out_210273699827536735[5] = 0.0;
   out_210273699827536735[6] = 0.0;
   out_210273699827536735[7] = 0.0;
   out_210273699827536735[8] = 0.0;
   out_210273699827536735[9] = 0.0;
   out_210273699827536735[10] = 0.0;
   out_210273699827536735[11] = 0.0;
   out_210273699827536735[12] = 0.0;
   out_210273699827536735[13] = 0.0;
   out_210273699827536735[14] = 0.0;
   out_210273699827536735[15] = 0.0;
   out_210273699827536735[16] = 0.0;
   out_210273699827536735[17] = 0.0;
   out_210273699827536735[18] = 0.0;
   out_210273699827536735[19] = 1.0;
   out_210273699827536735[20] = 0.0;
   out_210273699827536735[21] = 0.0;
   out_210273699827536735[22] = 0.0;
   out_210273699827536735[23] = 0.0;
   out_210273699827536735[24] = 0.0;
   out_210273699827536735[25] = 0.0;
   out_210273699827536735[26] = 0.0;
   out_210273699827536735[27] = 0.0;
   out_210273699827536735[28] = 0.0;
   out_210273699827536735[29] = 0.0;
   out_210273699827536735[30] = 0.0;
   out_210273699827536735[31] = 0.0;
   out_210273699827536735[32] = 0.0;
   out_210273699827536735[33] = 0.0;
   out_210273699827536735[34] = 0.0;
   out_210273699827536735[35] = 0.0;
   out_210273699827536735[36] = 0.0;
   out_210273699827536735[37] = 0.0;
   out_210273699827536735[38] = 1.0;
   out_210273699827536735[39] = 0.0;
   out_210273699827536735[40] = 0.0;
   out_210273699827536735[41] = 0.0;
   out_210273699827536735[42] = 0.0;
   out_210273699827536735[43] = 0.0;
   out_210273699827536735[44] = 0.0;
   out_210273699827536735[45] = 0.0;
   out_210273699827536735[46] = 0.0;
   out_210273699827536735[47] = 0.0;
   out_210273699827536735[48] = 0.0;
   out_210273699827536735[49] = 0.0;
   out_210273699827536735[50] = 0.0;
   out_210273699827536735[51] = 0.0;
   out_210273699827536735[52] = 0.0;
   out_210273699827536735[53] = 0.0;
   out_210273699827536735[54] = 0.0;
   out_210273699827536735[55] = 0.0;
   out_210273699827536735[56] = 0.0;
   out_210273699827536735[57] = 1.0;
   out_210273699827536735[58] = 0.0;
   out_210273699827536735[59] = 0.0;
   out_210273699827536735[60] = 0.0;
   out_210273699827536735[61] = 0.0;
   out_210273699827536735[62] = 0.0;
   out_210273699827536735[63] = 0.0;
   out_210273699827536735[64] = 0.0;
   out_210273699827536735[65] = 0.0;
   out_210273699827536735[66] = 0.0;
   out_210273699827536735[67] = 0.0;
   out_210273699827536735[68] = 0.0;
   out_210273699827536735[69] = 0.0;
   out_210273699827536735[70] = 0.0;
   out_210273699827536735[71] = 0.0;
   out_210273699827536735[72] = 0.0;
   out_210273699827536735[73] = 0.0;
   out_210273699827536735[74] = 0.0;
   out_210273699827536735[75] = 0.0;
   out_210273699827536735[76] = 1.0;
   out_210273699827536735[77] = 0.0;
   out_210273699827536735[78] = 0.0;
   out_210273699827536735[79] = 0.0;
   out_210273699827536735[80] = 0.0;
   out_210273699827536735[81] = 0.0;
   out_210273699827536735[82] = 0.0;
   out_210273699827536735[83] = 0.0;
   out_210273699827536735[84] = 0.0;
   out_210273699827536735[85] = 0.0;
   out_210273699827536735[86] = 0.0;
   out_210273699827536735[87] = 0.0;
   out_210273699827536735[88] = 0.0;
   out_210273699827536735[89] = 0.0;
   out_210273699827536735[90] = 0.0;
   out_210273699827536735[91] = 0.0;
   out_210273699827536735[92] = 0.0;
   out_210273699827536735[93] = 0.0;
   out_210273699827536735[94] = 0.0;
   out_210273699827536735[95] = 1.0;
   out_210273699827536735[96] = 0.0;
   out_210273699827536735[97] = 0.0;
   out_210273699827536735[98] = 0.0;
   out_210273699827536735[99] = 0.0;
   out_210273699827536735[100] = 0.0;
   out_210273699827536735[101] = 0.0;
   out_210273699827536735[102] = 0.0;
   out_210273699827536735[103] = 0.0;
   out_210273699827536735[104] = 0.0;
   out_210273699827536735[105] = 0.0;
   out_210273699827536735[106] = 0.0;
   out_210273699827536735[107] = 0.0;
   out_210273699827536735[108] = 0.0;
   out_210273699827536735[109] = 0.0;
   out_210273699827536735[110] = 0.0;
   out_210273699827536735[111] = 0.0;
   out_210273699827536735[112] = 0.0;
   out_210273699827536735[113] = 0.0;
   out_210273699827536735[114] = 1.0;
   out_210273699827536735[115] = 0.0;
   out_210273699827536735[116] = 0.0;
   out_210273699827536735[117] = 0.0;
   out_210273699827536735[118] = 0.0;
   out_210273699827536735[119] = 0.0;
   out_210273699827536735[120] = 0.0;
   out_210273699827536735[121] = 0.0;
   out_210273699827536735[122] = 0.0;
   out_210273699827536735[123] = 0.0;
   out_210273699827536735[124] = 0.0;
   out_210273699827536735[125] = 0.0;
   out_210273699827536735[126] = 0.0;
   out_210273699827536735[127] = 0.0;
   out_210273699827536735[128] = 0.0;
   out_210273699827536735[129] = 0.0;
   out_210273699827536735[130] = 0.0;
   out_210273699827536735[131] = 0.0;
   out_210273699827536735[132] = 0.0;
   out_210273699827536735[133] = 1.0;
   out_210273699827536735[134] = 0.0;
   out_210273699827536735[135] = 0.0;
   out_210273699827536735[136] = 0.0;
   out_210273699827536735[137] = 0.0;
   out_210273699827536735[138] = 0.0;
   out_210273699827536735[139] = 0.0;
   out_210273699827536735[140] = 0.0;
   out_210273699827536735[141] = 0.0;
   out_210273699827536735[142] = 0.0;
   out_210273699827536735[143] = 0.0;
   out_210273699827536735[144] = 0.0;
   out_210273699827536735[145] = 0.0;
   out_210273699827536735[146] = 0.0;
   out_210273699827536735[147] = 0.0;
   out_210273699827536735[148] = 0.0;
   out_210273699827536735[149] = 0.0;
   out_210273699827536735[150] = 0.0;
   out_210273699827536735[151] = 0.0;
   out_210273699827536735[152] = 1.0;
   out_210273699827536735[153] = 0.0;
   out_210273699827536735[154] = 0.0;
   out_210273699827536735[155] = 0.0;
   out_210273699827536735[156] = 0.0;
   out_210273699827536735[157] = 0.0;
   out_210273699827536735[158] = 0.0;
   out_210273699827536735[159] = 0.0;
   out_210273699827536735[160] = 0.0;
   out_210273699827536735[161] = 0.0;
   out_210273699827536735[162] = 0.0;
   out_210273699827536735[163] = 0.0;
   out_210273699827536735[164] = 0.0;
   out_210273699827536735[165] = 0.0;
   out_210273699827536735[166] = 0.0;
   out_210273699827536735[167] = 0.0;
   out_210273699827536735[168] = 0.0;
   out_210273699827536735[169] = 0.0;
   out_210273699827536735[170] = 0.0;
   out_210273699827536735[171] = 1.0;
   out_210273699827536735[172] = 0.0;
   out_210273699827536735[173] = 0.0;
   out_210273699827536735[174] = 0.0;
   out_210273699827536735[175] = 0.0;
   out_210273699827536735[176] = 0.0;
   out_210273699827536735[177] = 0.0;
   out_210273699827536735[178] = 0.0;
   out_210273699827536735[179] = 0.0;
   out_210273699827536735[180] = 0.0;
   out_210273699827536735[181] = 0.0;
   out_210273699827536735[182] = 0.0;
   out_210273699827536735[183] = 0.0;
   out_210273699827536735[184] = 0.0;
   out_210273699827536735[185] = 0.0;
   out_210273699827536735[186] = 0.0;
   out_210273699827536735[187] = 0.0;
   out_210273699827536735[188] = 0.0;
   out_210273699827536735[189] = 0.0;
   out_210273699827536735[190] = 1.0;
   out_210273699827536735[191] = 0.0;
   out_210273699827536735[192] = 0.0;
   out_210273699827536735[193] = 0.0;
   out_210273699827536735[194] = 0.0;
   out_210273699827536735[195] = 0.0;
   out_210273699827536735[196] = 0.0;
   out_210273699827536735[197] = 0.0;
   out_210273699827536735[198] = 0.0;
   out_210273699827536735[199] = 0.0;
   out_210273699827536735[200] = 0.0;
   out_210273699827536735[201] = 0.0;
   out_210273699827536735[202] = 0.0;
   out_210273699827536735[203] = 0.0;
   out_210273699827536735[204] = 0.0;
   out_210273699827536735[205] = 0.0;
   out_210273699827536735[206] = 0.0;
   out_210273699827536735[207] = 0.0;
   out_210273699827536735[208] = 0.0;
   out_210273699827536735[209] = 1.0;
   out_210273699827536735[210] = 0.0;
   out_210273699827536735[211] = 0.0;
   out_210273699827536735[212] = 0.0;
   out_210273699827536735[213] = 0.0;
   out_210273699827536735[214] = 0.0;
   out_210273699827536735[215] = 0.0;
   out_210273699827536735[216] = 0.0;
   out_210273699827536735[217] = 0.0;
   out_210273699827536735[218] = 0.0;
   out_210273699827536735[219] = 0.0;
   out_210273699827536735[220] = 0.0;
   out_210273699827536735[221] = 0.0;
   out_210273699827536735[222] = 0.0;
   out_210273699827536735[223] = 0.0;
   out_210273699827536735[224] = 0.0;
   out_210273699827536735[225] = 0.0;
   out_210273699827536735[226] = 0.0;
   out_210273699827536735[227] = 0.0;
   out_210273699827536735[228] = 1.0;
   out_210273699827536735[229] = 0.0;
   out_210273699827536735[230] = 0.0;
   out_210273699827536735[231] = 0.0;
   out_210273699827536735[232] = 0.0;
   out_210273699827536735[233] = 0.0;
   out_210273699827536735[234] = 0.0;
   out_210273699827536735[235] = 0.0;
   out_210273699827536735[236] = 0.0;
   out_210273699827536735[237] = 0.0;
   out_210273699827536735[238] = 0.0;
   out_210273699827536735[239] = 0.0;
   out_210273699827536735[240] = 0.0;
   out_210273699827536735[241] = 0.0;
   out_210273699827536735[242] = 0.0;
   out_210273699827536735[243] = 0.0;
   out_210273699827536735[244] = 0.0;
   out_210273699827536735[245] = 0.0;
   out_210273699827536735[246] = 0.0;
   out_210273699827536735[247] = 1.0;
   out_210273699827536735[248] = 0.0;
   out_210273699827536735[249] = 0.0;
   out_210273699827536735[250] = 0.0;
   out_210273699827536735[251] = 0.0;
   out_210273699827536735[252] = 0.0;
   out_210273699827536735[253] = 0.0;
   out_210273699827536735[254] = 0.0;
   out_210273699827536735[255] = 0.0;
   out_210273699827536735[256] = 0.0;
   out_210273699827536735[257] = 0.0;
   out_210273699827536735[258] = 0.0;
   out_210273699827536735[259] = 0.0;
   out_210273699827536735[260] = 0.0;
   out_210273699827536735[261] = 0.0;
   out_210273699827536735[262] = 0.0;
   out_210273699827536735[263] = 0.0;
   out_210273699827536735[264] = 0.0;
   out_210273699827536735[265] = 0.0;
   out_210273699827536735[266] = 1.0;
   out_210273699827536735[267] = 0.0;
   out_210273699827536735[268] = 0.0;
   out_210273699827536735[269] = 0.0;
   out_210273699827536735[270] = 0.0;
   out_210273699827536735[271] = 0.0;
   out_210273699827536735[272] = 0.0;
   out_210273699827536735[273] = 0.0;
   out_210273699827536735[274] = 0.0;
   out_210273699827536735[275] = 0.0;
   out_210273699827536735[276] = 0.0;
   out_210273699827536735[277] = 0.0;
   out_210273699827536735[278] = 0.0;
   out_210273699827536735[279] = 0.0;
   out_210273699827536735[280] = 0.0;
   out_210273699827536735[281] = 0.0;
   out_210273699827536735[282] = 0.0;
   out_210273699827536735[283] = 0.0;
   out_210273699827536735[284] = 0.0;
   out_210273699827536735[285] = 1.0;
   out_210273699827536735[286] = 0.0;
   out_210273699827536735[287] = 0.0;
   out_210273699827536735[288] = 0.0;
   out_210273699827536735[289] = 0.0;
   out_210273699827536735[290] = 0.0;
   out_210273699827536735[291] = 0.0;
   out_210273699827536735[292] = 0.0;
   out_210273699827536735[293] = 0.0;
   out_210273699827536735[294] = 0.0;
   out_210273699827536735[295] = 0.0;
   out_210273699827536735[296] = 0.0;
   out_210273699827536735[297] = 0.0;
   out_210273699827536735[298] = 0.0;
   out_210273699827536735[299] = 0.0;
   out_210273699827536735[300] = 0.0;
   out_210273699827536735[301] = 0.0;
   out_210273699827536735[302] = 0.0;
   out_210273699827536735[303] = 0.0;
   out_210273699827536735[304] = 1.0;
   out_210273699827536735[305] = 0.0;
   out_210273699827536735[306] = 0.0;
   out_210273699827536735[307] = 0.0;
   out_210273699827536735[308] = 0.0;
   out_210273699827536735[309] = 0.0;
   out_210273699827536735[310] = 0.0;
   out_210273699827536735[311] = 0.0;
   out_210273699827536735[312] = 0.0;
   out_210273699827536735[313] = 0.0;
   out_210273699827536735[314] = 0.0;
   out_210273699827536735[315] = 0.0;
   out_210273699827536735[316] = 0.0;
   out_210273699827536735[317] = 0.0;
   out_210273699827536735[318] = 0.0;
   out_210273699827536735[319] = 0.0;
   out_210273699827536735[320] = 0.0;
   out_210273699827536735[321] = 0.0;
   out_210273699827536735[322] = 0.0;
   out_210273699827536735[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_3137858729334731425) {
   out_3137858729334731425[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_3137858729334731425[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_3137858729334731425[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_3137858729334731425[3] = dt*state[12] + state[3];
   out_3137858729334731425[4] = dt*state[13] + state[4];
   out_3137858729334731425[5] = dt*state[14] + state[5];
   out_3137858729334731425[6] = state[6];
   out_3137858729334731425[7] = state[7];
   out_3137858729334731425[8] = state[8];
   out_3137858729334731425[9] = state[9];
   out_3137858729334731425[10] = state[10];
   out_3137858729334731425[11] = state[11];
   out_3137858729334731425[12] = state[12];
   out_3137858729334731425[13] = state[13];
   out_3137858729334731425[14] = state[14];
   out_3137858729334731425[15] = state[15];
   out_3137858729334731425[16] = state[16];
   out_3137858729334731425[17] = state[17];
}
void F_fun(double *state, double dt, double *out_6711386679040656198) {
   out_6711386679040656198[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6711386679040656198[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6711386679040656198[2] = 0;
   out_6711386679040656198[3] = 0;
   out_6711386679040656198[4] = 0;
   out_6711386679040656198[5] = 0;
   out_6711386679040656198[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6711386679040656198[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6711386679040656198[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_6711386679040656198[9] = 0;
   out_6711386679040656198[10] = 0;
   out_6711386679040656198[11] = 0;
   out_6711386679040656198[12] = 0;
   out_6711386679040656198[13] = 0;
   out_6711386679040656198[14] = 0;
   out_6711386679040656198[15] = 0;
   out_6711386679040656198[16] = 0;
   out_6711386679040656198[17] = 0;
   out_6711386679040656198[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6711386679040656198[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6711386679040656198[20] = 0;
   out_6711386679040656198[21] = 0;
   out_6711386679040656198[22] = 0;
   out_6711386679040656198[23] = 0;
   out_6711386679040656198[24] = 0;
   out_6711386679040656198[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6711386679040656198[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_6711386679040656198[27] = 0;
   out_6711386679040656198[28] = 0;
   out_6711386679040656198[29] = 0;
   out_6711386679040656198[30] = 0;
   out_6711386679040656198[31] = 0;
   out_6711386679040656198[32] = 0;
   out_6711386679040656198[33] = 0;
   out_6711386679040656198[34] = 0;
   out_6711386679040656198[35] = 0;
   out_6711386679040656198[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6711386679040656198[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6711386679040656198[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6711386679040656198[39] = 0;
   out_6711386679040656198[40] = 0;
   out_6711386679040656198[41] = 0;
   out_6711386679040656198[42] = 0;
   out_6711386679040656198[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6711386679040656198[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_6711386679040656198[45] = 0;
   out_6711386679040656198[46] = 0;
   out_6711386679040656198[47] = 0;
   out_6711386679040656198[48] = 0;
   out_6711386679040656198[49] = 0;
   out_6711386679040656198[50] = 0;
   out_6711386679040656198[51] = 0;
   out_6711386679040656198[52] = 0;
   out_6711386679040656198[53] = 0;
   out_6711386679040656198[54] = 0;
   out_6711386679040656198[55] = 0;
   out_6711386679040656198[56] = 0;
   out_6711386679040656198[57] = 1;
   out_6711386679040656198[58] = 0;
   out_6711386679040656198[59] = 0;
   out_6711386679040656198[60] = 0;
   out_6711386679040656198[61] = 0;
   out_6711386679040656198[62] = 0;
   out_6711386679040656198[63] = 0;
   out_6711386679040656198[64] = 0;
   out_6711386679040656198[65] = 0;
   out_6711386679040656198[66] = dt;
   out_6711386679040656198[67] = 0;
   out_6711386679040656198[68] = 0;
   out_6711386679040656198[69] = 0;
   out_6711386679040656198[70] = 0;
   out_6711386679040656198[71] = 0;
   out_6711386679040656198[72] = 0;
   out_6711386679040656198[73] = 0;
   out_6711386679040656198[74] = 0;
   out_6711386679040656198[75] = 0;
   out_6711386679040656198[76] = 1;
   out_6711386679040656198[77] = 0;
   out_6711386679040656198[78] = 0;
   out_6711386679040656198[79] = 0;
   out_6711386679040656198[80] = 0;
   out_6711386679040656198[81] = 0;
   out_6711386679040656198[82] = 0;
   out_6711386679040656198[83] = 0;
   out_6711386679040656198[84] = 0;
   out_6711386679040656198[85] = dt;
   out_6711386679040656198[86] = 0;
   out_6711386679040656198[87] = 0;
   out_6711386679040656198[88] = 0;
   out_6711386679040656198[89] = 0;
   out_6711386679040656198[90] = 0;
   out_6711386679040656198[91] = 0;
   out_6711386679040656198[92] = 0;
   out_6711386679040656198[93] = 0;
   out_6711386679040656198[94] = 0;
   out_6711386679040656198[95] = 1;
   out_6711386679040656198[96] = 0;
   out_6711386679040656198[97] = 0;
   out_6711386679040656198[98] = 0;
   out_6711386679040656198[99] = 0;
   out_6711386679040656198[100] = 0;
   out_6711386679040656198[101] = 0;
   out_6711386679040656198[102] = 0;
   out_6711386679040656198[103] = 0;
   out_6711386679040656198[104] = dt;
   out_6711386679040656198[105] = 0;
   out_6711386679040656198[106] = 0;
   out_6711386679040656198[107] = 0;
   out_6711386679040656198[108] = 0;
   out_6711386679040656198[109] = 0;
   out_6711386679040656198[110] = 0;
   out_6711386679040656198[111] = 0;
   out_6711386679040656198[112] = 0;
   out_6711386679040656198[113] = 0;
   out_6711386679040656198[114] = 1;
   out_6711386679040656198[115] = 0;
   out_6711386679040656198[116] = 0;
   out_6711386679040656198[117] = 0;
   out_6711386679040656198[118] = 0;
   out_6711386679040656198[119] = 0;
   out_6711386679040656198[120] = 0;
   out_6711386679040656198[121] = 0;
   out_6711386679040656198[122] = 0;
   out_6711386679040656198[123] = 0;
   out_6711386679040656198[124] = 0;
   out_6711386679040656198[125] = 0;
   out_6711386679040656198[126] = 0;
   out_6711386679040656198[127] = 0;
   out_6711386679040656198[128] = 0;
   out_6711386679040656198[129] = 0;
   out_6711386679040656198[130] = 0;
   out_6711386679040656198[131] = 0;
   out_6711386679040656198[132] = 0;
   out_6711386679040656198[133] = 1;
   out_6711386679040656198[134] = 0;
   out_6711386679040656198[135] = 0;
   out_6711386679040656198[136] = 0;
   out_6711386679040656198[137] = 0;
   out_6711386679040656198[138] = 0;
   out_6711386679040656198[139] = 0;
   out_6711386679040656198[140] = 0;
   out_6711386679040656198[141] = 0;
   out_6711386679040656198[142] = 0;
   out_6711386679040656198[143] = 0;
   out_6711386679040656198[144] = 0;
   out_6711386679040656198[145] = 0;
   out_6711386679040656198[146] = 0;
   out_6711386679040656198[147] = 0;
   out_6711386679040656198[148] = 0;
   out_6711386679040656198[149] = 0;
   out_6711386679040656198[150] = 0;
   out_6711386679040656198[151] = 0;
   out_6711386679040656198[152] = 1;
   out_6711386679040656198[153] = 0;
   out_6711386679040656198[154] = 0;
   out_6711386679040656198[155] = 0;
   out_6711386679040656198[156] = 0;
   out_6711386679040656198[157] = 0;
   out_6711386679040656198[158] = 0;
   out_6711386679040656198[159] = 0;
   out_6711386679040656198[160] = 0;
   out_6711386679040656198[161] = 0;
   out_6711386679040656198[162] = 0;
   out_6711386679040656198[163] = 0;
   out_6711386679040656198[164] = 0;
   out_6711386679040656198[165] = 0;
   out_6711386679040656198[166] = 0;
   out_6711386679040656198[167] = 0;
   out_6711386679040656198[168] = 0;
   out_6711386679040656198[169] = 0;
   out_6711386679040656198[170] = 0;
   out_6711386679040656198[171] = 1;
   out_6711386679040656198[172] = 0;
   out_6711386679040656198[173] = 0;
   out_6711386679040656198[174] = 0;
   out_6711386679040656198[175] = 0;
   out_6711386679040656198[176] = 0;
   out_6711386679040656198[177] = 0;
   out_6711386679040656198[178] = 0;
   out_6711386679040656198[179] = 0;
   out_6711386679040656198[180] = 0;
   out_6711386679040656198[181] = 0;
   out_6711386679040656198[182] = 0;
   out_6711386679040656198[183] = 0;
   out_6711386679040656198[184] = 0;
   out_6711386679040656198[185] = 0;
   out_6711386679040656198[186] = 0;
   out_6711386679040656198[187] = 0;
   out_6711386679040656198[188] = 0;
   out_6711386679040656198[189] = 0;
   out_6711386679040656198[190] = 1;
   out_6711386679040656198[191] = 0;
   out_6711386679040656198[192] = 0;
   out_6711386679040656198[193] = 0;
   out_6711386679040656198[194] = 0;
   out_6711386679040656198[195] = 0;
   out_6711386679040656198[196] = 0;
   out_6711386679040656198[197] = 0;
   out_6711386679040656198[198] = 0;
   out_6711386679040656198[199] = 0;
   out_6711386679040656198[200] = 0;
   out_6711386679040656198[201] = 0;
   out_6711386679040656198[202] = 0;
   out_6711386679040656198[203] = 0;
   out_6711386679040656198[204] = 0;
   out_6711386679040656198[205] = 0;
   out_6711386679040656198[206] = 0;
   out_6711386679040656198[207] = 0;
   out_6711386679040656198[208] = 0;
   out_6711386679040656198[209] = 1;
   out_6711386679040656198[210] = 0;
   out_6711386679040656198[211] = 0;
   out_6711386679040656198[212] = 0;
   out_6711386679040656198[213] = 0;
   out_6711386679040656198[214] = 0;
   out_6711386679040656198[215] = 0;
   out_6711386679040656198[216] = 0;
   out_6711386679040656198[217] = 0;
   out_6711386679040656198[218] = 0;
   out_6711386679040656198[219] = 0;
   out_6711386679040656198[220] = 0;
   out_6711386679040656198[221] = 0;
   out_6711386679040656198[222] = 0;
   out_6711386679040656198[223] = 0;
   out_6711386679040656198[224] = 0;
   out_6711386679040656198[225] = 0;
   out_6711386679040656198[226] = 0;
   out_6711386679040656198[227] = 0;
   out_6711386679040656198[228] = 1;
   out_6711386679040656198[229] = 0;
   out_6711386679040656198[230] = 0;
   out_6711386679040656198[231] = 0;
   out_6711386679040656198[232] = 0;
   out_6711386679040656198[233] = 0;
   out_6711386679040656198[234] = 0;
   out_6711386679040656198[235] = 0;
   out_6711386679040656198[236] = 0;
   out_6711386679040656198[237] = 0;
   out_6711386679040656198[238] = 0;
   out_6711386679040656198[239] = 0;
   out_6711386679040656198[240] = 0;
   out_6711386679040656198[241] = 0;
   out_6711386679040656198[242] = 0;
   out_6711386679040656198[243] = 0;
   out_6711386679040656198[244] = 0;
   out_6711386679040656198[245] = 0;
   out_6711386679040656198[246] = 0;
   out_6711386679040656198[247] = 1;
   out_6711386679040656198[248] = 0;
   out_6711386679040656198[249] = 0;
   out_6711386679040656198[250] = 0;
   out_6711386679040656198[251] = 0;
   out_6711386679040656198[252] = 0;
   out_6711386679040656198[253] = 0;
   out_6711386679040656198[254] = 0;
   out_6711386679040656198[255] = 0;
   out_6711386679040656198[256] = 0;
   out_6711386679040656198[257] = 0;
   out_6711386679040656198[258] = 0;
   out_6711386679040656198[259] = 0;
   out_6711386679040656198[260] = 0;
   out_6711386679040656198[261] = 0;
   out_6711386679040656198[262] = 0;
   out_6711386679040656198[263] = 0;
   out_6711386679040656198[264] = 0;
   out_6711386679040656198[265] = 0;
   out_6711386679040656198[266] = 1;
   out_6711386679040656198[267] = 0;
   out_6711386679040656198[268] = 0;
   out_6711386679040656198[269] = 0;
   out_6711386679040656198[270] = 0;
   out_6711386679040656198[271] = 0;
   out_6711386679040656198[272] = 0;
   out_6711386679040656198[273] = 0;
   out_6711386679040656198[274] = 0;
   out_6711386679040656198[275] = 0;
   out_6711386679040656198[276] = 0;
   out_6711386679040656198[277] = 0;
   out_6711386679040656198[278] = 0;
   out_6711386679040656198[279] = 0;
   out_6711386679040656198[280] = 0;
   out_6711386679040656198[281] = 0;
   out_6711386679040656198[282] = 0;
   out_6711386679040656198[283] = 0;
   out_6711386679040656198[284] = 0;
   out_6711386679040656198[285] = 1;
   out_6711386679040656198[286] = 0;
   out_6711386679040656198[287] = 0;
   out_6711386679040656198[288] = 0;
   out_6711386679040656198[289] = 0;
   out_6711386679040656198[290] = 0;
   out_6711386679040656198[291] = 0;
   out_6711386679040656198[292] = 0;
   out_6711386679040656198[293] = 0;
   out_6711386679040656198[294] = 0;
   out_6711386679040656198[295] = 0;
   out_6711386679040656198[296] = 0;
   out_6711386679040656198[297] = 0;
   out_6711386679040656198[298] = 0;
   out_6711386679040656198[299] = 0;
   out_6711386679040656198[300] = 0;
   out_6711386679040656198[301] = 0;
   out_6711386679040656198[302] = 0;
   out_6711386679040656198[303] = 0;
   out_6711386679040656198[304] = 1;
   out_6711386679040656198[305] = 0;
   out_6711386679040656198[306] = 0;
   out_6711386679040656198[307] = 0;
   out_6711386679040656198[308] = 0;
   out_6711386679040656198[309] = 0;
   out_6711386679040656198[310] = 0;
   out_6711386679040656198[311] = 0;
   out_6711386679040656198[312] = 0;
   out_6711386679040656198[313] = 0;
   out_6711386679040656198[314] = 0;
   out_6711386679040656198[315] = 0;
   out_6711386679040656198[316] = 0;
   out_6711386679040656198[317] = 0;
   out_6711386679040656198[318] = 0;
   out_6711386679040656198[319] = 0;
   out_6711386679040656198[320] = 0;
   out_6711386679040656198[321] = 0;
   out_6711386679040656198[322] = 0;
   out_6711386679040656198[323] = 1;
}
void h_4(double *state, double *unused, double *out_5605177345384889420) {
   out_5605177345384889420[0] = state[6] + state[9];
   out_5605177345384889420[1] = state[7] + state[10];
   out_5605177345384889420[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_845238850510018990) {
   out_845238850510018990[0] = 0;
   out_845238850510018990[1] = 0;
   out_845238850510018990[2] = 0;
   out_845238850510018990[3] = 0;
   out_845238850510018990[4] = 0;
   out_845238850510018990[5] = 0;
   out_845238850510018990[6] = 1;
   out_845238850510018990[7] = 0;
   out_845238850510018990[8] = 0;
   out_845238850510018990[9] = 1;
   out_845238850510018990[10] = 0;
   out_845238850510018990[11] = 0;
   out_845238850510018990[12] = 0;
   out_845238850510018990[13] = 0;
   out_845238850510018990[14] = 0;
   out_845238850510018990[15] = 0;
   out_845238850510018990[16] = 0;
   out_845238850510018990[17] = 0;
   out_845238850510018990[18] = 0;
   out_845238850510018990[19] = 0;
   out_845238850510018990[20] = 0;
   out_845238850510018990[21] = 0;
   out_845238850510018990[22] = 0;
   out_845238850510018990[23] = 0;
   out_845238850510018990[24] = 0;
   out_845238850510018990[25] = 1;
   out_845238850510018990[26] = 0;
   out_845238850510018990[27] = 0;
   out_845238850510018990[28] = 1;
   out_845238850510018990[29] = 0;
   out_845238850510018990[30] = 0;
   out_845238850510018990[31] = 0;
   out_845238850510018990[32] = 0;
   out_845238850510018990[33] = 0;
   out_845238850510018990[34] = 0;
   out_845238850510018990[35] = 0;
   out_845238850510018990[36] = 0;
   out_845238850510018990[37] = 0;
   out_845238850510018990[38] = 0;
   out_845238850510018990[39] = 0;
   out_845238850510018990[40] = 0;
   out_845238850510018990[41] = 0;
   out_845238850510018990[42] = 0;
   out_845238850510018990[43] = 0;
   out_845238850510018990[44] = 1;
   out_845238850510018990[45] = 0;
   out_845238850510018990[46] = 0;
   out_845238850510018990[47] = 1;
   out_845238850510018990[48] = 0;
   out_845238850510018990[49] = 0;
   out_845238850510018990[50] = 0;
   out_845238850510018990[51] = 0;
   out_845238850510018990[52] = 0;
   out_845238850510018990[53] = 0;
}
void h_10(double *state, double *unused, double *out_2811354315479325630) {
   out_2811354315479325630[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_2811354315479325630[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_2811354315479325630[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_8075210817719937862) {
   out_8075210817719937862[0] = 0;
   out_8075210817719937862[1] = 9.8100000000000005*cos(state[1]);
   out_8075210817719937862[2] = 0;
   out_8075210817719937862[3] = 0;
   out_8075210817719937862[4] = -state[8];
   out_8075210817719937862[5] = state[7];
   out_8075210817719937862[6] = 0;
   out_8075210817719937862[7] = state[5];
   out_8075210817719937862[8] = -state[4];
   out_8075210817719937862[9] = 0;
   out_8075210817719937862[10] = 0;
   out_8075210817719937862[11] = 0;
   out_8075210817719937862[12] = 1;
   out_8075210817719937862[13] = 0;
   out_8075210817719937862[14] = 0;
   out_8075210817719937862[15] = 1;
   out_8075210817719937862[16] = 0;
   out_8075210817719937862[17] = 0;
   out_8075210817719937862[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_8075210817719937862[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_8075210817719937862[20] = 0;
   out_8075210817719937862[21] = state[8];
   out_8075210817719937862[22] = 0;
   out_8075210817719937862[23] = -state[6];
   out_8075210817719937862[24] = -state[5];
   out_8075210817719937862[25] = 0;
   out_8075210817719937862[26] = state[3];
   out_8075210817719937862[27] = 0;
   out_8075210817719937862[28] = 0;
   out_8075210817719937862[29] = 0;
   out_8075210817719937862[30] = 0;
   out_8075210817719937862[31] = 1;
   out_8075210817719937862[32] = 0;
   out_8075210817719937862[33] = 0;
   out_8075210817719937862[34] = 1;
   out_8075210817719937862[35] = 0;
   out_8075210817719937862[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_8075210817719937862[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_8075210817719937862[38] = 0;
   out_8075210817719937862[39] = -state[7];
   out_8075210817719937862[40] = state[6];
   out_8075210817719937862[41] = 0;
   out_8075210817719937862[42] = state[4];
   out_8075210817719937862[43] = -state[3];
   out_8075210817719937862[44] = 0;
   out_8075210817719937862[45] = 0;
   out_8075210817719937862[46] = 0;
   out_8075210817719937862[47] = 0;
   out_8075210817719937862[48] = 0;
   out_8075210817719937862[49] = 0;
   out_8075210817719937862[50] = 1;
   out_8075210817719937862[51] = 0;
   out_8075210817719937862[52] = 0;
   out_8075210817719937862[53] = 1;
}
void h_13(double *state, double *unused, double *out_4621388109736156056) {
   out_4621388109736156056[0] = state[3];
   out_4621388109736156056[1] = state[4];
   out_4621388109736156056[2] = state[5];
}
void H_13(double *state, double *unused, double *out_2367034974822313811) {
   out_2367034974822313811[0] = 0;
   out_2367034974822313811[1] = 0;
   out_2367034974822313811[2] = 0;
   out_2367034974822313811[3] = 1;
   out_2367034974822313811[4] = 0;
   out_2367034974822313811[5] = 0;
   out_2367034974822313811[6] = 0;
   out_2367034974822313811[7] = 0;
   out_2367034974822313811[8] = 0;
   out_2367034974822313811[9] = 0;
   out_2367034974822313811[10] = 0;
   out_2367034974822313811[11] = 0;
   out_2367034974822313811[12] = 0;
   out_2367034974822313811[13] = 0;
   out_2367034974822313811[14] = 0;
   out_2367034974822313811[15] = 0;
   out_2367034974822313811[16] = 0;
   out_2367034974822313811[17] = 0;
   out_2367034974822313811[18] = 0;
   out_2367034974822313811[19] = 0;
   out_2367034974822313811[20] = 0;
   out_2367034974822313811[21] = 0;
   out_2367034974822313811[22] = 1;
   out_2367034974822313811[23] = 0;
   out_2367034974822313811[24] = 0;
   out_2367034974822313811[25] = 0;
   out_2367034974822313811[26] = 0;
   out_2367034974822313811[27] = 0;
   out_2367034974822313811[28] = 0;
   out_2367034974822313811[29] = 0;
   out_2367034974822313811[30] = 0;
   out_2367034974822313811[31] = 0;
   out_2367034974822313811[32] = 0;
   out_2367034974822313811[33] = 0;
   out_2367034974822313811[34] = 0;
   out_2367034974822313811[35] = 0;
   out_2367034974822313811[36] = 0;
   out_2367034974822313811[37] = 0;
   out_2367034974822313811[38] = 0;
   out_2367034974822313811[39] = 0;
   out_2367034974822313811[40] = 0;
   out_2367034974822313811[41] = 1;
   out_2367034974822313811[42] = 0;
   out_2367034974822313811[43] = 0;
   out_2367034974822313811[44] = 0;
   out_2367034974822313811[45] = 0;
   out_2367034974822313811[46] = 0;
   out_2367034974822313811[47] = 0;
   out_2367034974822313811[48] = 0;
   out_2367034974822313811[49] = 0;
   out_2367034974822313811[50] = 0;
   out_2367034974822313811[51] = 0;
   out_2367034974822313811[52] = 0;
   out_2367034974822313811[53] = 0;
}
void h_14(double *state, double *unused, double *out_267354547646598181) {
   out_267354547646598181[0] = state[6];
   out_267354547646598181[1] = state[7];
   out_267354547646598181[2] = state[8];
}
void H_14(double *state, double *unused, double *out_3928027282805391286) {
   out_3928027282805391286[0] = 0;
   out_3928027282805391286[1] = 0;
   out_3928027282805391286[2] = 0;
   out_3928027282805391286[3] = 0;
   out_3928027282805391286[4] = 0;
   out_3928027282805391286[5] = 0;
   out_3928027282805391286[6] = 1;
   out_3928027282805391286[7] = 0;
   out_3928027282805391286[8] = 0;
   out_3928027282805391286[9] = 0;
   out_3928027282805391286[10] = 0;
   out_3928027282805391286[11] = 0;
   out_3928027282805391286[12] = 0;
   out_3928027282805391286[13] = 0;
   out_3928027282805391286[14] = 0;
   out_3928027282805391286[15] = 0;
   out_3928027282805391286[16] = 0;
   out_3928027282805391286[17] = 0;
   out_3928027282805391286[18] = 0;
   out_3928027282805391286[19] = 0;
   out_3928027282805391286[20] = 0;
   out_3928027282805391286[21] = 0;
   out_3928027282805391286[22] = 0;
   out_3928027282805391286[23] = 0;
   out_3928027282805391286[24] = 0;
   out_3928027282805391286[25] = 1;
   out_3928027282805391286[26] = 0;
   out_3928027282805391286[27] = 0;
   out_3928027282805391286[28] = 0;
   out_3928027282805391286[29] = 0;
   out_3928027282805391286[30] = 0;
   out_3928027282805391286[31] = 0;
   out_3928027282805391286[32] = 0;
   out_3928027282805391286[33] = 0;
   out_3928027282805391286[34] = 0;
   out_3928027282805391286[35] = 0;
   out_3928027282805391286[36] = 0;
   out_3928027282805391286[37] = 0;
   out_3928027282805391286[38] = 0;
   out_3928027282805391286[39] = 0;
   out_3928027282805391286[40] = 0;
   out_3928027282805391286[41] = 0;
   out_3928027282805391286[42] = 0;
   out_3928027282805391286[43] = 0;
   out_3928027282805391286[44] = 1;
   out_3928027282805391286[45] = 0;
   out_3928027282805391286[46] = 0;
   out_3928027282805391286[47] = 0;
   out_3928027282805391286[48] = 0;
   out_3928027282805391286[49] = 0;
   out_3928027282805391286[50] = 0;
   out_3928027282805391286[51] = 0;
   out_3928027282805391286[52] = 0;
   out_3928027282805391286[53] = 0;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_4, H_4, NULL, in_z, in_R, in_ea, MAHA_THRESH_4);
}
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_10, H_10, NULL, in_z, in_R, in_ea, MAHA_THRESH_10);
}
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_13, H_13, NULL, in_z, in_R, in_ea, MAHA_THRESH_13);
}
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<3, 3, 0>(in_x, in_P, h_14, H_14, NULL, in_z, in_R, in_ea, MAHA_THRESH_14);
}
void pose_err_fun(double *nom_x, double *delta_x, double *out_3549856774171141684) {
  err_fun(nom_x, delta_x, out_3549856774171141684);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2925455247541166295) {
  inv_err_fun(nom_x, true_x, out_2925455247541166295);
}
void pose_H_mod_fun(double *state, double *out_210273699827536735) {
  H_mod_fun(state, out_210273699827536735);
}
void pose_f_fun(double *state, double dt, double *out_3137858729334731425) {
  f_fun(state,  dt, out_3137858729334731425);
}
void pose_F_fun(double *state, double dt, double *out_6711386679040656198) {
  F_fun(state,  dt, out_6711386679040656198);
}
void pose_h_4(double *state, double *unused, double *out_5605177345384889420) {
  h_4(state, unused, out_5605177345384889420);
}
void pose_H_4(double *state, double *unused, double *out_845238850510018990) {
  H_4(state, unused, out_845238850510018990);
}
void pose_h_10(double *state, double *unused, double *out_2811354315479325630) {
  h_10(state, unused, out_2811354315479325630);
}
void pose_H_10(double *state, double *unused, double *out_8075210817719937862) {
  H_10(state, unused, out_8075210817719937862);
}
void pose_h_13(double *state, double *unused, double *out_4621388109736156056) {
  h_13(state, unused, out_4621388109736156056);
}
void pose_H_13(double *state, double *unused, double *out_2367034974822313811) {
  H_13(state, unused, out_2367034974822313811);
}
void pose_h_14(double *state, double *unused, double *out_267354547646598181) {
  h_14(state, unused, out_267354547646598181);
}
void pose_H_14(double *state, double *unused, double *out_3928027282805391286) {
  H_14(state, unused, out_3928027282805391286);
}
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
}

const EKF pose = {
  .name = "pose",
  .kinds = { 4, 10, 13, 14 },
  .feature_kinds = {  },
  .f_fun = pose_f_fun,
  .F_fun = pose_F_fun,
  .err_fun = pose_err_fun,
  .inv_err_fun = pose_inv_err_fun,
  .H_mod_fun = pose_H_mod_fun,
  .predict = pose_predict,
  .hs = {
    { 4, pose_h_4 },
    { 10, pose_h_10 },
    { 13, pose_h_13 },
    { 14, pose_h_14 },
  },
  .Hs = {
    { 4, pose_H_4 },
    { 10, pose_H_10 },
    { 13, pose_H_13 },
    { 14, pose_H_14 },
  },
  .updates = {
    { 4, pose_update_4 },
    { 10, pose_update_10 },
    { 13, pose_update_13 },
    { 14, pose_update_14 },
  },
  .Hes = {
  },
  .sets = {
  },
  .extra_routines = {
  },
};

ekf_lib_init(pose)
