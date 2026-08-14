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
void live_H(double *in_vec, double *out_1599662452165301280);
void live_err_fun(double *nom_x, double *delta_x, double *out_3622568814855789465);
void live_inv_err_fun(double *nom_x, double *true_x, double *out_2929493076331753744);
void live_H_mod_fun(double *state, double *out_6401502196823900770);
void live_f_fun(double *state, double dt, double *out_5183724908727418553);
void live_F_fun(double *state, double dt, double *out_4128122670321329708);
void live_h_4(double *state, double *unused, double *out_6170941837536718469);
void live_H_4(double *state, double *unused, double *out_4546115748893698275);
void live_h_9(double *state, double *unused, double *out_6398692708014289450);
void live_H_9(double *state, double *unused, double *out_2741103186370749195);
void live_h_10(double *state, double *unused, double *out_3794258564083891362);
void live_H_10(double *state, double *unused, double *out_342420580824209973);
void live_h_12(double *state, double *unused, double *out_7213213050045898667);
void live_H_12(double *state, double *unused, double *out_7519369947773120345);
void live_h_35(double *state, double *unused, double *out_3758950511116105885);
void live_H_35(double *state, double *unused, double *out_1179453691521090899);
void live_h_32(double *state, double *unused, double *out_4019796114055821146);
void live_H_32(double *state, double *unused, double *out_332422268614822733);
void live_h_13(double *state, double *unused, double *out_6837557338946741780);
void live_H_13(double *state, double *unused, double *out_3190256376591046541);
void live_h_14(double *state, double *unused, double *out_6398692708014289450);
void live_H_14(double *state, double *unused, double *out_2741103186370749195);
void live_h_33(double *state, double *unused, double *out_2771746113833789026);
void live_H_33(double *state, double *unused, double *out_1971103313117766705);
void live_predict(double *in_x, double *in_P, double *in_Q, double dt);
}