#pragma once

#include "CommandBuffer.h"
#include "Buffer.h"
#include "View.h"
#include "Pipeline.h"

namespace Skateboard
{
	using VertexBuffer = VertexBufferView;
	using IndexBuffer = IndexBufferView;

	using Texture = TextureSRVRef;
}