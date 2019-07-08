#ifndef __BRICA2_MPI_DATATYPE_HPP__
#define __BRICA2_MPI_DATATYPE_HPP__

#include "mpi.h"

NAMESPACE_BEGIN(BRICA2_NAMESPACE)
NAMESPACE_BEGIN(mpi)

template <class T> MPI_Datatype datatype();
template <> MPI_Datatype datatype<char>() { return MPI_CHAR; }
template <> MPI_Datatype datatype<short>() { return MPI_SHORT; }
template <> MPI_Datatype datatype<int>() { return MPI_INT; }
template <> MPI_Datatype datatype<long>() { return MPI_LONG; }
template <> MPI_Datatype datatype<long long>() { return MPI_LONG_LONG; }
template <> MPI_Datatype datatype<float>() { return MPI_FLOAT; }
template <> MPI_Datatype datatype<double>() { return MPI_DOUBLE; }
template <> MPI_Datatype datatype<long double>() { return MPI_LONG_DOUBLE; }
template <> MPI_Datatype datatype<unsigned char>() { return MPI_UNSIGNED_CHAR; }
template <> MPI_Datatype datatype<unsigned short>() {
  return MPI_UNSIGNED_SHORT;
}
template <> MPI_Datatype datatype<unsigned>() { return MPI_UNSIGNED; }
template <> MPI_Datatype datatype<unsigned long>() { return MPI_UNSIGNED_LONG; }
template <> MPI_Datatype datatype<unsigned long long>() {
  return MPI_UNSIGNED_LONG_LONG;
}

NAMESPACE_END(mpi)
NAMESPACE_END(BRICA2_NAMESPACE)

#endif  // __BRICA2_MPI_DATATYPE_HPP__
