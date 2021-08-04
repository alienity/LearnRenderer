#pragma once
#ifndef FG_FRAMEGRAPH_HPP_
#define FG_FRAMEGRAPH_HPP_

#include <algorithm>
#include <fstream>
#include <iterator>
#include <memory>
#include <stack>
#include <string>
#include <type_traits>
#include <vector>

#include <FrameGraphPass.hpp>
#include <FrameGraphPassBuilder.hpp>
#include <FrameGraphResource.hpp>

namespace FG
{
	class FrameGraph
	{
	public:
		FrameGraph() = default;
		FrameGraph(const FrameGraph& that) = delete;
		FrameGraph(FrameGraph&& temp) = default;
		virtual ~FrameGraph() = default;
		FrameGraph& operator=(const FrameGraph& that) = delete;
		FrameGraph& operator=(FrameGraph&& temp) = default;

		template<typename DataType, typename... ArgumentTypes>
		FrameGraphPass<DataType>* AddRenderPass(ArgumentTypes&&... arguments)
		{
			_renderPasses.emplace_back(std::make_unique<FrameGraphPass<DataType>>(arguments...));
			auto renderPass = _renderPasses.back().get();

			FrameGraphPassBuilder builder(this, renderPass);
			renderPass->Setup(builder);

			return static_cast<FG::FrameGraphPass<DataType>*>(renderPass);
		}
		template<typename DescriptionType, typename ActualType>
		FrameGraphResource<DescriptionType, ActualType>* AddRetainedResource(const std::string& name, const DescriptionType& description, ActualType* actual = nullptr)
		{
			_resources.emplace_back(std::make_unique<FrameGraphResource<DescriptionType, ActualType>>(name, description, actual));
			return static_cast<FrameGraphResource<DescriptionType, ActualType>*>(_resources.back().get());
		}
		void Compile()
		{
			// Reference counting.
			for (auto& renderPass : _renderPasses)
				renderPass->_refCount = renderPass->_creates.size() + renderPass->_writes.size();
			for (auto& resource : _resources)
				resource->_refCount = resource->_readers.size();

			// Culling via flood fill from unreferenced resources.
			std::stack<FrameGraphResourceBase*> unreferencedResources;
			for (auto& resource : _resources)
				if (resource->_refCount == 0 && resource->Transient())
					unreferencedResources.push(resource.get());
			while (!unreferencedResources.empty())
			{
				auto unreferencedResource = unreferencedResources.top();
				unreferencedResources.pop();

				auto creator = const_cast<FrameGraphPassBase*>(unreferencedResource->_creator);
				if (creator->_refCount > 0)
					creator->_refCount--;
				if (creator->_refCount == 0 && !creator->CullImmune())
				{
					for (auto iteratee : creator->_reads)
					{
						auto readResource = const_cast<FrameGraphResourceBase*>(iteratee);
						if (readResource->_refCount > 0)
							readResource->_refCount--;
						if (readResource->_refCount == 0 && readResource->Transient())
							unreferencedResources.push(readResource);
					}
				}

				for (auto c_writer : unreferencedResource->_writers)
				{
					auto writer = const_cast<FrameGraphPassBase*>(c_writer);
					if (writer->_refCount > 0)
						writer->_refCount--;
					if (writer->_refCount == 0 && !writer->CullImmune())
					{
						for (auto iteratee : writer->_reads)
						{
							auto readResource = const_cast<FrameGraphResourceBase*>(iteratee);
							if (readResource->_refCount > 0)
								readResource->_refCount--;
							if (readResource->_refCount == 0 && readResource->Transient())
								unreferencedResources.push(readResource);
						}
					}
				}
			}

			// Timeline computation.
			_timeline.clear();
			for (auto& renderPass : _renderPasses)
			{
				if (renderPass->_refCount == 0 && !renderPass->CullImmune())
					continue;

				std::vector<FrameGraphResourceBase*> realizedResources, derealizedResources;

				for (auto resource : renderPass->_creates)
				{
					realizedResources.push_back(const_cast<FrameGraphResourceBase*>(resource));
					if (resource->_readers.empty() && resource->_writers.empty())
						derealizedResources.push_back(const_cast<FrameGraphResourceBase*>(resource));
				}

				auto reads_writes = renderPass->_reads;
				reads_writes.insert(reads_writes.end(), renderPass->_writes.begin(), renderPass->_writes.end());
				for (auto resource : reads_writes)
				{
					if (!resource->Transient())
						continue;

					bool valid = false;
					std::size_t last_index;
					if (!resource->_readers.empty())
					{
						auto last_reader = std::find_if(
							_renderPasses.begin(),
							_renderPasses.end(),
							[&resource](const std::unique_ptr<FrameGraphPassBase>& iteratee)
							{
								return iteratee.get() == resource->_readers.back();
							});
						if (last_reader != _renderPasses.end())
						{
							valid = true;
							last_index = std::distance(_renderPasses.begin(), last_reader);
						}
					}
					if (!resource->_writers.empty())
					{
						auto last_writer = std::find_if(
							_renderPasses.begin(),
							_renderPasses.end(),
							[&resource](const std::unique_ptr<FrameGraphPassBase>& iteratee)
							{
								return iteratee.get() == resource->_writers.back();
							});
						if (last_writer != _renderPasses.end())
						{
							valid = true;
							last_index = std::max(last_index, std::size_t(std::distance(_renderPasses.begin(), last_writer)));
						}
					}

					if (valid && _renderPasses[last_index] == renderPass)
						derealizedResources.push_back(const_cast<FrameGraphResourceBase*>(resource));
				}

				_timeline.push_back(Step{ renderPass.get(), realizedResources, derealizedResources });
			}
		}
		void Execute() const
		{
			for (auto& step : _timeline)
			{
				for (auto resource : step.realizedResources) resource->Realize();
				step.renderPass->Execute();
				for (auto resource : step.derealizedResources) resource->DeRealize();
			}
		}
		void Clear()
		{
			_renderPasses.clear();
			_resources.clear();
		}
		void ExportGraphviz(const std::string& filepath)
		{
			std::ofstream stream(filepath);
			stream << "digraph framegraph \n{\n";

			stream << "rankdir = LR\n";
			stream << "bgcolor = black\n\n";
			stream << "node [shape=rectangle, fontname=\"helvetica\", fontsize=12]\n\n";

			for (auto& renderPass : _renderPasses)
				stream << "\"" << renderPass->Name() << "\" [label=\"" << renderPass->Name() << "\\nRefs: " << renderPass->_refCount << "\", style=filled, fillcolor=darkorange]\n";
			stream << "\n";

			for (auto& resource : _resources)
				stream << "\"" << resource->Name() << "\" [label=\"" << resource->Name() << "\\nRefs: " << resource->_refCount << "\\nID: " << resource->Id() << "\", style=filled, fillcolor= " << (resource->Transient() ? "skyblue" : "steelblue") << "]\n";
			stream << "\n";

			for (auto& renderPass : _renderPasses)
			{
				stream << "\"" << renderPass->Name() << "\" -> { ";
				for (auto& resource : renderPass->_creates)
					stream << "\"" << resource->Name() << "\" ";
				stream << "} [color=seagreen]\n";

				stream << "\"" << renderPass->Name() << "\" -> { ";
				for (auto& resource : renderPass->_writes)
					stream << "\"" << resource->Name() << "\" ";
				stream << "} [color=gold]\n";
			}
			stream << "\n";

			for (auto& resource : _resources)
			{
				stream << "\"" << resource->Name() << "\" -> { ";
				for (auto& renderPass : resource->_readers)
					stream << "\"" << renderPass->Name() << "\" ";
				stream << "} [color=firebrick]\n";
			}
			stream << "}";
		}

