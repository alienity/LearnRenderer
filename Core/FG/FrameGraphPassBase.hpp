#pragma once
#ifndef FG_FRAME_GRAPH_PASS_BASE_HPP_
#define FG_FRAME_GRAPH_PASS_BASE_HPP_

#include <cstddef>
#include <string>
#include <vector>

namespace FG
{
	class FrameGraph;
	class FrameGraphResourceBase;
	//class FrameGraphBuilder;

	class FrameGraphPassBase
	{
	public:
		explicit FrameGraphPassBase(const std::string& name) :
			_name(name), _cullImmune(false), _refCount(0)
		{

		}
		FrameGraphPassBase(const FrameGraphPassBase& that) = delete;
		FrameGraphPassBase(FrameGraphPassBase&& temp) = default;
		virtual ~FrameGraphPassBase() = default;
		FrameGraphPassBase& operator=(const FrameGraphPassBase& that) = delete;
		FrameGraphPassBase& operator=(FrameGraphPassBase&& temp) = default;

		const std::string& Name() const
		{
			return _name;
		}
		void SetName(const std::string& name)
		{
			_name = name;
		}

		bool CullImmune() const
		{
			return _cullImmune;
		}
		void SetCullImmune(const bool cullImmune)
		{
			_cullImmune = cullImmune;
		}

	protected:
		friend class FrameGraph;
		friend class FrameGraphBuilder;

		virtual void Setup(FrameGraphBuilder& builder) = 0;
		virtual int Execute() const = 0;

		std::string                                 _name;
		bool                                        _cullImmune;
		std::vector<const FrameGraphResourceBase*>  _creates;
		std::vector<const FrameGraphResourceBase*>  _reads;
		std::vector<const FrameGraphResourceBase*>  _writes;
		std::size_t                                 _refCount; // Computed through framegraph compilation.
	};
}

#endif