/** 
  * @file
  * @copyright University of Warsaw
  * @section LICENSE
  * GPLv3+ (see the COPYING file or http://www.gnu.org/licenses/)
  *
  * @brief conjugate residual pressure solver 
  *   (for more detailed discussion consult Smolarkiewicz & Margolin 1994 
  *  Appl. Math and Comp. Sci. 
  *  Variational solver for elliptic problems in atmospheric flows)
  *
  * @section DERIVATION
  * 
  * for introduction see the derivation of minimal residual pressure solver (solver_pressure_mr.hpp)
  * 
  * \f$ -\frac{1}{\bar{\rho}} \nabla \cdot (\bar{\rho} (\hat{u} - \frac{\triangle t}{2} \nabla \Phi)) = 0 \f$
  *
  * above equation can be written as \f$ \mathcal{L}(\Phi) - R = 0 \f$ 
  * 
  * where \f$ \mathcal{L}() \f$ in theory may be any linear semidefinite operator
  * (this scheme doesn't require the operator to be self-adjoint)
  *
  * (for a concise discussion of the needed assumtions for operator \f$ \mathcal{L}() \f$ consult Smolarkiewicz & Margolin 1994)
  *
  * in this case:
  *
  * \f$ \mathcal{L}() = \Delta() \f$
  *
  * \f$ R = - \frac{1}{\rho} \nabla \cdot {\rho} \hat{u} \f$ 
  *
  * to obtain faster convergence (than minimum residual scheme) we start from dampened wave equation (instead of diffusion equation)
  * 
  * \f$ \mathcal{L}(\Phi) - R = 0 \;\;\;\;\;\;   \Rightarrow \;\;\;\;\;\;\;\;
  * \mathcal{L}(\Phi) - R = \frac{\partial^2 \Phi}{\partial \tau^2} + \frac{1}{T}\frac{\partial \Phi}{\partial \tau} \f$ 
  *
  * using centered differencing for the second derivative and one-sided differencing or the first derivative 
  * leads to three term recurrence formula
  *
  * \f$ \Phi^{n+1} = \gamma \Phi^{n} + (1-\gamma)\Phi^{n-1} + \beta (\mathcal{L}(\Phi^{n}) - R)  \f$
  *
  * where
  * 
  * \f$ \gamma = \frac{2+\frac{\triangle \tau}{T}}{1+\frac{\triangle \tau}{T}}  \f$
  *
  * \f$ \beta = \frac{\triangle \tau^2}{1+\frac{\triangle \tau}{T}}  \f$
  * 
  * which after rearranging leads to
  *
  * \f$ \Phi^{n+1} = \Phi^{n} + \beta ^{n} (\alpha ^{n} p^{n-1} + r^{n}) \f$
  * 
  * where:
  *
  * \f$ \alpha ^{n} = \frac{(\gamma ^{n} -1) \beta ^{n-1}}{\beta ^{n}} \f$
  *
  * \f$ p^{n} = \frac{\Phi^{n+1}-\Phi ^{n}}{\beta ^{n} } \f$
  *
  * \f$ r^n = \mathcal{L}(\Phi ^{n}) -R \f$
  *
  * this leads to recurrence algorithm for \f$ \Phi \f$, residual error (\f$ r \f$) and directional error (\f$ p \f$)
  * 
  * \f$ \Phi ^{n+1} = \Phi ^{n} + \beta ^{n} p^{n} \f$
  *
  * \f$ r^{n+1} = r^{n} + \beta ^{n} \mathcal{L}(p^{n}) \f$
  *
  * \f$ p^{n+1} = r^{n+1} + \alpha ^{n+1} p^{n} \f$
  *
  * with the coefficients
  *
  * \f$ \beta ^{n}= - \frac{<r^{n} \mathcal{L}(p^n)>}{<\mathcal{L}(p^n) \mathcal{L}(p^n)>} \f$
  *
  * \f$ \alpha ^{n+1}= -  \frac{<\mathcal{L}(r^{n+1}) \mathcal{L}(p^n)>}{<\mathcal{L}(p^n) \mathcal{L}(p^n)> } \f$
  *
  * The recurrence of \f$ p \f$ implies a recuurence for \f$ \mathcal{L}(p) \f$
  *
  * \f$ \mathcal{L}(p^{n+1}) = \mathcal{L}(r^{n+1}) + \alpha^{n+1} \mathcal{L}(p^{n}) \f$
  * 
  * iterations in pseudo-time stop when residual error is smaller than a given value (for example .0001)

*/

