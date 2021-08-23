#pragma once
#ifndef FG_FRAME_GRAPH_PASS_HPP_
#define FG_FRAME_GRAPH_PASS_HPP_

#include <functional>
#include <string>

#include "FrameGraphPassBase.hpp"

namespace FG
{
	class FrameGraphBuilder;

	template<typename _DataType>
	class FrameGraphPass : public FrameGraphPassBase
	{
	public:
		using DataType = _DataType;

		explicit FrameGraphPass(
			const std::string& name,
			const std::function<void(DataType&, FrameGraphBuilder&)>& setup,
			const std::function<void(const DataType&)>& execute) : FrameGraphPassBase(name), _setup(setup), _execute(execute)
		{

		}
		FrameGraphPass(const FrameGraphPass& that) = delete;
		FrameGraphPass(FrameGraphPass&& temp) = default;
		virtual ~FrameGraphPass() = default;
		FrameGraphPass& operator=(const FrameGraphPass& that) = delete;
		FrameGraphPass& operator=(FrameGraphPass&& temp) = default;

		const DataType& Data() const
		{
			return _data;
		}

	protected:
		void Setup(FrameGraphPassBuilder& builder) override
		{
			_setup(_data, builder);
		}
		void Execute() const override
		{
			_execute(_data);
		}

		DataType                                                      _data;
		const std::function<void(DataType&, FrameGraphBuilder&)>  _setup;
		const std::function<void(const DataType&)>                    _execute;
	};
}

#endif