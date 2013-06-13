/** 
 * @file
 * @copyright University of Warsaw
 * @section LICENSE
 * GPLv3+ (see the COPYING file or http://www.gnu.org/licenses/)
 *
 * \include "mpdata_1d_opt_sss/test_mpdata_1d_opt_eps.cpp"
 * \image html "../../tests/mpdata_1d_opt_sss/figure_iters=3.svg" TODO
 */

#include <libmpdata++/solvers/adv/mpdata_1d.hpp>
#include <libmpdata++/bcond/bcond.hpp>
#include <libmpdata++/concurr/threads.hpp>
#include <libmpdata++/output/gnuplot.hpp>

// TODO: make a common file with the setopts and setup from below?

using namespace libmpdataxx;

using real_t = float;
int n = 500, nt = 1600;

template <class T>
void setup(T &solver, int n) 
{
  blitz::firstIndex i;
  int width = 50, center = 100;
  solver.state() = where(i <= center-width/2 || i >= center+width/2, -400, 400) * blitz::tiny(real_t(0)); 
  solver.courant() = .5; 
}

template <class T>
void setopts(T &p, const int nt, const std::string &fname)
{
  p.outfreq = nt; // displays initial condition and the final state
  p.gnuplot_output = fname + ".svg";    
  p.outvars = {{0, {.name = "psi", .unit = "1"}}};
  p.gnuplot_command = "plot";
  p.gnuplot_with = "histeps";
  //p.gnuplot_yrange = "[-2:5]";
}

template <class solver_t, class vec_t>
void add_solver(vec_t &slvs, const std::string &fname)
{
  using output_t = output::gnuplot<solver_t>;
  typename output_t::params_t p;
  setopts(p, nt, fname);
  slvs.push_back(new concurr::threads<output_t, bcond::cyclic>(n, p));
  setup(slvs.back(), n);
}

int main() 
{
  const int n_dims = 1;
  boost::ptr_vector<concurr::any<real_t, n_dims>> slvs;

  const int n_eqs = 1;
  add_solver<solvers::mpdata_1d<real_t, 2, n_eqs>>(slvs, "mpdata_iters=2");
  add_solver<solvers::mpdata_1d<real_t, 2, n_eqs, formulae::mpdata::eps>>(slvs, "mpdata_iters=2_eps");
  add_solver<solvers::mpdata_1d<real_t, 3, n_eqs>>(slvs, "mpdata_iters=3");
  add_solver<solvers::mpdata_1d<real_t, 3, n_eqs, formulae::mpdata::eps>>(slvs, "mpdata_iters=3_eps");

  for (auto &slv : slvs) slv.advance(nt);
}