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

	// ��Ϊd3d12����Դ�ͷ�һ��Ҫ��gpu�첽�������ִ�н���֮��������Ҫ�ֶ�������Դ���ͷ�
	template<typename _ActualType>
	void DeRealize(std::unique_ptr<_ActualType>& actual_ptr, int fence)
	{
		static_assert(actual_ptr == nullptr, "actulal_ptr is nullptr, you should control the lifetime of the object by your own.");
	}

}

#endif