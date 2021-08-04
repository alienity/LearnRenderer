#pragma once
#ifndef FG_REALIZE_HPP_
#define FG_REALIZE_HPP_

#include <memory>
#include <type_traits>

namespace FG
{
	template<typename _DescriptionType, typename _ActualType> 
	struct MissingRealizeImplementation : std::false_type {};

	template<typename _DescriptionType, typename _ActualType>
	std::unique_ptr<_ActualType> Realize(const _DescriptionType& description)
	{
		static_assert(MissingRealizeImplementation<_DescriptionType, _ActualType>::value, "Missing realize implementation for description - type pair.");
		return nullptr;
	}
}

#endif