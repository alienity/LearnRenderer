#pragma once
#ifndef FG_RENDER_GRAPH_PASS_BUILDER_HPP_
#define FG_RENDER_GRAPH_PASS_BUILDER_HPP_

#include <string>

namespace FG
{
	class FrameGraph;
	class FrameGraphPassBase;

	// The interface between the framegraph and a render pass.
	class FrameGraphBuilder
	{
	public:
		explicit FrameGraphBuilder(FrameGraph* framegraph, FrameGraphPassBase* renderpass) :
			_framegraph(framegraph), _renderpass(renderpass)
		{

		}
		FrameGraphBuilder(const FrameGraphBuilder& that) = default;
		FrameGraphBuilder(FrameGraphBuilder&& temp) = default;
		virtual ~FrameGraphBuilder() = default;
		FrameGraphBuilder& operator=(const FrameGraphBuilder& that) = default;
		FrameGraphBuilder& operator=(FrameGraphBuilder&& temp) = default;

		template<typename ResourceType, typename DescriptionType> 
		ResourceType* Create(const std::string& name, const DescriptionType& description);
		template<typename ResourceType>
		ResourceType* Read(ResourceType* resource);
		template<typename ResourceType>
		ResourceType* Write(ResourceType* resource);

	protected:
		FrameGraph*          _framegraph;
		FrameGraphPassBase*  _renderpass;
	};
}

#endif