#pragma once
#include "detail/solver_pressure_common.hpp"
#include "../formulae/nabla_formulae.hpp" //gradient, diveregnce

namespace advoocat
{
  namespace solvers
  {
    template <class inhomo_solver_t, int u, int w>
    class pressure_cr : public detail::pressure_solver_common<inhomo_solver_t, u, w>
    {
      public:

      using parent_t = detail::pressure_solver_common<inhomo_solver_t, u, w>;
      using real_t = typename parent_t::real_t;

      typename parent_t::arr_t p_err, lap_p_err;

      private:

      void pressure_solver_update()
      {
	using namespace arakawa_c;
	using formulae::nabla::grad;
	using formulae::nabla::div;

	real_t beta = .25;   //TODO
        real_t alpha = 1.;   //TODO
	real_t rho = 1.;     //TODO    
        real_t tmp_den = 1.; //TODO

	int halo = this->halo;
	rng_t &i = this->i;
	rng_t &j = this->j;

	this->tmp_u(i, j) = this->state(u)(i, j);
	this->tmp_w(i, j) = this->state(w)(i, j);

        this->xchng(this->Phi,   i^halo, j^halo);
        this->xchng(this->tmp_u, i^halo, j^halo);
	this->xchng(this->tmp_w, i^halo, j^halo);

        //initail error   
        this->err(i, j) =
          - 1./ rho * div(rho * this->tmp_u, rho * this->tmp_w , i, j, this->dx, this->dz)
          + this->lap(this->Phi, i, j, this->dx, this->dz);
          /* + 1./rho * grad(Phi) * grad(rho) */ // should be added if rho is not constant

        p_err(i ,j) = this->err(i, j);
        lap_p_err(i,j) = this->lap(p_err, i, j, this->dx, this->dz);

	//pseudo-time loop
std::cerr<<"-------------------------------------------------"<<std::endl;
	real_t error = 1.;
	while (error > this->tol)
	{
          tmp_den = this->mem->sum(lap_p_err, lap_p_err, i, j);
          if (tmp_den != 0) beta = - this->mem->sum(this->err, lap_p_err, i, j) / tmp_den;
          this->Phi(i, j) += beta * p_err(i, j);
          this->err(i, j) += beta * lap_p_err(i, j);

          this->lap_err(i, j) = this->lap(this->err, i, j, this->dx, this->dz);         

          if (tmp_den != 0) alpha = - this->mem->sum(this->lap_err, lap_p_err, i, j) / tmp_den;          

          p_err(i, j) *= alpha;
          p_err(i, j) += this->err(i, j);  
 
          lap_p_err(i,j) *= alpha;
          lap_p_err(i,j) += this->lap_err(i,j);
 
          error = std::max(
            std::abs(this->mem->max(this->err(i,j))), 
            std::abs(this->mem->min(this->err(i,j)))
          );
//std::cerr<<error<<std::endl;
          this->iters++;
	}

	//end of pseudo_time loop
	this->xchng(this->Phi, i^halo, j^halo);

	this->tmp_u(i, j) -= grad<0>(this->Phi, i, j, this->dx);
	this->tmp_w(i, j) -= grad<1>(this->Phi, j, i, this->dz);

	this->tmp_u(i, j) -= this->state(u)(i, j);
	this->tmp_w(i, j) -= this->state(w)(i, j);
      }

      public:

      struct params_t : parent_t::params_t { };

      // ctor
      pressure_cr(
	typename parent_t::ctor_args_t args,
	const params_t &p
      ) :
	parent_t(args, p),
        lap_p_err(args.mem->tmp[__FILE__][0][0]),
	p_err(args.mem->tmp[__FILE__][0][1])
      {}

      static void alloc(typename parent_t::mem_t *mem, const int nx, const int ny)
      {
        parent_t::alloc(mem, nx, ny);

        const rng_t i(0, nx-1), j(0, ny-1);
        const int halo = parent_t::halo; 

        // temporary fields
        mem->tmp[__FILE__].push_back(new arrvec_t<typename parent_t::arr_t>());
	for (int n=0; n < 2; ++n) 
	  mem->tmp[__FILE__].back().push_back(new typename parent_t::arr_t(i^halo, j^halo)); 
      }
    }; 
  }; // namespace solvers
}; // namespace advoocat
