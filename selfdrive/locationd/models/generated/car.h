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
void car_err_fun(double *nom_x, double *delta_x, double *out_8928467092611092744);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8458266257392376640);
void car_H_mod_fun(double *state, double *out_6729596904764973580);
void car_f_fun(double *state, double dt, double *out_5070065485852511286);
void car_F_fun(double *state, double dt, double *out_8336486238807882069);
void car_h_25(double *state, double *unused, double *out_1918519873367899214);
void car_H_25(double *state, double *unused, double *out_8623128896152181528);
void car_h_24(double *state, double *unused, double *out_4068985978219487213);
void car_H_24(double *state, double *unused, double *out_604936289917013697);
void car_h_30(double *state, double *unused, double *out_2075706541719028359);
void car_H_30(double *state, double *unused, double *out_5295918847429761890);
void car_h_26(double *state, double *unused, double *out_47122023652287635);
void car_H_26(double *state, double *unused, double *out_6082111858683313864);
void car_h_27(double *state, double *unused, double *out_6880899871049192446);
void car_H_27(double *state, double *unused, double *out_7519512918613705107);
void car_h_29(double *state, double *unused, double *out_3933173880693707485);
void car_H_29(double *state, double *unused, double *out_5806150191744154074);
void car_h_28(double *state, double *unused, double *out_1147213510920962491);
void car_H_28(double *state, double *unused, double *out_723751174674623500);
void car_h_31(double *state, double *unused, double *out_713380529375319027);
void car_H_31(double *state, double *unused, double *out_8592482934275221100);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}