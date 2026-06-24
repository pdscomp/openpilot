#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void pose_update_4(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_10(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_13(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_update_14(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void pose_err_fun(double *nom_x, double *delta_x, double *out_1112526487984770436);
void pose_inv_err_fun(double *nom_x, double *true_x, double *out_2885793937532541180);
void pose_H_mod_fun(double *state, double *out_5077282169950591517);
void pose_f_fun(double *state, double dt, double *out_3821476900675952296);
void pose_F_fun(double *state, double dt, double *out_3235005186135283106);
void pose_h_4(double *state, double *unused, double *out_5950572713428507074);
void pose_H_4(double *state, double *unused, double *out_8849089379794499089);
void pose_h_10(double *state, double *unused, double *out_2179341978768680660);
void pose_H_10(double *state, double *unused, double *out_2186201478439983772);
void pose_h_13(double *state, double *unused, double *out_7892243649341508951);
void pose_H_13(double *state, double *unused, double *out_5015333916491975065);
void pose_h_14(double *state, double *unused, double *out_7785076606087955983);
void pose_H_14(double *state, double *unused, double *out_5766300947499126793);
void pose_predict(double *in_x, double *in_P, double *in_Q, double dt);
}