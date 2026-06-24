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
void err_fun(double *nom_x, double *delta_x, double *out_1112526487984770436) {
   out_1112526487984770436[0] = delta_x[0] + nom_x[0];
   out_1112526487984770436[1] = delta_x[1] + nom_x[1];
   out_1112526487984770436[2] = delta_x[2] + nom_x[2];
   out_1112526487984770436[3] = delta_x[3] + nom_x[3];
   out_1112526487984770436[4] = delta_x[4] + nom_x[4];
   out_1112526487984770436[5] = delta_x[5] + nom_x[5];
   out_1112526487984770436[6] = delta_x[6] + nom_x[6];
   out_1112526487984770436[7] = delta_x[7] + nom_x[7];
   out_1112526487984770436[8] = delta_x[8] + nom_x[8];
   out_1112526487984770436[9] = delta_x[9] + nom_x[9];
   out_1112526487984770436[10] = delta_x[10] + nom_x[10];
   out_1112526487984770436[11] = delta_x[11] + nom_x[11];
   out_1112526487984770436[12] = delta_x[12] + nom_x[12];
   out_1112526487984770436[13] = delta_x[13] + nom_x[13];
   out_1112526487984770436[14] = delta_x[14] + nom_x[14];
   out_1112526487984770436[15] = delta_x[15] + nom_x[15];
   out_1112526487984770436[16] = delta_x[16] + nom_x[16];
   out_1112526487984770436[17] = delta_x[17] + nom_x[17];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_2885793937532541180) {
   out_2885793937532541180[0] = -nom_x[0] + true_x[0];
   out_2885793937532541180[1] = -nom_x[1] + true_x[1];
   out_2885793937532541180[2] = -nom_x[2] + true_x[2];
   out_2885793937532541180[3] = -nom_x[3] + true_x[3];
   out_2885793937532541180[4] = -nom_x[4] + true_x[4];
   out_2885793937532541180[5] = -nom_x[5] + true_x[5];
   out_2885793937532541180[6] = -nom_x[6] + true_x[6];
   out_2885793937532541180[7] = -nom_x[7] + true_x[7];
   out_2885793937532541180[8] = -nom_x[8] + true_x[8];
   out_2885793937532541180[9] = -nom_x[9] + true_x[9];
   out_2885793937532541180[10] = -nom_x[10] + true_x[10];
   out_2885793937532541180[11] = -nom_x[11] + true_x[11];
   out_2885793937532541180[12] = -nom_x[12] + true_x[12];
   out_2885793937532541180[13] = -nom_x[13] + true_x[13];
   out_2885793937532541180[14] = -nom_x[14] + true_x[14];
   out_2885793937532541180[15] = -nom_x[15] + true_x[15];
   out_2885793937532541180[16] = -nom_x[16] + true_x[16];
   out_2885793937532541180[17] = -nom_x[17] + true_x[17];
}
void H_mod_fun(double *state, double *out_5077282169950591517) {
   out_5077282169950591517[0] = 1.0;
   out_5077282169950591517[1] = 0.0;
   out_5077282169950591517[2] = 0.0;
   out_5077282169950591517[3] = 0.0;
   out_5077282169950591517[4] = 0.0;
   out_5077282169950591517[5] = 0.0;
   out_5077282169950591517[6] = 0.0;
   out_5077282169950591517[7] = 0.0;
   out_5077282169950591517[8] = 0.0;
   out_5077282169950591517[9] = 0.0;
   out_5077282169950591517[10] = 0.0;
   out_5077282169950591517[11] = 0.0;
   out_5077282169950591517[12] = 0.0;
   out_5077282169950591517[13] = 0.0;
   out_5077282169950591517[14] = 0.0;
   out_5077282169950591517[15] = 0.0;
   out_5077282169950591517[16] = 0.0;
   out_5077282169950591517[17] = 0.0;
   out_5077282169950591517[18] = 0.0;
   out_5077282169950591517[19] = 1.0;
   out_5077282169950591517[20] = 0.0;
   out_5077282169950591517[21] = 0.0;
   out_5077282169950591517[22] = 0.0;
   out_5077282169950591517[23] = 0.0;
   out_5077282169950591517[24] = 0.0;
   out_5077282169950591517[25] = 0.0;
   out_5077282169950591517[26] = 0.0;
   out_5077282169950591517[27] = 0.0;
   out_5077282169950591517[28] = 0.0;
   out_5077282169950591517[29] = 0.0;
   out_5077282169950591517[30] = 0.0;
   out_5077282169950591517[31] = 0.0;
   out_5077282169950591517[32] = 0.0;
   out_5077282169950591517[33] = 0.0;
   out_5077282169950591517[34] = 0.0;
   out_5077282169950591517[35] = 0.0;
   out_5077282169950591517[36] = 0.0;
   out_5077282169950591517[37] = 0.0;
   out_5077282169950591517[38] = 1.0;
   out_5077282169950591517[39] = 0.0;
   out_5077282169950591517[40] = 0.0;
   out_5077282169950591517[41] = 0.0;
   out_5077282169950591517[42] = 0.0;
   out_5077282169950591517[43] = 0.0;
   out_5077282169950591517[44] = 0.0;
   out_5077282169950591517[45] = 0.0;
   out_5077282169950591517[46] = 0.0;
   out_5077282169950591517[47] = 0.0;
   out_5077282169950591517[48] = 0.0;
   out_5077282169950591517[49] = 0.0;
   out_5077282169950591517[50] = 0.0;
   out_5077282169950591517[51] = 0.0;
   out_5077282169950591517[52] = 0.0;
   out_5077282169950591517[53] = 0.0;
   out_5077282169950591517[54] = 0.0;
   out_5077282169950591517[55] = 0.0;
   out_5077282169950591517[56] = 0.0;
   out_5077282169950591517[57] = 1.0;
   out_5077282169950591517[58] = 0.0;
   out_5077282169950591517[59] = 0.0;
   out_5077282169950591517[60] = 0.0;
   out_5077282169950591517[61] = 0.0;
   out_5077282169950591517[62] = 0.0;
   out_5077282169950591517[63] = 0.0;
   out_5077282169950591517[64] = 0.0;
   out_5077282169950591517[65] = 0.0;
   out_5077282169950591517[66] = 0.0;
   out_5077282169950591517[67] = 0.0;
   out_5077282169950591517[68] = 0.0;
   out_5077282169950591517[69] = 0.0;
   out_5077282169950591517[70] = 0.0;
   out_5077282169950591517[71] = 0.0;
   out_5077282169950591517[72] = 0.0;
   out_5077282169950591517[73] = 0.0;
   out_5077282169950591517[74] = 0.0;
   out_5077282169950591517[75] = 0.0;
   out_5077282169950591517[76] = 1.0;
   out_5077282169950591517[77] = 0.0;
   out_5077282169950591517[78] = 0.0;
   out_5077282169950591517[79] = 0.0;
   out_5077282169950591517[80] = 0.0;
   out_5077282169950591517[81] = 0.0;
   out_5077282169950591517[82] = 0.0;
   out_5077282169950591517[83] = 0.0;
   out_5077282169950591517[84] = 0.0;
   out_5077282169950591517[85] = 0.0;
   out_5077282169950591517[86] = 0.0;
   out_5077282169950591517[87] = 0.0;
   out_5077282169950591517[88] = 0.0;
   out_5077282169950591517[89] = 0.0;
   out_5077282169950591517[90] = 0.0;
   out_5077282169950591517[91] = 0.0;
   out_5077282169950591517[92] = 0.0;
   out_5077282169950591517[93] = 0.0;
   out_5077282169950591517[94] = 0.0;
   out_5077282169950591517[95] = 1.0;
   out_5077282169950591517[96] = 0.0;
   out_5077282169950591517[97] = 0.0;
   out_5077282169950591517[98] = 0.0;
   out_5077282169950591517[99] = 0.0;
   out_5077282169950591517[100] = 0.0;
   out_5077282169950591517[101] = 0.0;
   out_5077282169950591517[102] = 0.0;
   out_5077282169950591517[103] = 0.0;
   out_5077282169950591517[104] = 0.0;
   out_5077282169950591517[105] = 0.0;
   out_5077282169950591517[106] = 0.0;
   out_5077282169950591517[107] = 0.0;
   out_5077282169950591517[108] = 0.0;
   out_5077282169950591517[109] = 0.0;
   out_5077282169950591517[110] = 0.0;
   out_5077282169950591517[111] = 0.0;
   out_5077282169950591517[112] = 0.0;
   out_5077282169950591517[113] = 0.0;
   out_5077282169950591517[114] = 1.0;
   out_5077282169950591517[115] = 0.0;
   out_5077282169950591517[116] = 0.0;
   out_5077282169950591517[117] = 0.0;
   out_5077282169950591517[118] = 0.0;
   out_5077282169950591517[119] = 0.0;
   out_5077282169950591517[120] = 0.0;
   out_5077282169950591517[121] = 0.0;
   out_5077282169950591517[122] = 0.0;
   out_5077282169950591517[123] = 0.0;
   out_5077282169950591517[124] = 0.0;
   out_5077282169950591517[125] = 0.0;
   out_5077282169950591517[126] = 0.0;
   out_5077282169950591517[127] = 0.0;
   out_5077282169950591517[128] = 0.0;
   out_5077282169950591517[129] = 0.0;
   out_5077282169950591517[130] = 0.0;
   out_5077282169950591517[131] = 0.0;
   out_5077282169950591517[132] = 0.0;
   out_5077282169950591517[133] = 1.0;
   out_5077282169950591517[134] = 0.0;
   out_5077282169950591517[135] = 0.0;
   out_5077282169950591517[136] = 0.0;
   out_5077282169950591517[137] = 0.0;
   out_5077282169950591517[138] = 0.0;
   out_5077282169950591517[139] = 0.0;
   out_5077282169950591517[140] = 0.0;
   out_5077282169950591517[141] = 0.0;
   out_5077282169950591517[142] = 0.0;
   out_5077282169950591517[143] = 0.0;
   out_5077282169950591517[144] = 0.0;
   out_5077282169950591517[145] = 0.0;
   out_5077282169950591517[146] = 0.0;
   out_5077282169950591517[147] = 0.0;
   out_5077282169950591517[148] = 0.0;
   out_5077282169950591517[149] = 0.0;
   out_5077282169950591517[150] = 0.0;
   out_5077282169950591517[151] = 0.0;
   out_5077282169950591517[152] = 1.0;
   out_5077282169950591517[153] = 0.0;
   out_5077282169950591517[154] = 0.0;
   out_5077282169950591517[155] = 0.0;
   out_5077282169950591517[156] = 0.0;
   out_5077282169950591517[157] = 0.0;
   out_5077282169950591517[158] = 0.0;
   out_5077282169950591517[159] = 0.0;
   out_5077282169950591517[160] = 0.0;
   out_5077282169950591517[161] = 0.0;
   out_5077282169950591517[162] = 0.0;
   out_5077282169950591517[163] = 0.0;
   out_5077282169950591517[164] = 0.0;
   out_5077282169950591517[165] = 0.0;
   out_5077282169950591517[166] = 0.0;
   out_5077282169950591517[167] = 0.0;
   out_5077282169950591517[168] = 0.0;
   out_5077282169950591517[169] = 0.0;
   out_5077282169950591517[170] = 0.0;
   out_5077282169950591517[171] = 1.0;
   out_5077282169950591517[172] = 0.0;
   out_5077282169950591517[173] = 0.0;
   out_5077282169950591517[174] = 0.0;
   out_5077282169950591517[175] = 0.0;
   out_5077282169950591517[176] = 0.0;
   out_5077282169950591517[177] = 0.0;
   out_5077282169950591517[178] = 0.0;
   out_5077282169950591517[179] = 0.0;
   out_5077282169950591517[180] = 0.0;
   out_5077282169950591517[181] = 0.0;
   out_5077282169950591517[182] = 0.0;
   out_5077282169950591517[183] = 0.0;
   out_5077282169950591517[184] = 0.0;
   out_5077282169950591517[185] = 0.0;
   out_5077282169950591517[186] = 0.0;
   out_5077282169950591517[187] = 0.0;
   out_5077282169950591517[188] = 0.0;
   out_5077282169950591517[189] = 0.0;
   out_5077282169950591517[190] = 1.0;
   out_5077282169950591517[191] = 0.0;
   out_5077282169950591517[192] = 0.0;
   out_5077282169950591517[193] = 0.0;
   out_5077282169950591517[194] = 0.0;
   out_5077282169950591517[195] = 0.0;
   out_5077282169950591517[196] = 0.0;
   out_5077282169950591517[197] = 0.0;
   out_5077282169950591517[198] = 0.0;
   out_5077282169950591517[199] = 0.0;
   out_5077282169950591517[200] = 0.0;
   out_5077282169950591517[201] = 0.0;
   out_5077282169950591517[202] = 0.0;
   out_5077282169950591517[203] = 0.0;
   out_5077282169950591517[204] = 0.0;
   out_5077282169950591517[205] = 0.0;
   out_5077282169950591517[206] = 0.0;
   out_5077282169950591517[207] = 0.0;
   out_5077282169950591517[208] = 0.0;
   out_5077282169950591517[209] = 1.0;
   out_5077282169950591517[210] = 0.0;
   out_5077282169950591517[211] = 0.0;
   out_5077282169950591517[212] = 0.0;
   out_5077282169950591517[213] = 0.0;
   out_5077282169950591517[214] = 0.0;
   out_5077282169950591517[215] = 0.0;
   out_5077282169950591517[216] = 0.0;
   out_5077282169950591517[217] = 0.0;
   out_5077282169950591517[218] = 0.0;
   out_5077282169950591517[219] = 0.0;
   out_5077282169950591517[220] = 0.0;
   out_5077282169950591517[221] = 0.0;
   out_5077282169950591517[222] = 0.0;
   out_5077282169950591517[223] = 0.0;
   out_5077282169950591517[224] = 0.0;
   out_5077282169950591517[225] = 0.0;
   out_5077282169950591517[226] = 0.0;
   out_5077282169950591517[227] = 0.0;
   out_5077282169950591517[228] = 1.0;
   out_5077282169950591517[229] = 0.0;
   out_5077282169950591517[230] = 0.0;
   out_5077282169950591517[231] = 0.0;
   out_5077282169950591517[232] = 0.0;
   out_5077282169950591517[233] = 0.0;
   out_5077282169950591517[234] = 0.0;
   out_5077282169950591517[235] = 0.0;
   out_5077282169950591517[236] = 0.0;
   out_5077282169950591517[237] = 0.0;
   out_5077282169950591517[238] = 0.0;
   out_5077282169950591517[239] = 0.0;
   out_5077282169950591517[240] = 0.0;
   out_5077282169950591517[241] = 0.0;
   out_5077282169950591517[242] = 0.0;
   out_5077282169950591517[243] = 0.0;
   out_5077282169950591517[244] = 0.0;
   out_5077282169950591517[245] = 0.0;
   out_5077282169950591517[246] = 0.0;
   out_5077282169950591517[247] = 1.0;
   out_5077282169950591517[248] = 0.0;
   out_5077282169950591517[249] = 0.0;
   out_5077282169950591517[250] = 0.0;
   out_5077282169950591517[251] = 0.0;
   out_5077282169950591517[252] = 0.0;
   out_5077282169950591517[253] = 0.0;
   out_5077282169950591517[254] = 0.0;
   out_5077282169950591517[255] = 0.0;
   out_5077282169950591517[256] = 0.0;
   out_5077282169950591517[257] = 0.0;
   out_5077282169950591517[258] = 0.0;
   out_5077282169950591517[259] = 0.0;
   out_5077282169950591517[260] = 0.0;
   out_5077282169950591517[261] = 0.0;
   out_5077282169950591517[262] = 0.0;
   out_5077282169950591517[263] = 0.0;
   out_5077282169950591517[264] = 0.0;
   out_5077282169950591517[265] = 0.0;
   out_5077282169950591517[266] = 1.0;
   out_5077282169950591517[267] = 0.0;
   out_5077282169950591517[268] = 0.0;
   out_5077282169950591517[269] = 0.0;
   out_5077282169950591517[270] = 0.0;
   out_5077282169950591517[271] = 0.0;
   out_5077282169950591517[272] = 0.0;
   out_5077282169950591517[273] = 0.0;
   out_5077282169950591517[274] = 0.0;
   out_5077282169950591517[275] = 0.0;
   out_5077282169950591517[276] = 0.0;
   out_5077282169950591517[277] = 0.0;
   out_5077282169950591517[278] = 0.0;
   out_5077282169950591517[279] = 0.0;
   out_5077282169950591517[280] = 0.0;
   out_5077282169950591517[281] = 0.0;
   out_5077282169950591517[282] = 0.0;
   out_5077282169950591517[283] = 0.0;
   out_5077282169950591517[284] = 0.0;
   out_5077282169950591517[285] = 1.0;
   out_5077282169950591517[286] = 0.0;
   out_5077282169950591517[287] = 0.0;
   out_5077282169950591517[288] = 0.0;
   out_5077282169950591517[289] = 0.0;
   out_5077282169950591517[290] = 0.0;
   out_5077282169950591517[291] = 0.0;
   out_5077282169950591517[292] = 0.0;
   out_5077282169950591517[293] = 0.0;
   out_5077282169950591517[294] = 0.0;
   out_5077282169950591517[295] = 0.0;
   out_5077282169950591517[296] = 0.0;
   out_5077282169950591517[297] = 0.0;
   out_5077282169950591517[298] = 0.0;
   out_5077282169950591517[299] = 0.0;
   out_5077282169950591517[300] = 0.0;
   out_5077282169950591517[301] = 0.0;
   out_5077282169950591517[302] = 0.0;
   out_5077282169950591517[303] = 0.0;
   out_5077282169950591517[304] = 1.0;
   out_5077282169950591517[305] = 0.0;
   out_5077282169950591517[306] = 0.0;
   out_5077282169950591517[307] = 0.0;
   out_5077282169950591517[308] = 0.0;
   out_5077282169950591517[309] = 0.0;
   out_5077282169950591517[310] = 0.0;
   out_5077282169950591517[311] = 0.0;
   out_5077282169950591517[312] = 0.0;
   out_5077282169950591517[313] = 0.0;
   out_5077282169950591517[314] = 0.0;
   out_5077282169950591517[315] = 0.0;
   out_5077282169950591517[316] = 0.0;
   out_5077282169950591517[317] = 0.0;
   out_5077282169950591517[318] = 0.0;
   out_5077282169950591517[319] = 0.0;
   out_5077282169950591517[320] = 0.0;
   out_5077282169950591517[321] = 0.0;
   out_5077282169950591517[322] = 0.0;
   out_5077282169950591517[323] = 1.0;
}
void f_fun(double *state, double dt, double *out_3821476900675952296) {
   out_3821476900675952296[0] = atan2((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), -(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]));
   out_3821476900675952296[1] = asin(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]));
   out_3821476900675952296[2] = atan2(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), -(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]));
   out_3821476900675952296[3] = dt*state[12] + state[3];
   out_3821476900675952296[4] = dt*state[13] + state[4];
   out_3821476900675952296[5] = dt*state[14] + state[5];
   out_3821476900675952296[6] = state[6];
   out_3821476900675952296[7] = state[7];
   out_3821476900675952296[8] = state[8];
   out_3821476900675952296[9] = state[9];
   out_3821476900675952296[10] = state[10];
   out_3821476900675952296[11] = state[11];
   out_3821476900675952296[12] = state[12];
   out_3821476900675952296[13] = state[13];
   out_3821476900675952296[14] = state[14];
   out_3821476900675952296[15] = state[15];
   out_3821476900675952296[16] = state[16];
   out_3821476900675952296[17] = state[17];
}
void F_fun(double *state, double dt, double *out_3235005186135283106) {
   out_3235005186135283106[0] = ((-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*cos(state[0])*cos(state[1]) - sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*cos(state[0])*cos(state[1]) - sin(dt*state[6])*sin(state[0])*cos(dt*state[7])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3235005186135283106[1] = ((-sin(dt*state[6])*sin(dt*state[8]) - sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*cos(state[1]) - (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*sin(state[1]) - sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(state[0]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*sin(state[1]) + (-sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) + sin(dt*state[8])*cos(dt*state[6]))*cos(state[1]) - sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(state[0]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3235005186135283106[2] = 0;
   out_3235005186135283106[3] = 0;
   out_3235005186135283106[4] = 0;
   out_3235005186135283106[5] = 0;
   out_3235005186135283106[6] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(dt*cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) - dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3235005186135283106[7] = (-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[6])*sin(dt*state[7])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[6])*sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) - dt*sin(dt*state[6])*sin(state[1])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + (-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))*(-dt*sin(dt*state[7])*cos(dt*state[6])*cos(state[0])*cos(state[1]) + dt*sin(dt*state[8])*sin(state[0])*cos(dt*state[6])*cos(dt*state[7])*cos(state[1]) - dt*sin(state[1])*cos(dt*state[6])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3235005186135283106[8] = ((dt*sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + dt*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (dt*sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]))*(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2)) + ((dt*sin(dt*state[6])*sin(dt*state[8]) + dt*sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (-dt*sin(dt*state[6])*cos(dt*state[8]) + dt*sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]))*(-(sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) + (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) - sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/(pow(-(sin(dt*state[6])*sin(dt*state[8]) + sin(dt*state[7])*cos(dt*state[6])*cos(dt*state[8]))*sin(state[1]) + (-sin(dt*state[6])*cos(dt*state[8]) + sin(dt*state[7])*sin(dt*state[8])*cos(dt*state[6]))*sin(state[0])*cos(state[1]) + cos(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2) + pow((sin(dt*state[6])*sin(dt*state[7])*sin(dt*state[8]) + cos(dt*state[6])*cos(dt*state[8]))*sin(state[0])*cos(state[1]) - (sin(dt*state[6])*sin(dt*state[7])*cos(dt*state[8]) - sin(dt*state[8])*cos(dt*state[6]))*sin(state[1]) + sin(dt*state[6])*cos(dt*state[7])*cos(state[0])*cos(state[1]), 2));
   out_3235005186135283106[9] = 0;
   out_3235005186135283106[10] = 0;
   out_3235005186135283106[11] = 0;
   out_3235005186135283106[12] = 0;
   out_3235005186135283106[13] = 0;
   out_3235005186135283106[14] = 0;
   out_3235005186135283106[15] = 0;
   out_3235005186135283106[16] = 0;
   out_3235005186135283106[17] = 0;
   out_3235005186135283106[18] = (-sin(dt*state[7])*sin(state[0])*cos(state[1]) - sin(dt*state[8])*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3235005186135283106[19] = (-sin(dt*state[7])*sin(state[1])*cos(state[0]) + sin(dt*state[8])*sin(state[0])*sin(state[1])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3235005186135283106[20] = 0;
   out_3235005186135283106[21] = 0;
   out_3235005186135283106[22] = 0;
   out_3235005186135283106[23] = 0;
   out_3235005186135283106[24] = 0;
   out_3235005186135283106[25] = (dt*sin(dt*state[7])*sin(dt*state[8])*sin(state[0])*cos(state[1]) - dt*sin(dt*state[7])*sin(state[1])*cos(dt*state[8]) + dt*cos(dt*state[7])*cos(state[0])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3235005186135283106[26] = (-dt*sin(dt*state[8])*sin(state[1])*cos(dt*state[7]) - dt*sin(state[0])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/sqrt(1 - pow(sin(dt*state[7])*cos(state[0])*cos(state[1]) - sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1]) + sin(state[1])*cos(dt*state[7])*cos(dt*state[8]), 2));
   out_3235005186135283106[27] = 0;
   out_3235005186135283106[28] = 0;
   out_3235005186135283106[29] = 0;
   out_3235005186135283106[30] = 0;
   out_3235005186135283106[31] = 0;
   out_3235005186135283106[32] = 0;
   out_3235005186135283106[33] = 0;
   out_3235005186135283106[34] = 0;
   out_3235005186135283106[35] = 0;
   out_3235005186135283106[36] = ((sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3235005186135283106[37] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-sin(dt*state[7])*sin(state[2])*cos(state[0])*cos(state[1]) + sin(dt*state[8])*sin(state[0])*sin(state[2])*cos(dt*state[7])*cos(state[1]) - sin(state[1])*sin(state[2])*cos(dt*state[7])*cos(dt*state[8]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(-sin(dt*state[7])*cos(state[0])*cos(state[1])*cos(state[2]) + sin(dt*state[8])*sin(state[0])*cos(dt*state[7])*cos(state[1])*cos(state[2]) - sin(state[1])*cos(dt*state[7])*cos(dt*state[8])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3235005186135283106[38] = ((-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (-sin(state[0])*sin(state[1])*sin(state[2]) - cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3235005186135283106[39] = 0;
   out_3235005186135283106[40] = 0;
   out_3235005186135283106[41] = 0;
   out_3235005186135283106[42] = 0;
   out_3235005186135283106[43] = (-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))*(dt*(sin(state[0])*cos(state[2]) - sin(state[1])*sin(state[2])*cos(state[0]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*sin(state[2])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + ((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))*(dt*(-sin(state[0])*sin(state[2]) - sin(state[1])*cos(state[0])*cos(state[2]))*cos(dt*state[7]) - dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[7])*sin(dt*state[8]) - dt*sin(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3235005186135283106[44] = (dt*(sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*sin(state[2])*cos(dt*state[7])*cos(state[1]))*(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2)) + (dt*(sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*cos(dt*state[7])*cos(dt*state[8]) - dt*sin(dt*state[8])*cos(dt*state[7])*cos(state[1])*cos(state[2]))*((-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) - (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) - sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]))/(pow(-(sin(state[0])*sin(state[2]) + sin(state[1])*cos(state[0])*cos(state[2]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*cos(state[2]) - sin(state[2])*cos(state[0]))*sin(dt*state[8])*cos(dt*state[7]) + cos(dt*state[7])*cos(dt*state[8])*cos(state[1])*cos(state[2]), 2) + pow(-(-sin(state[0])*cos(state[2]) + sin(state[1])*sin(state[2])*cos(state[0]))*sin(dt*state[7]) + (sin(state[0])*sin(state[1])*sin(state[2]) + cos(state[0])*cos(state[2]))*sin(dt*state[8])*cos(dt*state[7]) + sin(state[2])*cos(dt*state[7])*cos(dt*state[8])*cos(state[1]), 2));
   out_3235005186135283106[45] = 0;
   out_3235005186135283106[46] = 0;
   out_3235005186135283106[47] = 0;
   out_3235005186135283106[48] = 0;
   out_3235005186135283106[49] = 0;
   out_3235005186135283106[50] = 0;
   out_3235005186135283106[51] = 0;
   out_3235005186135283106[52] = 0;
   out_3235005186135283106[53] = 0;
   out_3235005186135283106[54] = 0;
   out_3235005186135283106[55] = 0;
   out_3235005186135283106[56] = 0;
   out_3235005186135283106[57] = 1;
   out_3235005186135283106[58] = 0;
   out_3235005186135283106[59] = 0;
   out_3235005186135283106[60] = 0;
   out_3235005186135283106[61] = 0;
   out_3235005186135283106[62] = 0;
   out_3235005186135283106[63] = 0;
   out_3235005186135283106[64] = 0;
   out_3235005186135283106[65] = 0;
   out_3235005186135283106[66] = dt;
   out_3235005186135283106[67] = 0;
   out_3235005186135283106[68] = 0;
   out_3235005186135283106[69] = 0;
   out_3235005186135283106[70] = 0;
   out_3235005186135283106[71] = 0;
   out_3235005186135283106[72] = 0;
   out_3235005186135283106[73] = 0;
   out_3235005186135283106[74] = 0;
   out_3235005186135283106[75] = 0;
   out_3235005186135283106[76] = 1;
   out_3235005186135283106[77] = 0;
   out_3235005186135283106[78] = 0;
   out_3235005186135283106[79] = 0;
   out_3235005186135283106[80] = 0;
   out_3235005186135283106[81] = 0;
   out_3235005186135283106[82] = 0;
   out_3235005186135283106[83] = 0;
   out_3235005186135283106[84] = 0;
   out_3235005186135283106[85] = dt;
   out_3235005186135283106[86] = 0;
   out_3235005186135283106[87] = 0;
   out_3235005186135283106[88] = 0;
   out_3235005186135283106[89] = 0;
   out_3235005186135283106[90] = 0;
   out_3235005186135283106[91] = 0;
   out_3235005186135283106[92] = 0;
   out_3235005186135283106[93] = 0;
   out_3235005186135283106[94] = 0;
   out_3235005186135283106[95] = 1;
   out_3235005186135283106[96] = 0;
   out_3235005186135283106[97] = 0;
   out_3235005186135283106[98] = 0;
   out_3235005186135283106[99] = 0;
   out_3235005186135283106[100] = 0;
   out_3235005186135283106[101] = 0;
   out_3235005186135283106[102] = 0;
   out_3235005186135283106[103] = 0;
   out_3235005186135283106[104] = dt;
   out_3235005186135283106[105] = 0;
   out_3235005186135283106[106] = 0;
   out_3235005186135283106[107] = 0;
   out_3235005186135283106[108] = 0;
   out_3235005186135283106[109] = 0;
   out_3235005186135283106[110] = 0;
   out_3235005186135283106[111] = 0;
   out_3235005186135283106[112] = 0;
   out_3235005186135283106[113] = 0;
   out_3235005186135283106[114] = 1;
   out_3235005186135283106[115] = 0;
   out_3235005186135283106[116] = 0;
   out_3235005186135283106[117] = 0;
   out_3235005186135283106[118] = 0;
   out_3235005186135283106[119] = 0;
   out_3235005186135283106[120] = 0;
   out_3235005186135283106[121] = 0;
   out_3235005186135283106[122] = 0;
   out_3235005186135283106[123] = 0;
   out_3235005186135283106[124] = 0;
   out_3235005186135283106[125] = 0;
   out_3235005186135283106[126] = 0;
   out_3235005186135283106[127] = 0;
   out_3235005186135283106[128] = 0;
   out_3235005186135283106[129] = 0;
   out_3235005186135283106[130] = 0;
   out_3235005186135283106[131] = 0;
   out_3235005186135283106[132] = 0;
   out_3235005186135283106[133] = 1;
   out_3235005186135283106[134] = 0;
   out_3235005186135283106[135] = 0;
   out_3235005186135283106[136] = 0;
   out_3235005186135283106[137] = 0;
   out_3235005186135283106[138] = 0;
   out_3235005186135283106[139] = 0;
   out_3235005186135283106[140] = 0;
   out_3235005186135283106[141] = 0;
   out_3235005186135283106[142] = 0;
   out_3235005186135283106[143] = 0;
   out_3235005186135283106[144] = 0;
   out_3235005186135283106[145] = 0;
   out_3235005186135283106[146] = 0;
   out_3235005186135283106[147] = 0;
   out_3235005186135283106[148] = 0;
   out_3235005186135283106[149] = 0;
   out_3235005186135283106[150] = 0;
   out_3235005186135283106[151] = 0;
   out_3235005186135283106[152] = 1;
   out_3235005186135283106[153] = 0;
   out_3235005186135283106[154] = 0;
   out_3235005186135283106[155] = 0;
   out_3235005186135283106[156] = 0;
   out_3235005186135283106[157] = 0;
   out_3235005186135283106[158] = 0;
   out_3235005186135283106[159] = 0;
   out_3235005186135283106[160] = 0;
   out_3235005186135283106[161] = 0;
   out_3235005186135283106[162] = 0;
   out_3235005186135283106[163] = 0;
   out_3235005186135283106[164] = 0;
   out_3235005186135283106[165] = 0;
   out_3235005186135283106[166] = 0;
   out_3235005186135283106[167] = 0;
   out_3235005186135283106[168] = 0;
   out_3235005186135283106[169] = 0;
   out_3235005186135283106[170] = 0;
   out_3235005186135283106[171] = 1;
   out_3235005186135283106[172] = 0;
   out_3235005186135283106[173] = 0;
   out_3235005186135283106[174] = 0;
   out_3235005186135283106[175] = 0;
   out_3235005186135283106[176] = 0;
   out_3235005186135283106[177] = 0;
   out_3235005186135283106[178] = 0;
   out_3235005186135283106[179] = 0;
   out_3235005186135283106[180] = 0;
   out_3235005186135283106[181] = 0;
   out_3235005186135283106[182] = 0;
   out_3235005186135283106[183] = 0;
   out_3235005186135283106[184] = 0;
   out_3235005186135283106[185] = 0;
   out_3235005186135283106[186] = 0;
   out_3235005186135283106[187] = 0;
   out_3235005186135283106[188] = 0;
   out_3235005186135283106[189] = 0;
   out_3235005186135283106[190] = 1;
   out_3235005186135283106[191] = 0;
   out_3235005186135283106[192] = 0;
   out_3235005186135283106[193] = 0;
   out_3235005186135283106[194] = 0;
   out_3235005186135283106[195] = 0;
   out_3235005186135283106[196] = 0;
   out_3235005186135283106[197] = 0;
   out_3235005186135283106[198] = 0;
   out_3235005186135283106[199] = 0;
   out_3235005186135283106[200] = 0;
   out_3235005186135283106[201] = 0;
   out_3235005186135283106[202] = 0;
   out_3235005186135283106[203] = 0;
   out_3235005186135283106[204] = 0;
   out_3235005186135283106[205] = 0;
   out_3235005186135283106[206] = 0;
   out_3235005186135283106[207] = 0;
   out_3235005186135283106[208] = 0;
   out_3235005186135283106[209] = 1;
   out_3235005186135283106[210] = 0;
   out_3235005186135283106[211] = 0;
   out_3235005186135283106[212] = 0;
   out_3235005186135283106[213] = 0;
   out_3235005186135283106[214] = 0;
   out_3235005186135283106[215] = 0;
   out_3235005186135283106[216] = 0;
   out_3235005186135283106[217] = 0;
   out_3235005186135283106[218] = 0;
   out_3235005186135283106[219] = 0;
   out_3235005186135283106[220] = 0;
   out_3235005186135283106[221] = 0;
   out_3235005186135283106[222] = 0;
   out_3235005186135283106[223] = 0;
   out_3235005186135283106[224] = 0;
   out_3235005186135283106[225] = 0;
   out_3235005186135283106[226] = 0;
   out_3235005186135283106[227] = 0;
   out_3235005186135283106[228] = 1;
   out_3235005186135283106[229] = 0;
   out_3235005186135283106[230] = 0;
   out_3235005186135283106[231] = 0;
   out_3235005186135283106[232] = 0;
   out_3235005186135283106[233] = 0;
   out_3235005186135283106[234] = 0;
   out_3235005186135283106[235] = 0;
   out_3235005186135283106[236] = 0;
   out_3235005186135283106[237] = 0;
   out_3235005186135283106[238] = 0;
   out_3235005186135283106[239] = 0;
   out_3235005186135283106[240] = 0;
   out_3235005186135283106[241] = 0;
   out_3235005186135283106[242] = 0;
   out_3235005186135283106[243] = 0;
   out_3235005186135283106[244] = 0;
   out_3235005186135283106[245] = 0;
   out_3235005186135283106[246] = 0;
   out_3235005186135283106[247] = 1;
   out_3235005186135283106[248] = 0;
   out_3235005186135283106[249] = 0;
   out_3235005186135283106[250] = 0;
   out_3235005186135283106[251] = 0;
   out_3235005186135283106[252] = 0;
   out_3235005186135283106[253] = 0;
   out_3235005186135283106[254] = 0;
   out_3235005186135283106[255] = 0;
   out_3235005186135283106[256] = 0;
   out_3235005186135283106[257] = 0;
   out_3235005186135283106[258] = 0;
   out_3235005186135283106[259] = 0;
   out_3235005186135283106[260] = 0;
   out_3235005186135283106[261] = 0;
   out_3235005186135283106[262] = 0;
   out_3235005186135283106[263] = 0;
   out_3235005186135283106[264] = 0;
   out_3235005186135283106[265] = 0;
   out_3235005186135283106[266] = 1;
   out_3235005186135283106[267] = 0;
   out_3235005186135283106[268] = 0;
   out_3235005186135283106[269] = 0;
   out_3235005186135283106[270] = 0;
   out_3235005186135283106[271] = 0;
   out_3235005186135283106[272] = 0;
   out_3235005186135283106[273] = 0;
   out_3235005186135283106[274] = 0;
   out_3235005186135283106[275] = 0;
   out_3235005186135283106[276] = 0;
   out_3235005186135283106[277] = 0;
   out_3235005186135283106[278] = 0;
   out_3235005186135283106[279] = 0;
   out_3235005186135283106[280] = 0;
   out_3235005186135283106[281] = 0;
   out_3235005186135283106[282] = 0;
   out_3235005186135283106[283] = 0;
   out_3235005186135283106[284] = 0;
   out_3235005186135283106[285] = 1;
   out_3235005186135283106[286] = 0;
   out_3235005186135283106[287] = 0;
   out_3235005186135283106[288] = 0;
   out_3235005186135283106[289] = 0;
   out_3235005186135283106[290] = 0;
   out_3235005186135283106[291] = 0;
   out_3235005186135283106[292] = 0;
   out_3235005186135283106[293] = 0;
   out_3235005186135283106[294] = 0;
   out_3235005186135283106[295] = 0;
   out_3235005186135283106[296] = 0;
   out_3235005186135283106[297] = 0;
   out_3235005186135283106[298] = 0;
   out_3235005186135283106[299] = 0;
   out_3235005186135283106[300] = 0;
   out_3235005186135283106[301] = 0;
   out_3235005186135283106[302] = 0;
   out_3235005186135283106[303] = 0;
   out_3235005186135283106[304] = 1;
   out_3235005186135283106[305] = 0;
   out_3235005186135283106[306] = 0;
   out_3235005186135283106[307] = 0;
   out_3235005186135283106[308] = 0;
   out_3235005186135283106[309] = 0;
   out_3235005186135283106[310] = 0;
   out_3235005186135283106[311] = 0;
   out_3235005186135283106[312] = 0;
   out_3235005186135283106[313] = 0;
   out_3235005186135283106[314] = 0;
   out_3235005186135283106[315] = 0;
   out_3235005186135283106[316] = 0;
   out_3235005186135283106[317] = 0;
   out_3235005186135283106[318] = 0;
   out_3235005186135283106[319] = 0;
   out_3235005186135283106[320] = 0;
   out_3235005186135283106[321] = 0;
   out_3235005186135283106[322] = 0;
   out_3235005186135283106[323] = 1;
}
void h_4(double *state, double *unused, double *out_5950572713428507074) {
   out_5950572713428507074[0] = state[6] + state[9];
   out_5950572713428507074[1] = state[7] + state[10];
   out_5950572713428507074[2] = state[8] + state[11];
}
void H_4(double *state, double *unused, double *out_8849089379794499089) {
   out_8849089379794499089[0] = 0;
   out_8849089379794499089[1] = 0;
   out_8849089379794499089[2] = 0;
   out_8849089379794499089[3] = 0;
   out_8849089379794499089[4] = 0;
   out_8849089379794499089[5] = 0;
   out_8849089379794499089[6] = 1;
   out_8849089379794499089[7] = 0;
   out_8849089379794499089[8] = 0;
   out_8849089379794499089[9] = 1;
   out_8849089379794499089[10] = 0;
   out_8849089379794499089[11] = 0;
   out_8849089379794499089[12] = 0;
   out_8849089379794499089[13] = 0;
   out_8849089379794499089[14] = 0;
   out_8849089379794499089[15] = 0;
   out_8849089379794499089[16] = 0;
   out_8849089379794499089[17] = 0;
   out_8849089379794499089[18] = 0;
   out_8849089379794499089[19] = 0;
   out_8849089379794499089[20] = 0;
   out_8849089379794499089[21] = 0;
   out_8849089379794499089[22] = 0;
   out_8849089379794499089[23] = 0;
   out_8849089379794499089[24] = 0;
   out_8849089379794499089[25] = 1;
   out_8849089379794499089[26] = 0;
   out_8849089379794499089[27] = 0;
   out_8849089379794499089[28] = 1;
   out_8849089379794499089[29] = 0;
   out_8849089379794499089[30] = 0;
   out_8849089379794499089[31] = 0;
   out_8849089379794499089[32] = 0;
   out_8849089379794499089[33] = 0;
   out_8849089379794499089[34] = 0;
   out_8849089379794499089[35] = 0;
   out_8849089379794499089[36] = 0;
   out_8849089379794499089[37] = 0;
   out_8849089379794499089[38] = 0;
   out_8849089379794499089[39] = 0;
   out_8849089379794499089[40] = 0;
   out_8849089379794499089[41] = 0;
   out_8849089379794499089[42] = 0;
   out_8849089379794499089[43] = 0;
   out_8849089379794499089[44] = 1;
   out_8849089379794499089[45] = 0;
   out_8849089379794499089[46] = 0;
   out_8849089379794499089[47] = 1;
   out_8849089379794499089[48] = 0;
   out_8849089379794499089[49] = 0;
   out_8849089379794499089[50] = 0;
   out_8849089379794499089[51] = 0;
   out_8849089379794499089[52] = 0;
   out_8849089379794499089[53] = 0;
}
void h_10(double *state, double *unused, double *out_2179341978768680660) {
   out_2179341978768680660[0] = 9.8100000000000005*sin(state[1]) - state[4]*state[8] + state[5]*state[7] + state[12] + state[15];
   out_2179341978768680660[1] = -9.8100000000000005*sin(state[0])*cos(state[1]) + state[3]*state[8] - state[5]*state[6] + state[13] + state[16];
   out_2179341978768680660[2] = -9.8100000000000005*cos(state[0])*cos(state[1]) - state[3]*state[7] + state[4]*state[6] + state[14] + state[17];
}
void H_10(double *state, double *unused, double *out_2186201478439983772) {
   out_2186201478439983772[0] = 0;
   out_2186201478439983772[1] = 9.8100000000000005*cos(state[1]);
   out_2186201478439983772[2] = 0;
   out_2186201478439983772[3] = 0;
   out_2186201478439983772[4] = -state[8];
   out_2186201478439983772[5] = state[7];
   out_2186201478439983772[6] = 0;
   out_2186201478439983772[7] = state[5];
   out_2186201478439983772[8] = -state[4];
   out_2186201478439983772[9] = 0;
   out_2186201478439983772[10] = 0;
   out_2186201478439983772[11] = 0;
   out_2186201478439983772[12] = 1;
   out_2186201478439983772[13] = 0;
   out_2186201478439983772[14] = 0;
   out_2186201478439983772[15] = 1;
   out_2186201478439983772[16] = 0;
   out_2186201478439983772[17] = 0;
   out_2186201478439983772[18] = -9.8100000000000005*cos(state[0])*cos(state[1]);
   out_2186201478439983772[19] = 9.8100000000000005*sin(state[0])*sin(state[1]);
   out_2186201478439983772[20] = 0;
   out_2186201478439983772[21] = state[8];
   out_2186201478439983772[22] = 0;
   out_2186201478439983772[23] = -state[6];
   out_2186201478439983772[24] = -state[5];
   out_2186201478439983772[25] = 0;
   out_2186201478439983772[26] = state[3];
   out_2186201478439983772[27] = 0;
   out_2186201478439983772[28] = 0;
   out_2186201478439983772[29] = 0;
   out_2186201478439983772[30] = 0;
   out_2186201478439983772[31] = 1;
   out_2186201478439983772[32] = 0;
   out_2186201478439983772[33] = 0;
   out_2186201478439983772[34] = 1;
   out_2186201478439983772[35] = 0;
   out_2186201478439983772[36] = 9.8100000000000005*sin(state[0])*cos(state[1]);
   out_2186201478439983772[37] = 9.8100000000000005*sin(state[1])*cos(state[0]);
   out_2186201478439983772[38] = 0;
   out_2186201478439983772[39] = -state[7];
   out_2186201478439983772[40] = state[6];
   out_2186201478439983772[41] = 0;
   out_2186201478439983772[42] = state[4];
   out_2186201478439983772[43] = -state[3];
   out_2186201478439983772[44] = 0;
   out_2186201478439983772[45] = 0;
   out_2186201478439983772[46] = 0;
   out_2186201478439983772[47] = 0;
   out_2186201478439983772[48] = 0;
   out_2186201478439983772[49] = 0;
   out_2186201478439983772[50] = 1;
   out_2186201478439983772[51] = 0;
   out_2186201478439983772[52] = 0;
   out_2186201478439983772[53] = 1;
}
void h_13(double *state, double *unused, double *out_7892243649341508951) {
   out_7892243649341508951[0] = state[3];
   out_7892243649341508951[1] = state[4];
   out_7892243649341508951[2] = state[5];
}
void H_13(double *state, double *unused, double *out_5015333916491975065) {
   out_5015333916491975065[0] = 0;
   out_5015333916491975065[1] = 0;
   out_5015333916491975065[2] = 0;
   out_5015333916491975065[3] = 1;
   out_5015333916491975065[4] = 0;
   out_5015333916491975065[5] = 0;
   out_5015333916491975065[6] = 0;
   out_5015333916491975065[7] = 0;
   out_5015333916491975065[8] = 0;
   out_5015333916491975065[9] = 0;
   out_5015333916491975065[10] = 0;
   out_5015333916491975065[11] = 0;
   out_5015333916491975065[12] = 0;
   out_5015333916491975065[13] = 0;
   out_5015333916491975065[14] = 0;
   out_5015333916491975065[15] = 0;
   out_5015333916491975065[16] = 0;
   out_5015333916491975065[17] = 0;
   out_5015333916491975065[18] = 0;
   out_5015333916491975065[19] = 0;
   out_5015333916491975065[20] = 0;
   out_5015333916491975065[21] = 0;
   out_5015333916491975065[22] = 1;
   out_5015333916491975065[23] = 0;
   out_5015333916491975065[24] = 0;
   out_5015333916491975065[25] = 0;
   out_5015333916491975065[26] = 0;
   out_5015333916491975065[27] = 0;
   out_5015333916491975065[28] = 0;
   out_5015333916491975065[29] = 0;
   out_5015333916491975065[30] = 0;
   out_5015333916491975065[31] = 0;
   out_5015333916491975065[32] = 0;
   out_5015333916491975065[33] = 0;
   out_5015333916491975065[34] = 0;
   out_5015333916491975065[35] = 0;
   out_5015333916491975065[36] = 0;
   out_5015333916491975065[37] = 0;
   out_5015333916491975065[38] = 0;
   out_5015333916491975065[39] = 0;
   out_5015333916491975065[40] = 0;
   out_5015333916491975065[41] = 1;
   out_5015333916491975065[42] = 0;
   out_5015333916491975065[43] = 0;
   out_5015333916491975065[44] = 0;
   out_5015333916491975065[45] = 0;
   out_5015333916491975065[46] = 0;
   out_5015333916491975065[47] = 0;
   out_5015333916491975065[48] = 0;
   out_5015333916491975065[49] = 0;
   out_5015333916491975065[50] = 0;
   out_5015333916491975065[51] = 0;
   out_5015333916491975065[52] = 0;
   out_5015333916491975065[53] = 0;
}
void h_14(double *state, double *unused, double *out_7785076606087955983) {
   out_7785076606087955983[0] = state[6];
   out_7785076606087955983[1] = state[7];
   out_7785076606087955983[2] = state[8];
}
void H_14(double *state, double *unused, double *out_5766300947499126793) {
   out_5766300947499126793[0] = 0;
   out_5766300947499126793[1] = 0;
   out_5766300947499126793[2] = 0;
   out_5766300947499126793[3] = 0;
   out_5766300947499126793[4] = 0;
   out_5766300947499126793[5] = 0;
   out_5766300947499126793[6] = 1;
   out_5766300947499126793[7] = 0;
   out_5766300947499126793[8] = 0;
   out_5766300947499126793[9] = 0;
   out_5766300947499126793[10] = 0;
   out_5766300947499126793[11] = 0;
   out_5766300947499126793[12] = 0;
   out_5766300947499126793[13] = 0;
   out_5766300947499126793[14] = 0;
   out_5766300947499126793[15] = 0;
   out_5766300947499126793[16] = 0;
   out_5766300947499126793[17] = 0;
   out_5766300947499126793[18] = 0;
   out_5766300947499126793[19] = 0;
   out_5766300947499126793[20] = 0;
   out_5766300947499126793[21] = 0;
   out_5766300947499126793[22] = 0;
   out_5766300947499126793[23] = 0;
   out_5766300947499126793[24] = 0;
   out_5766300947499126793[25] = 1;
   out_5766300947499126793[26] = 0;
   out_5766300947499126793[27] = 0;
   out_5766300947499126793[28] = 0;
   out_5766300947499126793[29] = 0;
   out_5766300947499126793[30] = 0;
   out_5766300947499126793[31] = 0;
   out_5766300947499126793[32] = 0;
   out_5766300947499126793[33] = 0;
   out_5766300947499126793[34] = 0;
   out_5766300947499126793[35] = 0;
   out_5766300947499126793[36] = 0;
   out_5766300947499126793[37] = 0;
   out_5766300947499126793[38] = 0;
   out_5766300947499126793[39] = 0;
   out_5766300947499126793[40] = 0;
   out_5766300947499126793[41] = 0;
   out_5766300947499126793[42] = 0;
   out_5766300947499126793[43] = 0;
   out_5766300947499126793[44] = 1;
   out_5766300947499126793[45] = 0;
   out_5766300947499126793[46] = 0;
   out_5766300947499126793[47] = 0;
   out_5766300947499126793[48] = 0;
   out_5766300947499126793[49] = 0;
   out_5766300947499126793[50] = 0;
   out_5766300947499126793[51] = 0;
   out_5766300947499126793[52] = 0;
   out_5766300947499126793[53] = 0;
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
void pose_err_fun(double *nom_x, double *delta_x, double *out_1112526487984770436) {
  err_fun(nom_x, delta_x, out_1112526487984770436);
}
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2885793937532541180) {
  inv_err_fun(nom_x, true_x, out_2885793937532541180);
}
void pose_H_mod_fun(double *state, double *out_5077282169950591517) {
  H_mod_fun(state, out_5077282169950591517);
}
void pose_f_fun(double *state, double dt, double *out_3821476900675952296) {
  f_fun(state,  dt, out_3821476900675952296);
}
void pose_F_fun(double *state, double dt, double *out_3235005186135283106) {
  F_fun(state,  dt, out_3235005186135283106);
}
void pose_h_4(double *state, double *unused, double *out_5950572713428507074) {
  h_4(state, unused, out_5950572713428507074);
}
void pose_H_4(double *state, double *unused, double *out_8849089379794499089) {
  H_4(state, unused, out_8849089379794499089);
}
void pose_h_10(double *state, double *unused, double *out_2179341978768680660) {
  h_10(state, unused, out_2179341978768680660);
}
void pose_H_10(double *state, double *unused, double *out_2186201478439983772) {
  H_10(state, unused, out_2186201478439983772);
}
void pose_h_13(double *state, double *unused, double *out_7892243649341508951) {
  h_13(state, unused, out_7892243649341508951);
}
void pose_H_13(double *state, double *unused, double *out_5015333916491975065) {
  H_13(state, unused, out_5015333916491975065);
}
void pose_h_14(double *state, double *unused, double *out_7785076606087955983) {
  h_14(state, unused, out_7785076606087955983);
}
void pose_H_14(double *state, double *unused, double *out_5766300947499126793) {
  H_14(state, unused, out_5766300947499126793);
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
