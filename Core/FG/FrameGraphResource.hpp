#pragma once
#ifndef FG_RESOURCE_HPP_
#define FG_RESOURCE_HPP_

#include <memory>
#include <string>
#include <variant>

#include "Realize.hpp"
#include "FrameGraphResourceBase.hpp"

namespace FG
{
	class FrameGraphPassBase;

	template<typename _DescriptionType, typename _ActualType>
	class FrameGraphResource : public FrameGraphResourceBase
	{
	public:
		using DescriptionType = _DescriptionType;
		using ActualType = _ActualType;

		explicit FrameGraphResource(const std::string& name, const FrameGraphPassBase* creator, const DescriptionType& description)
			: FrameGraphResourceBase(name, creator), _description(description), _actual(std::unique_ptr<ActualType>())
		{
			// Transient (normal) constructor.
		}
		explicit FrameGraphResource(const std::string& name, const DescriptionType& description, ActualType* actual = nullptr)
			: FrameGraphResourceBase(name, nullptr), _description(description), _actual(actual)
		{
			// Retained (import) constructor.
			if (!actual) _actual = FG::Realize<DescriptionType, ActualType>(_description);
		}
		FrameGraphResource(const FrameGraphResource& that) = delete;
		FrameGraphResource(FrameGraphResource&& temp) = default;
		~FrameGraphResource() = default;
		FrameGraphResource& operator=(const FrameGraphResource& that) = delete;
		FrameGraphResource& operator=(FrameGraphResource&& temp) = default;

		const DescriptionType& Description() const
		{
			return _description;
		}
		ActualType* Actual() const // If transient, only valid through the realized interval of the resource.
		{
			return std::holds_alternative<std::unique_ptr<ActualType>>(_actual) ? std::get<std::unique_ptr<ActualType>>(_actual).get() : std::get<ActualType*>(_actual);
		}

	protected:
		void Realize() override
		{
			if (Transient()) std::get<std::unique_ptr<ActualType>>(_actual) = FG::Realize<DescriptionType, ActualType>(_description);
		}
		void DeRealize(int fence) override
		{
			//if (Transient()) std::get<std::unique_ptr<ActualType>>(_actual).reset();
			if (Transient()) FG::DeRealize(std::get<std::unique_ptr<ActualType>(_actual), fence));
		}

		DescriptionType                                         _description;
		std::variant<std::unique_ptr<ActualType>, ActualType*>  _actual;
	};
}

#endif