/** @file
* @copyright University of Warsaw
* @section LICENSE
* GPLv3+ (see the COPYING file or http://www.gnu.org/licenses/)
*/

#pragma once

#include <libmpdata++/bcond/bcond.hpp>

namespace libmpdataxx
{
  namespace bcond
  {
    template <typename real_t>
    class open_left_1d : public bcond_t<real_t>
    {
      using parent_t = bcond_t<real_t>;
      using arr_t = blitz::Array<real_t, 1>;
      using parent_t::parent_t; // inheriting ctor
      
      real_t init_sclr;

      public:

      // methods invoked by the solver
      void bcinit(const arr_t &a)
      {
        assert(min(a(this->left_edge_sclr)) == max(a(this->left_edge_sclr))
               && "variable initial signal on open boundary");

	init_sclr = min(a(this->left_edge_sclr));
      }

      void fill_halos_sclr(const arr_t &a)
      {
        for (int i = this->left_halo_sclr.first(); i <= this->left_halo_sclr.last(); ++i)
	  a(rng_t(i, i)) = init_sclr;
      }

      void fill_halos_vctr_alng(const arr_t &a)
      {
        for (int i = this->left_halo_vctr.first(); i <= this->left_halo_vctr.last(); ++i)
	  a(rng_t(i, i)) = a(this->left_intr_vctr.first());
      }
    };

    template <typename real_t>
    class open_rght_1d : public bcond_t<real_t>
    {
      using parent_t = bcond_t<real_t>;
      using arr_t = blitz::Array<real_t, 1>;
      using parent_t::parent_t; // inheriting ctor
      
      real_t init_sclr;

      public:

      // methods invoked by the solver
      void bcinit(const arr_t &a)
      {
	init_sclr = min(a(this->rght_edge_sclr));
      }

      void fill_halos_sclr(const arr_t &a)
      {
        for (int i = this->rght_halo_sclr.first(); i <= this->rght_halo_sclr.last(); ++i)
	  a(rng_t(i, i)) = init_sclr;
      }

      void fill_halos_vctr_alng(const arr_t &a)
      {
        for (int i = this->rght_halo_vctr.first(); i <= this->rght_halo_vctr.last(); ++i)
	  a(rng_t(i, i)) = a(this->rght_intr_vctr.first());
      }
      
    };
  }; // namespace bcond
}; // namespace libmpdataxx