	protected:
		friend FrameGraphPassBuilder;

		struct Step
		{
			FrameGraphPassBase* renderPass;
			std::vector<FrameGraphResourceBase*> realizedResources;
			std::vector<FrameGraphResourceBase*> derealizedResources;
		};

		std::vector<std::unique_ptr<FrameGraphPassBase>>      _renderPasses;
		std::vector<std::unique_ptr<FrameGraphResourceBase>>  _resources;
		std::vector<Step>                                     _timeline; // Computed through framegraph compilation.
	};

	template<typename ResourceType, typename DescriptionType>
	ResourceType* FrameGraphPassBuilder::Create(const std::string& name, const DescriptionType& description)
	{
		static_assert(std::is_same<typename ResourceType::DescriptionType, DescriptionType>::value, "Description does not match the resource.");
		_framegraph->_resources.emplace_back(std::make_unique<ResourceType>(name, _renderpass, description));
		const auto resource = _framegraph->_resources.back().get();
		_renderpass->_creates.push_back(resource);
		return static_cast<ResourceType*>(resource);
	}
	template<typename ResourceType>
	ResourceType* FrameGraphPassBuilder::Read(ResourceType* resource)
	{
		resource->_readers.push_back(_renderpass);
		_renderpass->_reads.push_back(resource);
		return resource;
	}
	template<typename ResourceType>
	ResourceType* FrameGraphPassBuilder::Write(ResourceType* resource)
	{
		resource->_writers.push_back(_renderpass);
		_renderpass->_writes.push_back(resource);
		return resource;
	}
}

#endif