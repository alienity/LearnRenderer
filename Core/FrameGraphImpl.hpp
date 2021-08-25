#pragma once

#include "FG/FrameGraph.hpp"

#include "d3dx12.h"
#include "Color.h"

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "ShadowBuffer.h"
#include "GpuBuffer.h"

namespace FG
{
	struct ColorBufferDescription
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t NumMips;
		uint32_t ArrayCount;
		DXGI_FORMAT Format;
		uint32_t NumColorSamples;
		uint32_t NumCoverageSamples;
	};

	struct DepthBufferDescription
	{
		float ClearDepth;
		uint8_t ClearStencil;
		uint32_t Width;
		uint32_t Height;
		uint32_t NumSamples;
		DXGI_FORMAT Format;
	};

	struct ShadowBufferDescription
	{
		uint32_t Width;
		uint32_t Height;
	};

	struct ByteAddressBufferDescription
	{
		uint32_t NumElements;
		uint32_t ElementSize;
	};

	struct IndirectArgsBufferDescription {
		uint32_t NumElements;
		uint32_t ElementSize;
	};

	struct StructuredBufferDescription {
		uint32_t NumElements;
		uint32_t ElementSize;
	};

	struct TypedBufferDescription {
		uint32_t NumElements;
		uint32_t ElementSize;
		DXGI_FORMAT Format;
	};

	using ColorBufferResource = FG::FrameGraphResource<ColorBufferDescription, ColorBuffer>;
	using DepthBufferResource = FG::FrameGraphResource<DepthBufferDescription, DepthBuffer>;
	using ShadowBufferResource = FG::FrameGraphResource<ShadowBufferDescription, ShadowBuffer>;
	using ByteAddressBufferResource = FG::FrameGraphResource<ByteAddressBufferDescription, ByteAddressBuffer>;
	using IndirectArgsBufferResource = FG::FrameGraphResource<IndirectArgsBufferDescription, IndirectArgsBuffer>;
	using StructuredBufferResource = FG::FrameGraphResource<StructuredBufferDescription, StructuredBuffer>;
	using TypedBufferResource = FG::FrameGraphResource<TypedBufferDescription, TypedBuffer>;

	std::list<std::pair<int, std::unique_ptr<ColorBuffer>>> g_ColorBufferCache;
	std::list<std::pair<int, std::unique_ptr<DepthBuffer>>> g_DepthBufferCache;
	std::list<std::pair<int, std::unique_ptr<ShadowBuffer>>> g_ShadowBufferCache;
	std::list<std::pair<int, std::unique_ptr<ByteAddressBuffer>>> g_ByteAddressBufferCache;
	std::list<std::pair<int, std::unique_ptr<IndirectArgsBuffer>>> g_IndirectArgsBufferCache;
	std::list<std::pair<int, std::unique_ptr<StructuredBuffer>>> g_StructuredBufferCache;
	std::list<std::pair<int, std::unique_ptr<TypedBuffer>>> g_TypedBufferCache;

	template<>
	std::unique_ptr<ColorBuffer> Realize(const ColorBufferDescription& description)
	{
		std::unique_ptr<ColorBuffer> _ColorBuffer = std::make_unique<ColorBuffer>();
		if (description.NumColorSamples >= description.NumCoverageSamples) {
			if (description.NumColorSamples > 1 || description.NumCoverageSamples > 1) {
				_ColorBuffer->SetMsaaMode(description.NumColorSamples, description.NumCoverageSamples);
			}
		}
		ASSERT(!(description.NumMips > 0 && description.ArrayCount > 0));
		if (description.ArrayCount > 0) {
			_ColorBuffer->Create(L"TmpColorArray", description.Width, description.Height, 
				description.ArrayCount, description.Format);
		} else {
			_ColorBuffer->Create(L"TmpColorBuffer", description.Width, description.Height,
				description.NumMips, description.Format);
		}
		return _ColorBuffer;
	}

	template<>
	std::unique_ptr<DepthBuffer> Realize(const DepthBufferDescription& description)
	{
		std::unique_ptr<DepthBuffer> _DepthBuffer = 
			std::make_unique<DepthBuffer>(description.ClearDepth, description.ClearStencil);
		if (description.NumSamples > 1) {
			_DepthBuffer->Create(L"TmpDepthBuffer", description.Width, description.Height, description.Format, description.NumSamples);
		} else {
			_DepthBuffer->Create(L"TmpDepthBuffer", description.Width, description.Height, description.Format);
		}
		return _DepthBuffer;
	}

	template<>
	std::unique_ptr<ShadowBuffer> Realize(const ShadowBufferDescription& description)
	{
		std::unique_ptr<ShadowBuffer> _ShadowBuffer = std::make_unique<ShadowBuffer>();
		_ShadowBuffer->Create(L"TmpShadowBuffer", description.Width, description.Height);
		return _ShadowBuffer;
	}

	template<>
	std::unique_ptr<ByteAddressBuffer> Realize(const ByteAddressBufferDescription& description)
	{
		std::unique_ptr<ByteAddressBuffer> _ByteAddressBuffer = std::make_unique<ByteAddressBuffer>();
		_ByteAddressBuffer->Create(L"TmpByteAddressBuffer", description.NumElements, description.ElementSize);
		return _ByteAddressBuffer;
	}

	template<>
	std::unique_ptr<IndirectArgsBuffer> Realize(const IndirectArgsBufferDescription& description)
	{
		std::unique_ptr<IndirectArgsBuffer> _IndirectArgsBuffer = std::make_unique<IndirectArgsBuffer>();
		_IndirectArgsBuffer->Create(L"TmpIndirectArgsBuffer", description.NumElements, description.ElementSize);
		return _IndirectArgsBuffer;
	}

	template<>
	std::unique_ptr<StructuredBuffer> Realize(const StructuredBufferDescription& description)
	{
		std::unique_ptr<StructuredBuffer> _StructuredBuffer = std::make_unique<StructuredBuffer>();
		_StructuredBuffer->Create(L"TmpStructuredBuffer", description.NumElements, description.ElementSize);
		return _StructuredBuffer;
	}

	template<>
	std::unique_ptr<TypedBuffer> Realize(const TypedBufferDescription& description)
	{
		std::unique_ptr<TypedBuffer> _TypedBuffer = std::make_unique<TypedBuffer>(description.Format);
		_TypedBuffer->Create(L"TmpTypedBuffer", description.NumElements, description.ElementSize);
		return _TypedBuffer;
	}



	template<>
	void DeRealize(std::unique_ptr<ColorBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<ColorBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_ColorBufferCache.push_back(cache_pair);
	}

	template<>
	void DeRealize(std::unique_ptr<DepthBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<DepthBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_DepthBufferCache.push_back(cache_pair);
	}

	template<>
	void DeRealize(std::unique_ptr<ShadowBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<ShadowBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_ShadowBufferCache.push_back(cache_pair);
	}

	template<>
	void DeRealize(std::unique_ptr<ByteAddressBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<ByteAddressBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_ByteAddressBufferCache.push_back(cache_pair);
	}

	template<>
	void DeRealize(std::unique_ptr<IndirectArgsBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<IndirectArgsBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_IndirectArgsBufferCache.push_back(cache_pair);
	}

	template<>
	void DeRealize(std::unique_ptr<StructuredBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<StructuredBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_StructuredBufferCache.push_back(cache_pair);
	}

	template<>
	void DeRealize(std::unique_ptr<TypedBuffer>& actual_ptr, int fence)
	{
		std::pair<int, std::unique_ptr<TypedBuffer>> cache_pair(fence, std::move(actual_ptr));
		g_TypedBufferCache.push_back(cache_pair);
	}


}