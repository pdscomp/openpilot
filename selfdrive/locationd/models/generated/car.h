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
void car_err_fun(double *nom_x, double *delta_x, double *out_6958026418178170623);
void car_inv_err_fun(double *nom_x, double *true_x, double *out_5491229326505777206);
void car_H_mod_fun(double *state, double *out_8036634064303217169);
void car_f_fun(double *state, double dt, double *out_1838120844356692343);
void car_F_fun(double *state, double dt, double *out_1773791557638419892);
void car_h_25(double *state, double *unused, double *out_4124099070478046083);
void car_H_25(double *state, double *unused, double *out_2629196256626097774);
void car_h_24(double *state, double *unused, double *out_9130188661917389180);
void car_H_24(double *state, double *unused, double *out_928729342176225362);
void car_h_30(double *state, double *unused, double *out_4269618894698701054);
void car_H_30(double *state, double *unused, double *out_110863298118849147);
void car_h_26(double *state, double *unused, double *out_2392949840869179807);
void car_H_26(double *state, double *unused, double *out_6370699575500153998);
void car_h_27(double *state, double *unused, double *out_9125772338138891728);
void car_H_27(double *state, double *unused, double *out_4933298515569762755);
void car_h_29(double *state, double *unused, double *out_6373245745215174927);
void car_H_29(double *state, double *unused, double *out_6646661242439313788);
void car_h_28(double *state, double *unused, double *out_720115518494665075);
void car_H_28(double *state, double *unused, double *out_4683030970873987537);
void car_h_31(double *state, double *unused, double *out_486649542224023733);
void car_H_31(double *state, double *unused, double *out_2598550294749137346);
void car_predict(double *in_x, double *in_P, double *in_Q, double dt);
void car_set_mass(double x);
void car_set_rotational_inertia(double x);
void car_set_center_to_front(double x);
void car_set_center_to_rear(double x);
void car_set_stiffness_front(double x);
void car_set_stiffness_rear(double x);
}