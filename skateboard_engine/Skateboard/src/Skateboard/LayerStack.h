#pragma once
#include "Layer.h"
#include <vector>

namespace Skateboard
{
	using vlayer = std::vector<Layer*>;
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		void PushOverlay(Layer* overlay);	// Overlays will always be on top of all layers
		void PopOverlay(Layer* overlay);

		vlayer::iterator begin()				{ return v_Layers.begin(); }
		vlayer::iterator end()					{ return v_Layers.end(); }
		vlayer::reverse_iterator rbegin()		{ return v_Layers.rbegin(); }
		vlayer::reverse_iterator rend()			{ return v_Layers.rend(); }
		vlayer::const_iterator begin()			const { return v_Layers.begin(); }
		vlayer::const_iterator end()			const { return v_Layers.end(); }
		vlayer::const_reverse_iterator rbegin() const { return v_Layers.rbegin(); }
		vlayer::const_reverse_iterator rend()	const { return v_Layers.rend(); }

	private:
		vlayer v_Layers;
		uint32_t m_InsertIndex;
	};
}