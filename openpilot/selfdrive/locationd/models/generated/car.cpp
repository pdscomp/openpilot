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
void err_fun(double *nom_x, double *delta_x, double *out_3157126624170581198) {
   out_3157126624170581198[0] = delta_x[0] + nom_x[0];
   out_3157126624170581198[1] = delta_x[1] + nom_x[1];
   out_3157126624170581198[2] = delta_x[2] + nom_x[2];
   out_3157126624170581198[3] = delta_x[3] + nom_x[3];
   out_3157126624170581198[4] = delta_x[4] + nom_x[4];
   out_3157126624170581198[5] = delta_x[5] + nom_x[5];
   out_3157126624170581198[6] = delta_x[6] + nom_x[6];
   out_3157126624170581198[7] = delta_x[7] + nom_x[7];
   out_3157126624170581198[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_4673666440688567315) {
   out_4673666440688567315[0] = -nom_x[0] + true_x[0];
   out_4673666440688567315[1] = -nom_x[1] + true_x[1];
   out_4673666440688567315[2] = -nom_x[2] + true_x[2];
   out_4673666440688567315[3] = -nom_x[3] + true_x[3];
   out_4673666440688567315[4] = -nom_x[4] + true_x[4];
   out_4673666440688567315[5] = -nom_x[5] + true_x[5];
   out_4673666440688567315[6] = -nom_x[6] + true_x[6];
   out_4673666440688567315[7] = -nom_x[7] + true_x[7];
   out_4673666440688567315[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_7076448425335990647) {
   out_7076448425335990647[0] = 1.0;
   out_7076448425335990647[1] = 0.0;
   out_7076448425335990647[2] = 0.0;
   out_7076448425335990647[3] = 0.0;
   out_7076448425335990647[4] = 0.0;
   out_7076448425335990647[5] = 0.0;
   out_7076448425335990647[6] = 0.0;
   out_7076448425335990647[7] = 0.0;
   out_7076448425335990647[8] = 0.0;
   out_7076448425335990647[9] = 0.0;
   out_7076448425335990647[10] = 1.0;
   out_7076448425335990647[11] = 0.0;
   out_7076448425335990647[12] = 0.0;
   out_7076448425335990647[13] = 0.0;
   out_7076448425335990647[14] = 0.0;
   out_7076448425335990647[15] = 0.0;
   out_7076448425335990647[16] = 0.0;
   out_7076448425335990647[17] = 0.0;
   out_7076448425335990647[18] = 0.0;
   out_7076448425335990647[19] = 0.0;
   out_7076448425335990647[20] = 1.0;
   out_7076448425335990647[21] = 0.0;
   out_7076448425335990647[22] = 0.0;
   out_7076448425335990647[23] = 0.0;
   out_7076448425335990647[24] = 0.0;
   out_7076448425335990647[25] = 0.0;
   out_7076448425335990647[26] = 0.0;
   out_7076448425335990647[27] = 0.0;
   out_7076448425335990647[28] = 0.0;
   out_7076448425335990647[29] = 0.0;
   out_7076448425335990647[30] = 1.0;
   out_7076448425335990647[31] = 0.0;
   out_7076448425335990647[32] = 0.0;
   out_7076448425335990647[33] = 0.0;
   out_7076448425335990647[34] = 0.0;
   out_7076448425335990647[35] = 0.0;
   out_7076448425335990647[36] = 0.0;
   out_7076448425335990647[37] = 0.0;
   out_7076448425335990647[38] = 0.0;
   out_7076448425335990647[39] = 0.0;
   out_7076448425335990647[40] = 1.0;
   out_7076448425335990647[41] = 0.0;
   out_7076448425335990647[42] = 0.0;
   out_7076448425335990647[43] = 0.0;
   out_7076448425335990647[44] = 0.0;
   out_7076448425335990647[45] = 0.0;
   out_7076448425335990647[46] = 0.0;
   out_7076448425335990647[47] = 0.0;
   out_7076448425335990647[48] = 0.0;
   out_7076448425335990647[49] = 0.0;
   out_7076448425335990647[50] = 1.0;
   out_7076448425335990647[51] = 0.0;
   out_7076448425335990647[52] = 0.0;
   out_7076448425335990647[53] = 0.0;
   out_7076448425335990647[54] = 0.0;
   out_7076448425335990647[55] = 0.0;
   out_7076448425335990647[56] = 0.0;
   out_7076448425335990647[57] = 0.0;
   out_7076448425335990647[58] = 0.0;
   out_7076448425335990647[59] = 0.0;
   out_7076448425335990647[60] = 1.0;
   out_7076448425335990647[61] = 0.0;
   out_7076448425335990647[62] = 0.0;
   out_7076448425335990647[63] = 0.0;
   out_7076448425335990647[64] = 0.0;
   out_7076448425335990647[65] = 0.0;
   out_7076448425335990647[66] = 0.0;
   out_7076448425335990647[67] = 0.0;
   out_7076448425335990647[68] = 0.0;
   out_7076448425335990647[69] = 0.0;
   out_7076448425335990647[70] = 1.0;
   out_7076448425335990647[71] = 0.0;
   out_7076448425335990647[72] = 0.0;
   out_7076448425335990647[73] = 0.0;
   out_7076448425335990647[74] = 0.0;
   out_7076448425335990647[75] = 0.0;
   out_7076448425335990647[76] = 0.0;
   out_7076448425335990647[77] = 0.0;
   out_7076448425335990647[78] = 0.0;
   out_7076448425335990647[79] = 0.0;
   out_7076448425335990647[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_9009949425942935353) {
   out_9009949425942935353[0] = state[0];
   out_9009949425942935353[1] = state[1];
   out_9009949425942935353[2] = state[2];
   out_9009949425942935353[3] = state[3];
   out_9009949425942935353[4] = state[4];
   out_9009949425942935353[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8100000000000005*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_9009949425942935353[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_9009949425942935353[7] = state[7];
   out_9009949425942935353[8] = state[8];
}
void F_fun(double *state, double dt, double *out_6772166287624075049) {
   out_6772166287624075049[0] = 1;
   out_6772166287624075049[1] = 0;
   out_6772166287624075049[2] = 0;
   out_6772166287624075049[3] = 0;
   out_6772166287624075049[4] = 0;
   out_6772166287624075049[5] = 0;
   out_6772166287624075049[6] = 0;
   out_6772166287624075049[7] = 0;
   out_6772166287624075049[8] = 0;
   out_6772166287624075049[9] = 0;
   out_6772166287624075049[10] = 1;
   out_6772166287624075049[11] = 0;
   out_6772166287624075049[12] = 0;
   out_6772166287624075049[13] = 0;
   out_6772166287624075049[14] = 0;
   out_6772166287624075049[15] = 0;
   out_6772166287624075049[16] = 0;
   out_6772166287624075049[17] = 0;
   out_6772166287624075049[18] = 0;
   out_6772166287624075049[19] = 0;
   out_6772166287624075049[20] = 1;
   out_6772166287624075049[21] = 0;
   out_6772166287624075049[22] = 0;
   out_6772166287624075049[23] = 0;
   out_6772166287624075049[24] = 0;
   out_6772166287624075049[25] = 0;
   out_6772166287624075049[26] = 0;
   out_6772166287624075049[27] = 0;
   out_6772166287624075049[28] = 0;
   out_6772166287624075049[29] = 0;
   out_6772166287624075049[30] = 1;
   out_6772166287624075049[31] = 0;
   out_6772166287624075049[32] = 0;
   out_6772166287624075049[33] = 0;
   out_6772166287624075049[34] = 0;
   out_6772166287624075049[35] = 0;
   out_6772166287624075049[36] = 0;
   out_6772166287624075049[37] = 0;
   out_6772166287624075049[38] = 0;
   out_6772166287624075049[39] = 0;
   out_6772166287624075049[40] = 1;
   out_6772166287624075049[41] = 0;
   out_6772166287624075049[42] = 0;
   out_6772166287624075049[43] = 0;
   out_6772166287624075049[44] = 0;
   out_6772166287624075049[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_6772166287624075049[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_6772166287624075049[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_6772166287624075049[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_6772166287624075049[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_6772166287624075049[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_6772166287624075049[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_6772166287624075049[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_6772166287624075049[53] = -9.8100000000000005*dt;
   out_6772166287624075049[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_6772166287624075049[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_6772166287624075049[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_6772166287624075049[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_6772166287624075049[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_6772166287624075049[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_6772166287624075049[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_6772166287624075049[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_6772166287624075049[62] = 0;
   out_6772166287624075049[63] = 0;
   out_6772166287624075049[64] = 0;
   out_6772166287624075049[65] = 0;
   out_6772166287624075049[66] = 0;
   out_6772166287624075049[67] = 0;
   out_6772166287624075049[68] = 0;
   out_6772166287624075049[69] = 0;
   out_6772166287624075049[70] = 1;
   out_6772166287624075049[71] = 0;
   out_6772166287624075049[72] = 0;
   out_6772166287624075049[73] = 0;
   out_6772166287624075049[74] = 0;
   out_6772166287624075049[75] = 0;
   out_6772166287624075049[76] = 0;
   out_6772166287624075049[77] = 0;
   out_6772166287624075049[78] = 0;
   out_6772166287624075049[79] = 0;
   out_6772166287624075049[80] = 1;
}
void h_25(double *state, double *unused, double *out_3147464506924368092) {
   out_3147464506924368092[0] = state[6];
}
void H_25(double *state, double *unused, double *out_736028625381073427) {
   out_736028625381073427[0] = 0;
   out_736028625381073427[1] = 0;
   out_736028625381073427[2] = 0;
   out_736028625381073427[3] = 0;
   out_736028625381073427[4] = 0;
   out_736028625381073427[5] = 0;
   out_736028625381073427[6] = 1;
   out_736028625381073427[7] = 0;
   out_736028625381073427[8] = 0;
}
void h_24(double *state, double *unused, double *out_2685318194937829437) {
   out_2685318194937829437[0] = state[4];
   out_2685318194937829437[1] = state[5];
}
void H_24(double *state, double *unused, double *out_4537465657531939167) {
   out_4537465657531939167[0] = 0;
   out_4537465657531939167[1] = 0;
   out_4537465657531939167[2] = 0;
   out_4537465657531939167[3] = 0;
   out_4537465657531939167[4] = 1;
   out_4537465657531939167[5] = 0;
   out_4537465657531939167[6] = 0;
   out_4537465657531939167[7] = 0;
   out_4537465657531939167[8] = 0;
   out_4537465657531939167[9] = 0;
   out_4537465657531939167[10] = 0;
   out_4537465657531939167[11] = 0;
   out_4537465657531939167[12] = 0;
   out_4537465657531939167[13] = 0;
   out_4537465657531939167[14] = 1;
   out_4537465657531939167[15] = 0;
   out_4537465657531939167[16] = 0;
   out_4537465657531939167[17] = 0;
}
void h_30(double *state, double *unused, double *out_7536014309964511083) {
   out_7536014309964511083[0] = state[4];
}
void H_30(double *state, double *unused, double *out_1782304333126175200) {
   out_1782304333126175200[0] = 0;
   out_1782304333126175200[1] = 0;
   out_1782304333126175200[2] = 0;
   out_1782304333126175200[3] = 0;
   out_1782304333126175200[4] = 1;
   out_1782304333126175200[5] = 0;
   out_1782304333126175200[6] = 0;
   out_1782304333126175200[7] = 0;
   out_1782304333126175200[8] = 0;
}
void h_26(double *state, double *unused, double *out_5152744849751302577) {
   out_5152744849751302577[0] = state[7];
}
void H_26(double *state, double *unused, double *out_4477531944255129651) {
   out_4477531944255129651[0] = 0;
   out_4477531944255129651[1] = 0;
   out_4477531944255129651[2] = 0;
   out_4477531944255129651[3] = 0;
   out_4477531944255129651[4] = 0;
   out_4477531944255129651[5] = 0;
   out_4477531944255129651[6] = 0;
   out_4477531944255129651[7] = 1;
   out_4477531944255129651[8] = 0;
}
void h_27(double *state, double *unused, double *out_1775736216339172739) {
   out_1775736216339172739[0] = state[3];
}
void H_27(double *state, double *unused, double *out_392458978674249711) {
   out_392458978674249711[0] = 0;
   out_392458978674249711[1] = 0;
   out_392458978674249711[2] = 0;
   out_392458978674249711[3] = 1;
   out_392458978674249711[4] = 0;
   out_392458978674249711[5] = 0;
   out_392458978674249711[6] = 0;
   out_392458978674249711[7] = 0;
   out_392458978674249711[8] = 0;
}
void h_29(double *state, double *unused, double *out_5909056496084194888) {
   out_5909056496084194888[0] = state[1];
}
void H_29(double *state, double *unused, double *out_2292535677440567384) {
   out_2292535677440567384[0] = 0;
   out_2292535677440567384[1] = 1;
   out_2292535677440567384[2] = 0;
   out_2292535677440567384[3] = 0;
   out_2292535677440567384[4] = 0;
   out_2292535677440567384[5] = 0;
   out_2292535677440567384[6] = 0;
   out_2292535677440567384[7] = 0;
   out_2292535677440567384[8] = 0;
}
void h_28(double *state, double *unused, double *out_6777409484000018384) {
   out_6777409484000018384[0] = state[0];
}
void H_28(double *state, double *unused, double *out_2789863339628963190) {
   out_2789863339628963190[0] = 1;
   out_2789863339628963190[1] = 0;
   out_2789863339628963190[2] = 0;
   out_2789863339628963190[3] = 0;
   out_2789863339628963190[4] = 0;
   out_2789863339628963190[5] = 0;
   out_2789863339628963190[6] = 0;
   out_2789863339628963190[7] = 0;
   out_2789863339628963190[8] = 0;
}
void h_31(double *state, double *unused, double *out_8711451640308888361) {
   out_8711451640308888361[0] = state[8];
}
void H_31(double *state, double *unused, double *out_5103740046488481127) {
   out_5103740046488481127[0] = 0;
   out_5103740046488481127[1] = 0;
   out_5103740046488481127[2] = 0;
   out_5103740046488481127[3] = 0;
   out_5103740046488481127[4] = 0;
   out_5103740046488481127[5] = 0;
   out_5103740046488481127[6] = 0;
   out_5103740046488481127[7] = 0;
   out_5103740046488481127[8] = 1;
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
void car_err_fun(double *nom_x, double *delta_x, double *out_3157126624170581198) {
  err_fun(nom_x, delta_x, out_3157126624170581198);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_4673666440688567315) {
  inv_err_fun(nom_x, true_x, out_4673666440688567315);
}
void car_H_mod_fun(double *state, double *out_7076448425335990647) {
  H_mod_fun(state, out_7076448425335990647);
}
void car_f_fun(double *state, double dt, double *out_9009949425942935353) {
  f_fun(state,  dt, out_9009949425942935353);
}
void car_F_fun(double *state, double dt, double *out_6772166287624075049) {
  F_fun(state,  dt, out_6772166287624075049);
}
void car_h_25(double *state, double *unused, double *out_3147464506924368092) {
  h_25(state, unused, out_3147464506924368092);
}
void car_H_25(double *state, double *unused, double *out_736028625381073427) {
  H_25(state, unused, out_736028625381073427);
}
void car_h_24(double *state, double *unused, double *out_2685318194937829437) {
  h_24(state, unused, out_2685318194937829437);
}
void car_H_24(double *state, double *unused, double *out_4537465657531939167) {
  H_24(state, unused, out_4537465657531939167);
}
void car_h_30(double *state, double *unused, double *out_7536014309964511083) {
  h_30(state, unused, out_7536014309964511083);
}
void car_H_30(double *state, double *unused, double *out_1782304333126175200) {
  H_30(state, unused, out_1782304333126175200);
}
void car_h_26(double *state, double *unused, double *out_5152744849751302577) {
  h_26(state, unused, out_5152744849751302577);
}
void car_H_26(double *state, double *unused, double *out_4477531944255129651) {
  H_26(state, unused, out_4477531944255129651);
}
void car_h_27(double *state, double *unused, double *out_1775736216339172739) {
  h_27(state, unused, out_1775736216339172739);
}
void car_H_27(double *state, double *unused, double *out_392458978674249711) {
  H_27(state, unused, out_392458978674249711);
}
void car_h_29(double *state, double *unused, double *out_5909056496084194888) {
  h_29(state, unused, out_5909056496084194888);
}
void car_H_29(double *state, double *unused, double *out_2292535677440567384) {
  H_29(state, unused, out_2292535677440567384);
}
void car_h_28(double *state, double *unused, double *out_6777409484000018384) {
  h_28(state, unused, out_6777409484000018384);
}
void car_H_28(double *state, double *unused, double *out_2789863339628963190) {
  H_28(state, unused, out_2789863339628963190);
}
void car_h_31(double *state, double *unused, double *out_8711451640308888361) {
  h_31(state, unused, out_8711451640308888361);
}
void car_H_31(double *state, double *unused, double *out_5103740046488481127) {
  H_31(state, unused, out_5103740046488481127);
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
