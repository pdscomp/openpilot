#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_3549856774171141684);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2925455247541166295);
void pose_H_mod_fun(double *state, double *out_210273699827536735);
void pose_f_fun(double *state, double dt, double *out_3137858729334731425);
void pose_F_fun(double *state, double dt, double *out_6711386679040656198);
void pose_h_4(double *state, double *unused, double *out_5605177345384889420);
void pose_H_4(double *state, double *unused, double *out_845238850510018990);
void pose_h_10(double *state, double *unused, double *out_2811354315479325630);
void pose_H_10(double *state, double *unused, double *out_8075210817719937862);
void pose_h_13(double *state, double *unused, double *out_4621388109736156056);
void pose_H_13(double *state, double *unused, double *out_2367034974822313811);
void pose_h_14(double *state, double *unused, double *out_267354547646598181);
void pose_H_14(double *state, double *unused, double *out_3928027282805391286);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}