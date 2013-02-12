/** @file
 * @copyright University of Warsaw
 * @section LICENSE
 * GPLv3+ (see the COPYING file or http://www.gnu.org/licenses/)
 */

#pragma once

#include "solver_common.hpp"
#include "../../bcond/bcond.hpp"

namespace advoocat
{
  namespace solvers
  {
    namespace detail
    {
      using namespace advoocat::arakawa_c;

      template<class mem_t, int n_tlev, int halo>
      class solver_1d : public solver_common<mem_t, n_tlev, halo>
      {
	using parent_t = solver_common<mem_t, n_tlev, halo>;
	using arr_t = typename mem_t::arr_t;

	protected:

        typedef std::unique_ptr<bcond::bcond_t<typename mem_t::real_t>> bc_p; // TODO: move to parent
        bc_p bcxl, bcxr;
     
	rng_t i; // TODO: idx_t i do common?

	void xchng(int e, int lev = 0) 
	{
          //this->mem.barrier();
	  bcxl->fill_halos( this->mem.psi[e][ this->mem.n[e] - lev ] );
	  bcxr->fill_halos( this->mem.psi[e][ this->mem.n[e] - lev ] );
          //this->mem.barrier();
	}

	// ctor
	solver_1d(mem_t &mem, bc_p &bcxl, bc_p &bcxr, const rng_t &i) :
	  parent_t(mem), i(i), bcxl(std::move(bcxl)), bcxr(std::move(bcxr))
	{
	}

	public:

	static void alloc(mem_t &mem, const int nx)   
        {
          const rng_t i(0, nx-1);
          
	  for (int e = 0; e < mem_t::n_eqs; ++e) // equations
	    for (int n = 0; n < n_tlev; ++n) // time levels
	      mem.psi[e].push_back(new arr_t(i^halo));
    
	  mem.C.push_back(new arr_t(i^h));
        } 
      };
    }; // namespace detail
  }; // namespace solvers
}; // namespace advoocat