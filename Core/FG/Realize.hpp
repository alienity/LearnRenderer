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

	// 因为d3d12的资源释放一定要是gpu异步计算操作执行结束之后，所以需要手动管理资源的释放
	template<typename _ActualType>
	void DeRealize(std::unique_ptr<_ActualType>& actual_ptr, int fence)
	{
		static_assert(actual_ptr == nullptr, "actulal_ptr is nullptr, you should control the lifetime of the object by your own.");
	}

}

#endif