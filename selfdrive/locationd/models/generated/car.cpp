#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                      Code generated with SymPy 1.14.0                      *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_8928467092611092744) {
   out_8928467092611092744[0] = delta_x[0] + nom_x[0];
   out_8928467092611092744[1] = delta_x[1] + nom_x[1];
   out_8928467092611092744[2] = delta_x[2] + nom_x[2];
   out_8928467092611092744[3] = delta_x[3] + nom_x[3];
   out_8928467092611092744[4] = delta_x[4] + nom_x[4];
   out_8928467092611092744[5] = delta_x[5] + nom_x[5];
   out_8928467092611092744[6] = delta_x[6] + nom_x[6];
   out_8928467092611092744[7] = delta_x[7] + nom_x[7];
   out_8928467092611092744[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_8458266257392376640) {
   out_8458266257392376640[0] = -nom_x[0] + true_x[0];
   out_8458266257392376640[1] = -nom_x[1] + true_x[1];
   out_8458266257392376640[2] = -nom_x[2] + true_x[2];
   out_8458266257392376640[3] = -nom_x[3] + true_x[3];
   out_8458266257392376640[4] = -nom_x[4] + true_x[4];
   out_8458266257392376640[5] = -nom_x[5] + true_x[5];
   out_8458266257392376640[6] = -nom_x[6] + true_x[6];
   out_8458266257392376640[7] = -nom_x[7] + true_x[7];
   out_8458266257392376640[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_6729596904764973580) {
   out_6729596904764973580[0] = 1.0;
   out_6729596904764973580[1] = 0.0;
   out_6729596904764973580[2] = 0.0;
   out_6729596904764973580[3] = 0.0;
   out_6729596904764973580[4] = 0.0;
   out_6729596904764973580[5] = 0.0;
   out_6729596904764973580[6] = 0.0;
   out_6729596904764973580[7] = 0.0;
   out_6729596904764973580[8] = 0.0;
   out_6729596904764973580[9] = 0.0;
   out_6729596904764973580[10] = 1.0;
   out_6729596904764973580[11] = 0.0;
   out_6729596904764973580[12] = 0.0;
   out_6729596904764973580[13] = 0.0;
   out_6729596904764973580[14] = 0.0;
   out_6729596904764973580[15] = 0.0;
   out_6729596904764973580[16] = 0.0;
   out_6729596904764973580[17] = 0.0;
   out_6729596904764973580[18] = 0.0;
   out_6729596904764973580[19] = 0.0;
   out_6729596904764973580[20] = 1.0;
   out_6729596904764973580[21] = 0.0;
   out_6729596904764973580[22] = 0.0;
   out_6729596904764973580[23] = 0.0;
   out_6729596904764973580[24] = 0.0;
   out_6729596904764973580[25] = 0.0;
   out_6729596904764973580[26] = 0.0;
   out_6729596904764973580[27] = 0.0;
   out_6729596904764973580[28] = 0.0;
   out_6729596904764973580[29] = 0.0;
   out_6729596904764973580[30] = 1.0;
   out_6729596904764973580[31] = 0.0;
   out_6729596904764973580[32] = 0.0;
   out_6729596904764973580[33] = 0.0;
   out_6729596904764973580[34] = 0.0;
   out_6729596904764973580[35] = 0.0;
   out_6729596904764973580[36] = 0.0;
   out_6729596904764973580[37] = 0.0;
   out_6729596904764973580[38] = 0.0;
   out_6729596904764973580[39] = 0.0;
   out_6729596904764973580[40] = 1.0;
   out_6729596904764973580[41] = 0.0;
   out_6729596904764973580[42] = 0.0;
   out_6729596904764973580[43] = 0.0;
   out_6729596904764973580[44] = 0.0;
   out_6729596904764973580[45] = 0.0;
   out_6729596904764973580[46] = 0.0;
   out_6729596904764973580[47] = 0.0;
   out_6729596904764973580[48] = 0.0;
   out_6729596904764973580[49] = 0.0;
   out_6729596904764973580[50] = 1.0;
   out_6729596904764973580[51] = 0.0;
   out_6729596904764973580[52] = 0.0;
   out_6729596904764973580[53] = 0.0;
   out_6729596904764973580[54] = 0.0;
   out_6729596904764973580[55] = 0.0;
   out_6729596904764973580[56] = 0.0;
   out_6729596904764973580[57] = 0.0;
   out_6729596904764973580[58] = 0.0;
   out_6729596904764973580[59] = 0.0;
   out_6729596904764973580[60] = 1.0;
   out_6729596904764973580[61] = 0.0;
   out_6729596904764973580[62] = 0.0;
   out_6729596904764973580[63] = 0.0;
   out_6729596904764973580[64] = 0.0;
   out_6729596904764973580[65] = 0.0;
   out_6729596904764973580[66] = 0.0;
   out_6729596904764973580[67] = 0.0;
   out_6729596904764973580[68] = 0.0;
   out_6729596904764973580[69] = 0.0;
   out_6729596904764973580[70] = 1.0;
   out_6729596904764973580[71] = 0.0;
   out_6729596904764973580[72] = 0.0;
   out_6729596904764973580[73] = 0.0;
   out_6729596904764973580[74] = 0.0;
   out_6729596904764973580[75] = 0.0;
   out_6729596904764973580[76] = 0.0;
   out_6729596904764973580[77] = 0.0;
   out_6729596904764973580[78] = 0.0;
   out_6729596904764973580[79] = 0.0;
   out_6729596904764973580[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_5070065485852511286) {
   out_5070065485852511286[0] = state[0];
   out_5070065485852511286[1] = state[1];
   out_5070065485852511286[2] = state[2];
   out_5070065485852511286[3] = state[3];
   out_5070065485852511286[4] = state[4];
   out_5070065485852511286[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_5070065485852511286[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_5070065485852511286[7] = state[7];
   out_5070065485852511286[8] = state[8];
}
void F_fun(double *state, double dt, double *out_8336486238807882069) {
   out_8336486238807882069[0] = 1;
   out_8336486238807882069[1] = 0;
   out_8336486238807882069[2] = 0;
   out_8336486238807882069[3] = 0;
   out_8336486238807882069[4] = 0;
   out_8336486238807882069[5] = 0;
   out_8336486238807882069[6] = 0;
   out_8336486238807882069[7] = 0;
   out_8336486238807882069[8] = 0;
   out_8336486238807882069[9] = 0;
   out_8336486238807882069[10] = 1;
   out_8336486238807882069[11] = 0;
   out_8336486238807882069[12] = 0;
   out_8336486238807882069[13] = 0;
   out_8336486238807882069[14] = 0;
   out_8336486238807882069[15] = 0;
   out_8336486238807882069[16] = 0;
   out_8336486238807882069[17] = 0;
   out_8336486238807882069[18] = 0;
   out_8336486238807882069[19] = 0;
   out_8336486238807882069[20] = 1;
   out_8336486238807882069[21] = 0;
   out_8336486238807882069[22] = 0;
   out_8336486238807882069[23] = 0;
   out_8336486238807882069[24] = 0;
   out_8336486238807882069[25] = 0;
   out_8336486238807882069[26] = 0;
   out_8336486238807882069[27] = 0;
   out_8336486238807882069[28] = 0;
   out_8336486238807882069[29] = 0;
   out_8336486238807882069[30] = 1;
   out_8336486238807882069[31] = 0;
   out_8336486238807882069[32] = 0;
   out_8336486238807882069[33] = 0;
   out_8336486238807882069[34] = 0;
   out_8336486238807882069[35] = 0;
   out_8336486238807882069[36] = 0;
   out_8336486238807882069[37] = 0;
   out_8336486238807882069[38] = 0;
   out_8336486238807882069[39] = 0;
   out_8336486238807882069[40] = 1;
   out_8336486238807882069[41] = 0;
   out_8336486238807882069[42] = 0;
   out_8336486238807882069[43] = 0;
   out_8336486238807882069[44] = 0;
   out_8336486238807882069[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_8336486238807882069[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_8336486238807882069[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_8336486238807882069[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_8336486238807882069[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_8336486238807882069[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_8336486238807882069[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_8336486238807882069[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_8336486238807882069[53] = -9.8100000000000005*dt;
   out_8336486238807882069[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_8336486238807882069[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_8336486238807882069[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_8336486238807882069[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_8336486238807882069[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_8336486238807882069[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_8336486238807882069[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_8336486238807882069[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_8336486238807882069[62] = 0;
   out_8336486238807882069[63] = 0;
   out_8336486238807882069[64] = 0;
   out_8336486238807882069[65] = 0;
   out_8336486238807882069[66] = 0;
   out_8336486238807882069[67] = 0;
   out_8336486238807882069[68] = 0;
   out_8336486238807882069[69] = 0;
   out_8336486238807882069[70] = 1;
   out_8336486238807882069[71] = 0;
   out_8336486238807882069[72] = 0;
   out_8336486238807882069[73] = 0;
   out_8336486238807882069[74] = 0;
   out_8336486238807882069[75] = 0;
   out_8336486238807882069[76] = 0;
   out_8336486238807882069[77] = 0;
   out_8336486238807882069[78] = 0;
   out_8336486238807882069[79] = 0;
   out_8336486238807882069[80] = 1;
}
void h_25(double *state, double *unused, double *out_1918519873367899214) {
   out_1918519873367899214[0] = state[6];
}
void H_25(double *state, double *unused, double *out_8623128896152181528) {
   out_8623128896152181528[0] = 0;
   out_8623128896152181528[1] = 0;
   out_8623128896152181528[2] = 0;
   out_8623128896152181528[3] = 0;
   out_8623128896152181528[4] = 0;
   out_8623128896152181528[5] = 0;
   out_8623128896152181528[6] = 1;
   out_8623128896152181528[7] = 0;
   out_8623128896152181528[8] = 0;
}
void h_24(double *state, double *unused, double *out_4068985978219487213) {
   out_4068985978219487213[0] = state[4];
   out_4068985978219487213[1] = state[5];
}
void H_24(double *state, double *unused, double *out_604936289917013697) {
   out_604936289917013697[0] = 0;
   out_604936289917013697[1] = 0;
   out_604936289917013697[2] = 0;
   out_604936289917013697[3] = 0;
   out_604936289917013697[4] = 1;
   out_604936289917013697[5] = 0;
   out_604936289917013697[6] = 0;
   out_604936289917013697[7] = 0;
   out_604936289917013697[8] = 0;
   out_604936289917013697[9] = 0;
   out_604936289917013697[10] = 0;
   out_604936289917013697[11] = 0;
   out_604936289917013697[12] = 0;
   out_604936289917013697[13] = 0;
   out_604936289917013697[14] = 1;
   out_604936289917013697[15] = 0;
   out_604936289917013697[16] = 0;
   out_604936289917013697[17] = 0;
}
void h_30(double *state, double *unused, double *out_2075706541719028359) {
   out_2075706541719028359[0] = state[4];
}
void H_30(double *state, double *unused, double *out_5295918847429761890) {
   out_5295918847429761890[0] = 0;
   out_5295918847429761890[1] = 0;
   out_5295918847429761890[2] = 0;
   out_5295918847429761890[3] = 0;
   out_5295918847429761890[4] = 1;
   out_5295918847429761890[5] = 0;
   out_5295918847429761890[6] = 0;
   out_5295918847429761890[7] = 0;
   out_5295918847429761890[8] = 0;
}
void h_26(double *state, double *unused, double *out_47122023652287635) {
   out_47122023652287635[0] = state[7];
}
void H_26(double *state, double *unused, double *out_6082111858683313864) {
   out_6082111858683313864[0] = 0;
   out_6082111858683313864[1] = 0;
   out_6082111858683313864[2] = 0;
   out_6082111858683313864[3] = 0;
   out_6082111858683313864[4] = 0;
   out_6082111858683313864[5] = 0;
   out_6082111858683313864[6] = 0;
   out_6082111858683313864[7] = 1;
   out_6082111858683313864[8] = 0;
}
void h_27(double *state, double *unused, double *out_6880899871049192446) {
   out_6880899871049192446[0] = state[3];
}
void H_27(double *state, double *unused, double *out_7519512918613705107) {
   out_7519512918613705107[0] = 0;
   out_7519512918613705107[1] = 0;
   out_7519512918613705107[2] = 0;
   out_7519512918613705107[3] = 1;
   out_7519512918613705107[4] = 0;
   out_7519512918613705107[5] = 0;
   out_7519512918613705107[6] = 0;
   out_7519512918613705107[7] = 0;
   out_7519512918613705107[8] = 0;
}
void h_29(double *state, double *unused, double *out_3933173880693707485) {
   out_3933173880693707485[0] = state[1];
}
void H_29(double *state, double *unused, double *out_5806150191744154074) {
   out_5806150191744154074[0] = 0;
   out_5806150191744154074[1] = 1;
   out_5806150191744154074[2] = 0;
   out_5806150191744154074[3] = 0;
   out_5806150191744154074[4] = 0;
   out_5806150191744154074[5] = 0;
   out_5806150191744154074[6] = 0;
   out_5806150191744154074[7] = 0;
   out_5806150191744154074[8] = 0;
}
void h_28(double *state, double *unused, double *out_1147213510920962491) {
   out_1147213510920962491[0] = state[0];
}
void H_28(double *state, double *unused, double *out_723751174674623500) {
   out_723751174674623500[0] = 1;
   out_723751174674623500[1] = 0;
   out_723751174674623500[2] = 0;
   out_723751174674623500[3] = 0;
   out_723751174674623500[4] = 0;
   out_723751174674623500[5] = 0;
   out_723751174674623500[6] = 0;
   out_723751174674623500[7] = 0;
   out_723751174674623500[8] = 0;
}
void h_31(double *state, double *unused, double *out_713380529375319027) {
   out_713380529375319027[0] = state[8];
}
void H_31(double *state, double *unused, double *out_8592482934275221100) {
   out_8592482934275221100[0] = 0;
   out_8592482934275221100[1] = 0;
   out_8592482934275221100[2] = 0;
   out_8592482934275221100[3] = 0;
   out_8592482934275221100[4] = 0;
   out_8592482934275221100[5] = 0;
   out_8592482934275221100[6] = 0;
   out_8592482934275221100[7] = 0;
   out_8592482934275221100[8] = 1;
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

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_8928467092611092744) {
  err_fun(nom_x, delta_x, out_8928467092611092744);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_8458266257392376640) {
  inv_err_fun(nom_x, true_x, out_8458266257392376640);
}
void car_H_mod_fun(double *state, double *out_6729596904764973580) {
  H_mod_fun(state, out_6729596904764973580);
}
void car_f_fun(double *state, double dt, double *out_5070065485852511286) {
  f_fun(state,  dt, out_5070065485852511286);
}
void car_F_fun(double *state, double dt, double *out_8336486238807882069) {
  F_fun(state,  dt, out_8336486238807882069);
}
void car_h_25(double *state, double *unused, double *out_1918519873367899214) {
  h_25(state, unused, out_1918519873367899214);
}
void car_H_25(double *state, double *unused, double *out_8623128896152181528) {
  H_25(state, unused, out_8623128896152181528);
}
void car_h_24(double *state, double *unused, double *out_4068985978219487213) {
  h_24(state, unused, out_4068985978219487213);
}
void car_H_24(double *state, double *unused, double *out_604936289917013697) {
  H_24(state, unused, out_604936289917013697);
}
void car_h_30(double *state, double *unused, double *out_2075706541719028359) {
  h_30(state, unused, out_2075706541719028359);
}
void car_H_30(double *state, double *unused, double *out_5295918847429761890) {
  H_30(state, unused, out_5295918847429761890);
}
void car_h_26(double *state, double *unused, double *out_47122023652287635) {
  h_26(state, unused, out_47122023652287635);
}
void car_H_26(double *state, double *unused, double *out_6082111858683313864) {
  H_26(state, unused, out_6082111858683313864);
}
void car_h_27(double *state, double *unused, double *out_6880899871049192446) {
  h_27(state, unused, out_6880899871049192446);
}
void car_H_27(double *state, double *unused, double *out_7519512918613705107) {
  H_27(state, unused, out_7519512918613705107);
}
void car_h_29(double *state, double *unused, double *out_3933173880693707485) {
  h_29(state, unused, out_3933173880693707485);
}
void car_H_29(double *state, double *unused, double *out_5806150191744154074) {
  H_29(state, unused, out_5806150191744154074);
}
void car_h_28(double *state, double *unused, double *out_1147213510920962491) {
  h_28(state, unused, out_1147213510920962491);
}
void car_H_28(double *state, double *unused, double *out_723751174674623500) {
  H_28(state, unused, out_723751174674623500);
}
void car_h_31(double *state, double *unused, double *out_713380529375319027) {
  h_31(state, unused, out_713380529375319027);
}
void car_H_31(double *state, double *unused, double *out_8592482934275221100) {
  H_31(state, unused, out_8592482934275221100);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
