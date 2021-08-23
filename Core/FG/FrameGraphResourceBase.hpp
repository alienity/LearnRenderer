#pragma once
#ifndef FG_RESOURCE_BASE_HPP_
#define FG_RESOURCE_BASE_HPP_

#include <cstddef>
#include <string>
#include <vector>

namespace FG
{
	class FrameGraph;
	class FrameGraphPassBase;
	
	class FrameGraphResourceBase
	{
	public:
		explicit FrameGraphResourceBase(const std::string& name, const FrameGraphPassBase* creator)
			: _name(name), _creator(creator), _refCount(0)
		{
			static std::size_t id = 0;
			_id = id++;
		}
		FrameGraphResourceBase(const FrameGraphResourceBase& that) = delete;
		FrameGraphResourceBase(FrameGraphResourceBase&& temp) = default;
		virtual ~FrameGraphResourceBase() = default;
		FrameGraphResourceBase& operator=(const FrameGraphResourceBase& that) = delete;
		FrameGraphResourceBase& operator=(FrameGraphResourceBase&& temp) = default;

		std::size_t Id() const
		{
			return _id;
		}

		const std::string& Name() const
		{
			return _name;
		}
		void SetName(const std::string& name)
		{
			_name = name;
		}

		bool Transient() const
		{
			return _creator != nullptr;
		}

	protected:
		friend FrameGraph;
		friend FrameGraphBuilder;

		virtual void Realize() = 0;
		virtual void DeRealize() = 0;

		std::size_t                             _id;
		std::string                             _name;
		const FrameGraphPassBase*               _creator;
		std::vector<const FrameGraphPassBase*>  _readers;
		std::vector<const FrameGraphPassBase*>  _writers;
		std::size_t                             _refCount; // Computed through framegraph compilation.
	};


}

#endif