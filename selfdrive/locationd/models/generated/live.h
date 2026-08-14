#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void live_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_9(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_12(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_35(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_32(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_update_33(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void live_H(double *in_vec, double *out_5994229723236113210);
void live_err_fun(double *nom_x, double *delta_x, double *out_501756441084002475);
void live_inv_err_fun(double *nom_x, double *true_x, double *out_8765487857309980659);
void live_H_mod_fun(double *state, double *out_7442190330464269811);
void live_f_fun(double *state, double dt, double *out_576377093012955591);
void live_F_fun(double *state, double dt, double *out_8887476980993821717);
void live_h_4(double *state, double *unused, double *out_2509567471795557399);
void live_H_4(double *state, double *unused, double *out_4206762896513139488);
void live_h_9(double *state, double *unused, double *out_1341243896040479391);
void live_H_9(double *state, double *unused, double *out_6952762241931964658);
void live_h_10(double *state, double *unused, double *out_5174407441893473423);
void live_H_10(double *state, double *unused, double *out_18635511288206505);
void live_h_12(double *state, double *unused, double *out_7369860149460724559);
void live_H_12(double *state, double *unused, double *out_6572852863513961636);
void live_h_35(double *state, double *unused, double *out_6124942867874314771);
void live_H_35(double *state, double *unused, double *out_3827289831188947927);
void live_h_32(double *state, double *unused, double *out_4363505666386922559);
void live_H_32(double *state, double *unused, double *out_279445491201789198);
void live_h_13(double *state, double *unused, double *out_8628529975430544573);
void live_H_13(double *state, double *unused, double *out_4874493919612819204);
void live_h_14(double *state, double *unused, double *out_1341243896040479391);
void live_H_14(double *state, double *unused, double *out_6952762241931964658);
void live_h_33(double *state, double *unused, double *out_8653268643023109756);
void live_H_33(double *state, double *unused, double *out_676732826550090323);
void live_predict(double *in_x, double *in_P, double *in_Q, double dt);
}