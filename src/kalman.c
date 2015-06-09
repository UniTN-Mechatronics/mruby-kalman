/***************************************************************************/
/*                                                                         */
/* kalman.c - Kalman filtering for mruby                                   */
/* Copyright (C) 2015 Paolo Bosetti and Mirko Brentari,                    */
/* paolo[dot]bosetti[at]unitn.it                                           */
/* Department of Industrial Engineering, University of Trento              */
/*                                                                         */
/* This library is free software.  You can redistribute it and/or          */
/* modify it under the terms of the GNU GENERAL PUBLIC LICENSE 2.0.        */
/*                                                                         */
/* This library is distributed in the hope that it will be useful,         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/* Artistic License 2.0 for more details.                                  */
/*                                                                         */
/* See the file LICENSE                                                    */
/*                                                                         */
/***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/string.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/value.h"
#include "mruby/array.h"
#include "mruby/numeric.h"

#include "memory.h"

// Struct holding data:
typedef struct {
  double theta_est;
  double thetad_bias_est;
  double Q_theta;
  double Q_thetad_bias;
  double R;
  double y;
  double S;
  double P[2][2];
  double K[2];
} kalman_data_s;

// Garbage collector handler, for kalman_data struct
// if kalman_data contains other dynamic data, free it too!
// Check it with GC.start
static void kalman_data_destructor(mrb_state *mrb, void *p_) {
  kalman_data_s *pd = (kalman_data_s *)p_;
  free(pd);
  // or simply:
  // mrb_free(mrb, pd);
};

// Creating data type and reference for GC, in a const struct
const struct mrb_data_type kalman_data_type = {"kalman_data",
                                               kalman_data_destructor};

// Utility function for getting the struct out of the wrapping IV @data
static void mrb_kalman_get_data(mrb_state *mrb, mrb_value self,
                                kalman_data_s **data) {
  mrb_value data_value;
  data_value = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@data"));

  // Loading data from data_value into p_data:
  Data_Get_Struct(mrb, data_value, &kalman_data_type, *data);
  if (!*data)
    mrb_raise(mrb, E_RUNTIME_ERROR, "Could not access @data");
}

// Data Initializer C function (not exposed!)
static void mrb_kalman_init(mrb_state *mrb, mrb_value self, double theta,
                            double thetad) {
  mrb_value data_value;  // this IV holds the data
  kalman_data_s *p_data; // pointer to the C struct

  data_value = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@data"));

  // if @data already exists, free its content:
  if (!mrb_nil_p(data_value)) {
    Data_Get_Struct(mrb, data_value, &kalman_data_type, p_data);
    free(p_data);
  }
  // Allocate and zero-out the data struct:
  p_data = malloc(sizeof(kalman_data_s));
  memset(p_data, 0, sizeof(kalman_data_s));
  if (!p_data)
    mrb_raise(mrb, E_RUNTIME_ERROR, "Could not allocate @data");

  // Wrap struct into @data:
  mrb_iv_set(
      mrb, self, mrb_intern_lit(mrb, "@data"), // set @data
      mrb_obj_value(                           // with value hold in struct
          Data_Wrap_Struct(mrb, mrb->object_class, &kalman_data_type, p_data)));

  // Now set values into struct:
  p_data->theta_est = theta;
  p_data->thetad_bias_est = thetad;
  p_data->Q_theta = 0.0000001;
  p_data->Q_thetad_bias = 0.00001;
  p_data->R = 2.0;
  p_data->P[0][0] = 10;
  p_data->P[1][0] = 0;
  p_data->P[0][1] = 0;
  p_data->P[1][1] = 10;
}

static mrb_value mrb_kalman_initialize(mrb_state *mrb, mrb_value self) {
  mrb_float theta, thetad = 0.0;
  mrb_int narg = 0;
  narg = mrb_get_args(mrb, "|ff", &theta, &thetad);

  // Call strcut initializer:
  if (narg == 1)
    mrb_kalman_init(mrb, self, theta, 0);
  else if (narg == 0)
    mrb_kalman_init(mrb, self, 0, 0);
  else
    mrb_kalman_init(mrb, self, theta, thetad);
  return mrb_nil_value();
}

static mrb_value mrb_kalman_theta(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // Play with p_data content:
  return mrb_float_value(mrb, p_data->theta_est);
}

static mrb_value mrb_kalman_thetad(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // Play with p_data content:
  return mrb_float_value(mrb, p_data->thetad_bias_est);
}

static mrb_value mrb_kalman_Q_theta(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // Play with p_data content:
  return mrb_float_value(mrb, p_data->Q_theta);
}

static mrb_value mrb_kalman_set_Q_theta(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;
  mrb_float v;
  mrb_get_args(mrb, "f", &v);
  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);
  p_data->Q_theta = v;
  // Play with p_data content:
  return mrb_float_value(mrb, p_data->Q_theta);
}

static mrb_value mrb_kalman_Q_thetad(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // Play with p_data content:
  return mrb_float_value(mrb, p_data->Q_thetad_bias);
}

static mrb_value mrb_kalman_set_Q_thetad(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;
  mrb_float v;
  mrb_get_args(mrb, "f", &v);
  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);
  p_data->Q_thetad_bias = v;
  // Play with p_data content:
  return mrb_float_value(mrb, p_data->Q_thetad_bias);
}

static mrb_value mrb_kalman_R(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // Play with p_data content:
  return mrb_float_value(mrb, p_data->R);
}

static mrb_value mrb_kalman_set_R(mrb_state *mrb, mrb_value self) {
  kalman_data_s *p_data = NULL;
  mrb_float v;
  mrb_get_args(mrb, "f", &v);
  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);
  p_data->R = v;
  // Play with p_data content:
  return mrb_float_value(mrb, p_data->R);
}


static mrb_value mrb_kalman_P_ary(mrb_state *mrb, mrb_value self) {
  mrb_int i, j;
  mrb_value a;
  kalman_data_s *p_data = NULL;

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);
  a = mrb_ary_new(mrb);
  for (i = 0; i < 2; i++) {
    mrb_value line = mrb_ary_new(mrb);
    for (j = 0; j < 2; j++) {
      mrb_ary_push(mrb, line, mrb_float_value(mrb, p_data->P[i][j]));
    }
    mrb_ary_push(mrb, a, line);
  }
  // Play with p_data content:
  return a;
}

static mrb_value mrb_kalman_P(mrb_state *mrb, mrb_value self) {
  mrb_int i, j;
  kalman_data_s *p_data = NULL;
  mrb_get_args(mrb, "ii", &i, &j);

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // Play with p_data content:
  return mrb_float_value(mrb, p_data->P[i][j]);
}

static mrb_value mrb_kalman_set_P(mrb_state *mrb, mrb_value self) {
  mrb_int i, j = 0;
  mrb_float v = 0;
  kalman_data_s *p_data = NULL;
  mrb_get_args(mrb, "iif", &i, &j, &v);
  // call utility for unwrapping @data into p_data:
  printf("Setting %d,%d = %f\n", i, j, v);
  mrb_kalman_get_data(mrb, self, &p_data);
  p_data->P[i][j] = v;
  // Play with p_data content:
  return mrb_float_value(mrb, p_data->P[i][j]);
}

static mrb_value mrb_kalman_update(mrb_state *mrb, mrb_value self) {
  mrb_float dt, theta, thetad;
  kalman_data_s *p_data = NULL;

  mrb_get_args(mrb, "fff", &dt, &theta, &thetad);

  // call utility for unwrapping @data into p_data:
  mrb_kalman_get_data(mrb, self, &p_data);

  // estimate step
  p_data->theta_est += dt * (thetad - p_data->thetad_bias_est);
  p_data->P[0][0] += dt * (p_data->P[1][1] * dt - p_data->P[0][1] -
                           p_data->P[1][0] + p_data->Q_theta);
  p_data->P[0][1] -= p_data->P[1][1] * dt;
  p_data->P[1][0] -= p_data->P[1][1] * dt;
  p_data->P[1][1] += p_data->Q_thetad_bias * dt;
  // observation step
  p_data->y = theta - p_data->theta_est;
  p_data->S = p_data->P[0][0] + p_data->R;
  // update step
  p_data->K[0] = p_data->P[0][0] / p_data->S;
  p_data->K[1] = p_data->P[1][0] / p_data->S;

  p_data->theta_est += p_data->K[0] * p_data->y;
  p_data->thetad_bias_est += p_data->K[1] * p_data->y;

  p_data->P[0][0] -= p_data->K[0] * p_data->P[0][0];
  p_data->P[0][1] -= p_data->K[0] * p_data->P[0][1];
  p_data->P[1][0] -= p_data->K[1] * p_data->P[0][0];
  p_data->P[1][1] -= p_data->K[1] * p_data->P[0][1];

  return mrb_float_value(mrb, p_data->theta_est);
}



void mrb_mruby_kalman_gem_init(mrb_state *mrb) {
  struct RClass *kalman;
  kalman = mrb_define_class(mrb, "Kalman", mrb->object_class);
  mrb_define_method(mrb, kalman, "initialize", mrb_kalman_initialize,
                    MRB_ARGS_OPT(2));
  mrb_define_method(mrb, kalman, "theta", mrb_kalman_theta, MRB_ARGS_NONE());
  mrb_define_method(mrb, kalman, "thetad", mrb_kalman_thetad, MRB_ARGS_NONE());
  mrb_define_method(mrb, kalman, "Q_theta", mrb_kalman_Q_theta, MRB_ARGS_NONE());
  mrb_define_method(mrb, kalman, "Q_theta=", mrb_kalman_set_Q_theta, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, kalman, "Q_thetad", mrb_kalman_Q_thetad, MRB_ARGS_NONE());
  mrb_define_method(mrb, kalman, "Q_thetad=", mrb_kalman_set_Q_thetad, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, kalman, "R", mrb_kalman_R, MRB_ARGS_NONE());
  mrb_define_method(mrb, kalman, "R=", mrb_kalman_set_R, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, kalman, "P", mrb_kalman_P_ary, MRB_ARGS_NONE());
  mrb_define_method(mrb, kalman, "[]", mrb_kalman_P, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, kalman, "[]=", mrb_kalman_set_P, MRB_ARGS_REQ(3));
  mrb_define_method(mrb, kalman, "update", mrb_kalman_update, MRB_ARGS_REQ(2));

}

void mrb_mruby_kalman_gem_final(mrb_state *mrb) {}
