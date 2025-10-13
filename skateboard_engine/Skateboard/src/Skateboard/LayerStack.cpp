#include "sktbdpch.h"

#define SKTBD_LOG_COMPONENT "LAYER SYSTEM"
#include "Skateboard/Log.h"

#include "LayerStack.h"

namespace Skateboard
{
	LayerStack::LayerStack() :
		m_InsertIndex(0u)
	{
	}

	LayerStack::~LayerStack()
	{
		// Loop through all the layers, detach and erase for proper removal of all layers
		for (Layer*& layer : v_Layers)
		{
			if (layer)
			{
				layer->OnDetach();
				delete layer, layer = nullptr;
			}
		}
		v_Layers.clear();
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		// Sanity check
		SKTBD_LOG_ASSERT(layer, "Attempted to push a NULL layer");

		// Insert the layer at the first possible free slot and increment the insert index for further insertion
		v_Layers.emplace(v_Layers.begin() + m_InsertIndex++, layer);

		// This layer has been attached to the stack, call appropriate actions
		layer->OnAttach();
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		// Find the layer, detach, erase and decrement the insert index for further potential insertion
		std::vector<Layer*>::iterator it;
		for (it = v_Layers.begin(); it < v_Layers.begin() + m_InsertIndex; ++it)
			if (*it == layer)
			{
				layer->OnDetach();
				v_Layers.erase(it);
				--m_InsertIndex;
				return;
			}

		// If we make it here we have tried to remove a non-existing layer
		SKTBD_MSG_WARN("Attempted to remove a non-existing layer: {0}", layer->GetName());
		//SKTBD_MSG_WARN("Attempted to remove a non-existing layer: {0}");
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		// Sanity check
		SKTBD_LOG_ASSERT(overlay, "Attempted to push a NULL overlay")

		// Overlays are always inserted on the very top
		v_Layers.emplace_back(overlay);

		// This overlay has been attached to the stack, call appropriate actions
		overlay->OnAttach();
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		// Same as PopLayer() but without insertion index decrementation, as overlay insertions are not tracked
		std::vector<Layer*>::iterator it;
		for (it = v_Layers.begin(); it != v_Layers.end(); ++it)
			if (*it == overlay)
			{
				overlay->OnDetach();
				v_Layers.erase(it);
				return;
			}

		// If we make it here we have tried to remove a non-existing layer
		SKTBD_MSG_WARN("Attempted to remove a non-existing overlay: {0}", overlay->GetName());
	}
}