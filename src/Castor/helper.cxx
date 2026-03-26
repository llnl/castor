// Copyright 2024-2026 Lawrence Livermore National Security, LLC and other Castor Developers.
// See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef __R2WMLHELP
#define __R2WMLHELP

#include "exception.hxx"
#include <memory>

///
/// @brief Attempt to cast a shared pointer using std::dynamic_pointer_cast
///
/// Throw an exception if the cast fails.
///
/// @param ptr Input pointer
/// @tparam T Type to cast to
/// @tparam U Type to cast from
/// @return The casted pointer
///
template <typename T, typename U>
std::shared_ptr<T> safety_cast(U ptr)
{
	std::shared_ptr<T> rptr = std::dynamic_pointer_cast<T>(ptr); // do the cast
	
	if (rptr)
		return rptr;
	else
		throw InvalidCastException(typeid(T).name(), typeid(U).name()); // give some diagnostics if it fails
}

///
/// @brief Attempt to cast a naked pointer using dynamic_cast
///
/// Throw an exception if the cast fails.
///
/// @param ptr Input pointer
/// @tparam T Type to cast to
/// @tparam U Type to cast from
/// @return The casted pointer
///
template <typename T, typename U>
T safety_cast_raw(U ptr)
{
	T rptr = dynamic_cast<T>(ptr); // do the cast
	
	if (rptr)
		return rptr;
	else
		throw InvalidCastException(typeid(T).name(), typeid(U).name()); // give some diagnostics if it fails
}

#endif
