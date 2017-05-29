#pragma once

#if defined(USE_MPI)
#  include <boost/mpi/environment.hpp>
#  include <boost/mpi/communicator.hpp>
#  include <boost/mpi/collectives.hpp>
#  include <mutex>
#else
#  include <cstdlib>
#endif


namespace libmpdataxx
{
  namespace concurr
  {
    namespace detail
    {
#if defined(USE_MPI)
      namespace 
      {
        // the anonymous namespace is a hack to avoid
        // "*** The MPI_Errhandler_set() function was called after MPI_FINALIZE was invoked."
        // error if multiple solvers are instantiated 
        boost::mpi::environment mpienv(boost::mpi::threading::serialized);
        // TODO: the ``multiple'' threading level could be reduced to single in
        // case the sharedmem->size() == 1
      }
      std::mutex mpi_mutex;
#endif

      template <typename real_t, int n_dims>
      class distmem
      {
#if defined(USE_MPI)
        boost::mpi::communicator mpicom;
#endif

        private:

        template <typename Op>
        real_t reduce_hlpr(const real_t &val)
        {
#if defined(USE_MPI)
          real_t res;
          std::lock_guard<std::mutex> lock(mpi_mutex);
          boost::mpi::all_reduce(mpicom, val, res, Op());
          return res;
#else
          return val;
#endif
        }


        public:

        std::array<int, n_dims> grid_size;

        int rank() 
        { 
#if defined(USE_MPI)
          return mpicom.rank();  // is it thread-safe? TODO: init once in ctor?
#else
          return 0;
#endif
        }

        int size() 
        {
#if defined(USE_MPI)
          return mpicom.size();   // is it thread-safe? TODO: init once in ctor?
#else
          return 1;
#endif
        }

        void barrier()
        {
#if defined(USE_MPI)
          mpicom.barrier();
#else
          assert(false);
#endif
        }

        real_t min(const real_t &val)
        {
#if defined(USE_MPI)
          return reduce_hlpr<boost::mpi::minimum<real_t>>(val);
#else
          return val;
#endif
        }

        real_t max(const real_t &val)
        {
#if defined(USE_MPI)
          return reduce_hlpr<boost::mpi::maximum<real_t>>(val);
#else
          return val;
#endif
        }

        real_t sum(const real_t &val)
        {
          return reduce_hlpr<std::plus<real_t>>(val);
        }

        // ctor
        distmem(const std::array<int, n_dims> &grid_size) 
          : grid_size(grid_size)
        {
#if !defined(USE_MPI)
          if (
            // mpich
            std::getenv("PMI_RANK") != NULL ||
            // openmpi
            std::getenv("OMPI_COMM_WORLD_RANK") != NULL ||
            // lam
            std::getenv("LAMRANK") != NULL
          ) throw std::runtime_error("mpirun environment variable detected but libmpdata++ was compiled with MPI disabled");
#else
          if (boost::mpi::environment::thread_level() != boost::mpi::threading::serialized)
          {
            throw std::runtime_error("failed to initialise MPI environment with MPI_THREAD_SERIALIZED");
          }
#endif
        }
      };
    }
  }
}
