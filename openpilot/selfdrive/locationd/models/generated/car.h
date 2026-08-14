#pragma once
#include "rednose/helpers/ekf.h"
extern "C" {
void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea);
void car_err_fun(double *nom_x, double *delta_x, double *out_3157126624170581198);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_4673666440688567315);
void car_H_mod_fun(double *state, double *out_7076448425335990647);
void car_f_fun(double *state, double dt, double *out_9009949425942935353);
void car_F_fun(double *state, double dt, double *out_6772166287624075049);
void car_h_25(double *state, double *unused, double *out_3147464506924368092);
void car_H_25(double *state, double *unused, double *out_736028625381073427);
void car_h_24(double *state, double *unused, double *out_2685318194937829437);
void car_H_24(double *state, double *unused, double *out_4537465657531939167);
void car_h_30(double *state, double *unused, double *out_7536014309964511083);
void car_H_30(double *state, double *unused, double *out_1782304333126175200);
void car_h_26(double *state, double *unused, double *out_5152744849751302577);
void car_H_26(double *state, double *unused, double *out_4477531944255129651);
void car_h_27(double *state, double *unused, double *out_1775736216339172739);
void car_H_27(double *state, double *unused, double *out_392458978674249711);
void car_h_29(double *state, double *unused, double *out_5909056496084194888);
void car_H_29(double *state, double *unused, double *out_2292535677440567384);
void car_h_28(double *state, double *unused, double *out_6777409484000018384);
void car_H_28(double *state, double *unused, double *out_2789863339628963190);
void car_h_31(double *state, double *unused, double *out_8711451640308888361);
void car_H_31(double *state, double *unused, double *out_5103740046488481127);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}