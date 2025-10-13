#include "AGCCommandBuffer.h"

#include "AGC/Graphics/AGCF.h"
#include "AGC/Graphics/RHI/AGCGraphicsContext.h"

Skateboard::AGCComputeCommandBuffer::AGCComputeCommandBuffer(CommandBufferPriority_ prior) :
	ComputeCommandBuffer(prior), m_ACB(), m_RootCompute(nullptr)
{
	m_CommandBufferMemory = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<
		uint32_t, sce::Agc::Alignment::kCommandBuffer>(SKTBD_DCB_SIZE);

	m_ACB.init(m_CommandBufferMemory.data, m_CommandBufferMemory.count * sizeof(uint32_t), nullptr, nullptr);
}

void Skateboard::AGCComputeCommandBuffer::SetDebugName(const std::wstring& debug_name)
{
	ComputeCommandBuffer::SetDebugName(debug_name);
	CommandBuffer::SetDebugName(debug_name);
	sce::Agc::Core::registerResource(&m_ACB, ToString(debug_name).c_str());
}

Skateboard::AGCComputeCommandBuffer::~AGCComputeCommandBuffer()
{
	gAGCContext->GetMemAllocator()->TypedAlignedFree(m_CommandBufferMemory);
}

void Skateboard::AGCComputeCommandBuffer::SetCurrentPipeline(Pipeline* pipeline)
{
	SKTBD_LOG_ASSERT(pipeline->GetType() == PipelineType_Compute, "only compute pipelines can be set on compute command buffers")
	m_CurrentPipeline = static_cast<AGCComputePipeline*>(pipeline);

	const auto& State = dynamic_cast<IAGCPipelineInterface*>(pipeline)->GetRegisterState();

	const auto SHRegisters = State.GetShRegisters(); auto SHnum = State.GetShCount();
	const auto UCRegisters = State.GetUcRegisters(); auto UCnum = State.GetUcCount();

	m_ACB.setUcRegistersDirect(UCRegisters, UCnum);
	m_ACB.setShRegistersDirect(SHRegisters, SHnum);
}

void Skateboard::AGCComputeCommandBuffer::SetShaderInputGraphics(ShaderInputLayout*)
{
	SKTBD_MSG_WARN("Setting Shader Inputs for graphics on compute only buffer has not effect")
}

void Skateboard::AGCComputeCommandBuffer::SetShaderInputCompute(ShaderInputLayout* compute_layout)
{
	m_RootCompute = static_cast<AGCShaderInputLayout*>(compute_layout);
	
}

Skateboard::AGCShaderInputLayout* Skateboard::AGCComputeCommandBuffer::GetComputeLayout()
{
	m_ComputeResourcesMustBeRebound = true;
	return m_RootCompute;
}

Skateboard::AGCShaderInputLayout* Skateboard::AGCComputeCommandBuffer::GetComputeLayout() const
{
	return m_RootCompute;
}

Skateboard::AGCGraphicsCommandBuffer::AGCGraphicsCommandBuffer(CommandBufferPriority_ prior) :
	GraphicsCommandBuffer(prior), m_VertexBuffers(),
	m_CurrentPipeline(nullptr),
	m_RootGraphics(nullptr),
	m_RootCompute(nullptr),
	m_ComputeResourcesMustBeRebound(false),
	m_GraphicsResourcesMustBeRebound(false),
	m_VertexBufferDataMustBeRebound(false)
{
	m_CommandBufferMemory = gAGCContext->GetMemAllocator()->TypedAlignedAllocate<
		uint32_t, sce::Agc::Alignment::kCommandBuffer>(SKTBD_DCB_SIZE);

	m_DCB.init(m_CommandBufferMemory.data, m_CommandBufferMemory.count * sizeof(uint32_t), nullptr, nullptr);
	m_STB.init(256, &m_DCB, &m_DCB);
}

void Skateboard::AGCGraphicsCommandBuffer::SetDebugName(const std::wstring& debug_name)
{
	CommandBuffer::SetDebugName(debug_name);
	sce::Agc::Core::registerResource(&m_DCB, ToString(debug_name).c_str());
}

Skateboard::AGCGraphicsCommandBuffer::~AGCGraphicsCommandBuffer()
{
	gAGCContext->GetMemAllocator()->TypedAlignedFree(m_CommandBufferMemory);
}

void Skateboard::AGCGraphicsCommandBuffer::SetCurrentPipeline(Pipeline* pipeline)
{
	m_CurrentPipeline = pipeline;

	const auto& State = dynamic_cast<IAGCPipelineInterface*>(pipeline)->GetRegisterState();

	auto CXRegisters = State.GetCxRegisters(); auto CXnum = State.GetCxCount();
	auto SHRegisters = State.GetShRegisters(); auto SHnum = State.GetShCount();
	auto UCRegisters = State.GetUcRegisters(); auto UCnum = State.GetUcCount();

	m_DCB.setCxRegistersIndirect(CXRegisters, CXnum);
	m_DCB.setShRegistersIndirect(SHRegisters, SHnum);
	m_DCB.setUcRegistersIndirect(UCRegisters, UCnum);
}
