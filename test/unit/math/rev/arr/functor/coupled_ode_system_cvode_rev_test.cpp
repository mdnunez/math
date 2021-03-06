#include <stan/math/rev/arr.hpp>
#include <gtest/gtest.h>
#include <test/unit/util.hpp>
#include <test/unit/math/prim/arr/functor/harmonic_oscillator.hpp>
#include <test/unit/math/prim/arr/functor/mock_ode_functor.hpp>
#include <test/unit/math/prim/arr/functor/mock_throwing_ode_functor.hpp>

struct StanMathRevOdeCVode : public ::testing::Test {
  void SetUp() {
    stan::math::recover_memory();
  }
  std::stringstream msgs;
  std::vector<double> x;
  std::vector<int> x_int;
  double t0;
};

// ******************** DV ****************************
TEST_F(StanMathRevOdeCVode, coupled_ode_system_dv) {
  using stan::math::coupled_ode_system_cvode;

  harm_osc_ode_fun harm_osc;

  std::vector<stan::math::var> theta;
  std::vector<double> coupled_y0;
  std::vector<double> y0;
  std::vector<double> dy_dt;

  double gamma(0.15);
  t0 = 0;

  theta.push_back(gamma);
  y0.push_back(1.0);
  y0.push_back(0.5);

  coupled_y0.push_back(1.0);
  coupled_y0.push_back(0.5);
  coupled_y0.push_back(1.0);
  coupled_y0.push_back(2.0);

  coupled_ode_system_cvode<harm_osc_ode_fun, double, stan::math::var>
    system(harm_osc, y0, t0, theta, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  system(coupled_y0, dy_dt, t0);

  EXPECT_FLOAT_EQ(0.5, dy_dt[0]);
  EXPECT_FLOAT_EQ(-1.075, dy_dt[1]);
  EXPECT_FLOAT_EQ(2, dy_dt[2]);
  EXPECT_FLOAT_EQ(-1.8, dy_dt[3]);
}
TEST_F(StanMathRevOdeCVode, decouple_states_dv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;

  mock_ode_functor mock_ode;

  size_t T = 10;

  std::vector<double> y0(2);
  y0[0] = 1.0;
  y0[1] = 0.5;

  std::vector<var> theta(1);
  theta[0] = 0.15;

  coupled_ode_system_cvode<mock_ode_functor, double, var>
    coupled_system(mock_ode, y0, t0, theta, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  size_t k = 0;
  std::vector<std::vector<double> > ys_coupled(T);
  for (size_t t = 0; t < T; t++) {
    std::vector<double> coupled_state(coupled_system.size(), 0.0);
    for (size_t n = 0; n < coupled_system.size(); n++)
      coupled_state[n] = ++k;
    ys_coupled[t] = coupled_state;
  }

  std::vector<std::vector<var> > ys;
  ys = coupled_system.decouple_states(ys_coupled);

  ASSERT_EQ(T, ys.size());
  for (size_t t = 0; t < T; t++)
    ASSERT_EQ(2U, ys[t].size());

  for (size_t t = 0; t < T; t++)
    for (size_t n = 0; n < 2; n++)
      EXPECT_FLOAT_EQ(ys_coupled[t][n], ys[t][n].val());
}
TEST_F(StanMathRevOdeCVode, initial_state_dv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<double> y0_d(N, 0.0);
  std::vector<var> theta_v(M, 0.0);

  for (size_t n = 0; n < N; n++)
    y0_d[n] = n+1;
  for (size_t m = 0; m < M; m++)
    theta_v[m] = 10 * (m+1);

  coupled_ode_system_cvode<mock_ode_functor, double, var>
    coupled_system_dv(base_ode, y0_d, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double> state = coupled_system_dv.initial_state();
  for (size_t n = 0; n < N; n++)
    EXPECT_FLOAT_EQ(y0_d[n], state[n])
      << "we don't need derivatives of y0; initial state gets the initial values";
  for (size_t n = N; n < state.size(); n++)
    EXPECT_FLOAT_EQ(0.0, state[n]);
}
TEST_F(StanMathRevOdeCVode, size_dv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<double> y0_d(N, 0.0);
  std::vector<var> theta_v(M, 0.0);

  coupled_ode_system_cvode<mock_ode_functor, double, var>
    coupled_system_dv(base_ode, y0_d, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  EXPECT_EQ(N + N * M, coupled_system_dv.size());
}

TEST_F(StanMathRevOdeCVode, memory_recovery_dv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<double> y0_d(N, 0.0);
  std::vector<var> theta_v(M, 0.0);

  coupled_ode_system_cvode<mock_ode_functor, double, var>
    coupled_system_dv(base_ode, y0_d, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double> y(3,0);
  std::vector<double> dy_dt(3,0);
  double t = 10;

  EXPECT_TRUE(stan::math::empty_nested());
  EXPECT_NO_THROW(coupled_system_dv(y, dy_dt, t));
  EXPECT_TRUE(stan::math::empty_nested());
}

TEST_F(StanMathRevOdeCVode, memory_recovery_exception_dv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  std::string message = "ode throws";

  const size_t N = 3;
  const size_t M = 4;
  for (size_t n = 0; n < N+1; n++) {
    std::stringstream scoped_message;
    scoped_message << "iteration " << n;
    SCOPED_TRACE(scoped_message.str());
    mock_throwing_ode_functor<std::logic_error> throwing_ode(message, 1);

    std::vector<double> y0_d(N, 0.0);
    std::vector<var> theta_v(M, 0.0);

    coupled_ode_system_cvode<mock_throwing_ode_functor<std::logic_error>,
                             double, var>
      coupled_system_dv(throwing_ode, y0_d, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

    std::vector<double> y(3,0);
    std::vector<double> dy_dt(3,0);
    double t = 10;

    EXPECT_TRUE(stan::math::empty_nested());
    EXPECT_THROW_MSG(coupled_system_dv(y, dy_dt, t),
                     std::logic_error,
                     message);
    EXPECT_TRUE(stan::math::empty_nested());
  }
}

// ******************** VD ****************************

TEST_F(StanMathRevOdeCVode, coupled_ode_system_vd) {
  using stan::math::coupled_ode_system_cvode;

  harm_osc_ode_fun harm_osc;

  std::vector<double> theta;
  std::vector<double> coupled_y0;
  std::vector<stan::math::var> y0_var;
  std::vector<double> y0_adj;
  double t0;
  std::vector<double> dy_dt;

  double gamma(0.15);
  t0 = 0;

  theta.push_back(gamma);

  coupled_y0.push_back(1.0);
  coupled_y0.push_back(0.5);
  coupled_y0.push_back(1.0);
  coupled_y0.push_back(3.0);
  coupled_y0.push_back(2.0);
  coupled_y0.push_back(5.0);

  y0_var.push_back(1.0);
  y0_var.push_back(0.5);

  coupled_ode_system_cvode<harm_osc_ode_fun, stan::math::var, double>
    system(harm_osc, y0_var, t0, theta, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  system(coupled_y0, dy_dt, t0);

  EXPECT_FLOAT_EQ(1.0, dy_dt[0]);
  EXPECT_FLOAT_EQ(-2.0 - 0.15 * 1.0, dy_dt[1]);
  EXPECT_FLOAT_EQ(0 + 1.0 * 0 + 3.0 * 1 + 0, dy_dt[2]);
  EXPECT_FLOAT_EQ(-1.0 - 1.0 * 1.0 - 0.15 * 3.0, dy_dt[3]);
  EXPECT_FLOAT_EQ(1.0 + 2.0 * 0 + 5.0 * 1.0, dy_dt[4]);
  EXPECT_FLOAT_EQ(-0.15 - 1.0 * 2.0 - 0.15 * 5.0, dy_dt[5]);
}
TEST_F(StanMathRevOdeCVode, decouple_states_vd) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;

  mock_ode_functor mock_ode;
  size_t T = 10;

  std::vector<var> y0(2);
  std::vector<double> theta(1);

  y0[0] = 1.0;
  y0[1] = 0.5;
  theta[0] = 0.15;

  coupled_ode_system_cvode<mock_ode_functor, var, double>
    coupled_system(mock_ode, y0, t0, theta, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  size_t k = 0;
  std::vector<std::vector<double> > ys_coupled(T);
  for (size_t t = 0; t < T; t++) {
    std::vector<double> coupled_state(coupled_system.size(), 0.0);
    for (size_t n = 0; n < coupled_system.size(); n++)
      coupled_state[n] = ++k;
    ys_coupled[t] = coupled_state;
  }

  std::vector<std::vector<var> > ys;
  ys = coupled_system.decouple_states(ys_coupled);

  ASSERT_EQ(T, ys.size());
  for (size_t t = 0; t < T; t++)
    ASSERT_EQ(2U, ys[t].size());

  for (size_t t = 0; t < T; t++)
    for (size_t n = 0; n < 2; n++)
      EXPECT_FLOAT_EQ(ys_coupled[t][n] + y0[n].val(),
                      ys[t][n].val());
}
TEST_F(StanMathRevOdeCVode, initial_state_vd) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<var> y0_v(N, 0.0);
  std::vector<double> theta_d(M, 0.0);

  for (size_t n = 0; n < N; n++)
    y0_v[n] = n + 1;
  for (size_t m = 0; m < M; m++)
    theta_d[m] = 10 * (m + 1);

  coupled_ode_system_cvode<mock_ode_functor, var, double>
    coupled_system_vd(base_ode, y0_v, t0, theta_d, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double> state;

  state = coupled_system_vd.initial_state();
  for (size_t n = 0; n < N; n++)
    EXPECT_FLOAT_EQ(0.0, state[n])
      << "we need derivatives of y0; initial state gets set to 0";
  for (size_t n = N; n < state.size(); n++)
    EXPECT_FLOAT_EQ(0.0, state[n]);
}
TEST_F(StanMathRevOdeCVode, size_vd) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<var> y0_v(N, 0.0);
  std::vector<double> theta_d(M, 0.0);

  coupled_ode_system_cvode<mock_ode_functor, var, double>
    coupled_system_vd(base_ode, y0_v, t0, theta_d, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  EXPECT_EQ(N + N * N, coupled_system_vd.size());
}

TEST_F(StanMathRevOdeCVode, memory_recovery_vd) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<var> y0_v(N, 0.0);
  std::vector<double> theta_d(M, 0.0);

  coupled_ode_system_cvode<mock_ode_functor, var, double>
    coupled_system_vd(base_ode, y0_v, t0, theta_d, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double> y(3,0);
  std::vector<double> dy_dt(3,0);
  double t = 10;

  EXPECT_TRUE(stan::math::empty_nested());
  EXPECT_NO_THROW(coupled_system_vd(y, dy_dt, t));
  EXPECT_TRUE(stan::math::empty_nested());
}

TEST_F(StanMathRevOdeCVode, memory_recovery_exception_vd) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  std::string message = "ode throws";

  const size_t N = 3;
  const size_t M = 4;
  for (size_t n = 0; n < N+1; n++) {
    std::stringstream scoped_message;
    scoped_message << "iteration " << n;
    SCOPED_TRACE(scoped_message.str());
    mock_throwing_ode_functor<std::logic_error> throwing_ode(message, 1);

    std::vector<var> y0_v(N, 0.0);
    std::vector<double> theta_d(M, 0.0);

    coupled_ode_system_cvode<mock_throwing_ode_functor<std::logic_error>,
                             var, double>
      coupled_system_vd(throwing_ode, y0_v, t0, theta_d, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

    std::vector<double> y(3,0);
    std::vector<double> dy_dt(3,0);
    double t = 10;

    EXPECT_TRUE(stan::math::empty_nested());
    EXPECT_THROW_MSG(coupled_system_vd(y, dy_dt, t),
                     std::logic_error,
                     message);
    EXPECT_TRUE(stan::math::empty_nested());
  }
}


// ******************** VV ****************************

TEST_F(StanMathRevOdeCVode, coupled_ode_system_vv) {
  using stan::math::coupled_ode_system_cvode;

  std::vector<stan::math::var> y0_var;
  y0_var.push_back(1.0);
  y0_var.push_back(0.5);

  std::vector<stan::math::var> theta_var;
  theta_var.push_back(0.15);

  harm_osc_ode_fun harm_osc;
  coupled_ode_system_cvode<harm_osc_ode_fun, stan::math::var, stan::math::var>
    system(harm_osc, y0_var, t0, theta_var, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double> coupled_y0(8, 0);

  t0 = 0;

  std::vector<double> dy_dt;
  system(coupled_y0, dy_dt, t0);

  std::vector<double> y0_double(2);
  y0_double[0] = 1.0;
  y0_double[1] = 0.5;

  std::vector<double> theta_double(1);
  theta_double[0] = 0.15;

  std::vector<double>
    dy_dt_base = harm_osc(0.0, y0_double, theta_double, x, x_int, &msgs);

  EXPECT_FLOAT_EQ(dy_dt_base[0], dy_dt[0]);
  EXPECT_FLOAT_EQ(dy_dt_base[1], dy_dt[1]);
  EXPECT_FLOAT_EQ(0, dy_dt[2]);
  EXPECT_FLOAT_EQ(-1, dy_dt[3]);
  EXPECT_FLOAT_EQ(1, dy_dt[4]);
  EXPECT_FLOAT_EQ(-0.15, dy_dt[5]);
  EXPECT_FLOAT_EQ(0, dy_dt[6]);
  EXPECT_FLOAT_EQ(-0.5, dy_dt[7]);
}
TEST_F(StanMathRevOdeCVode, decouple_states_vv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;

  harm_osc_ode_fun harm_osc;

  std::vector<var> y0(2);
  std::vector<var> theta(1);

  y0[0] = 1.0;
  y0[1] = 0.5;
  theta[0] = 0.15;

  coupled_ode_system_cvode<harm_osc_ode_fun, var, var>
    coupled_system(harm_osc, y0, t0, theta, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  size_t T = 10;
  size_t k = 0;
  std::vector<std::vector<double> > ys_coupled(T);
  for (size_t t = 0; t < T; t++) {
    std::vector<double> coupled_state(coupled_system.size(), 0.0);
    for (size_t n = 0; n < coupled_system.size(); n++)
      coupled_state[n] = ++k;
    ys_coupled[t] = coupled_state;
  }

  std::vector<std::vector<var> > ys;
  ys = coupled_system.decouple_states(ys_coupled);

  ASSERT_EQ(T, ys.size());
  for (size_t t = 0; t < T; t++)
    ASSERT_EQ(2U, ys[t].size());

  for (size_t t = 0; t < T; t++)
    for (size_t n = 0; n < 2; n++)
      EXPECT_FLOAT_EQ(ys_coupled[t][n] + y0[n].val(),
                      ys[t][n].val());
}
TEST_F(StanMathRevOdeCVode, initial_state_vv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<var> y0_v(N, 0.0);
  std::vector<var> theta_v(M, 0.0);

  for (size_t n = 0; n < N; n++)
    y0_v[n] = n+1;
  for (size_t m = 0; m < M; m++)
    theta_v[m] = 10 * (m+1);

  coupled_ode_system_cvode<mock_ode_functor, var, var>
    coupled_system_vv(base_ode, y0_v, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double>  state = coupled_system_vv.initial_state();
  for (size_t n = 0; n < N; n++)
    EXPECT_FLOAT_EQ(0.0, state[n])
      << "we need derivatives of y0; initial state gets set to 0";
  for (size_t n = N; n < state.size(); n++)
    EXPECT_FLOAT_EQ(0.0, state[n]);
}
TEST_F(StanMathRevOdeCVode, size_vv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<var> y0_v(N, 0.0);
  std::vector<var> theta_v(M, 0.0);

  coupled_ode_system_cvode<mock_ode_functor, var, var>
    coupled_system_vv(base_ode, y0_v, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  EXPECT_EQ(N + N * N + N * M, coupled_system_vv.size());
}

TEST_F(StanMathRevOdeCVode, memory_recovery_vv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  mock_ode_functor base_ode;

  const size_t N = 3;
  const size_t M = 4;

  std::vector<var> y0_v(N, 0.0);
  std::vector<var> theta_v(M, 0.0);

  coupled_ode_system_cvode<mock_ode_functor, var, var>
    coupled_system_vv(base_ode, y0_v, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

  std::vector<double> y(3,0);
  std::vector<double> dy_dt(3,0);
  double t = 10;

  EXPECT_TRUE(stan::math::empty_nested());
  EXPECT_NO_THROW(coupled_system_vv(y, dy_dt, t));
  EXPECT_TRUE(stan::math::empty_nested());
}

TEST_F(StanMathRevOdeCVode, memory_recovery_exception_vv) {
  using stan::math::coupled_ode_system_cvode;
  using stan::math::var;
  std::string message = "ode throws";

  const size_t N = 3;
  const size_t M = 4;
  for (size_t n = 0; n < N + 1; n++) {
    std::stringstream scoped_message;
    scoped_message << "iteration " << n;
    SCOPED_TRACE(scoped_message.str());
    mock_throwing_ode_functor<std::logic_error> throwing_ode(message, 1);

    std::vector<var> y0_v(N, 0.0);
    std::vector<var> theta_v(M, 0.0);

    coupled_ode_system_cvode<mock_throwing_ode_functor<std::logic_error>, var, var>
      coupled_system_vv(throwing_ode, y0_v, t0, theta_v, x, x_int, 1e-8, 1e-10, 1e6, &msgs);

    std::vector<double> y(3,0);
    std::vector<double> dy_dt(3,0);
    double t = 10;

    EXPECT_TRUE(stan::math::empty_nested());
    EXPECT_THROW_MSG(coupled_system_vv(y, dy_dt, t),
                     std::logic_error,
                     message);
    EXPECT_TRUE(stan::math::empty_nested());
  }
}
