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
		int Width;
		int Height;
		int NumMips;
		int ArrayCount;
		DXGI_FORMAT Format;
		int NumColorSamples;
		int NumCoverageSamples;
	};

	struct DepthBufferDescription
	{
		float ClearDepth;
		float ClearStencil;
		int Width;
		int Height;
		int NumSamples;
		DXGI_FORMAT Format;
	};

	struct ShadowBufferDescription
	{
		int Width;
		int Height;
	};

	struct ByteAddressBufferDescription
	{
		int NumElements;
		int ElementSize;
	};

	struct IndirectArgsBufferDescription {
		int NumElements;
		int ElementSize;
	};

	struct StructuredBufferDescription {
		int NumElements;
		int ElementSize;
	};

	struct TypedBufferDescription {
		int NumElements;
		int ElementSize;
		DXGI_FORMAT Format;
	};

	using ColorBufferResource = FG::FrameGraphResource<ColorBufferDescription, ColorBuffer>;
	using DepthBufferResource = FG::FrameGraphResource<DepthBufferDescription, DepthBuffer>;
	using ShadowBufferResource = FG::FrameGraphResource<ShadowBufferDescription, ShadowBuffer>;
	using ByteAddressBufferResource = FG::FrameGraphResource<ByteAddressBufferDescription, ByteAddressBuffer>;
	using IndirectArgsBufferResource = FG::FrameGraphResource<IndirectArgsBufferDescription, IndirectArgsBuffer>;
	using StructuredBufferResource = FG::FrameGraphResource<StructuredBufferDescription, StructuredBuffer>;
	using TypedBufferResource = FG::FrameGraphResource<TypedBufferDescription, TypedBuffer>;

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

}