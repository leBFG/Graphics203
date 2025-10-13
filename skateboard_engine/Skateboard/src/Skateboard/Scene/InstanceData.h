//#pragma once
//#include "Skateboard/Log.h"
//#include <string>
//#include <unordered_map>
//
//#define MESHID_ERROR UINT32_MAX
//#define INSTNACEID_ERROR UINT32_MAX
//
//namespace Skateboard
//{
//	typedef uint32_t MeshID;
//	typedef uint32_t InstanceID;
//
//
//	struct SceneInstanceData
//	{
//		const std::string& GetMeshTag(MeshID meshID) const
//		{
//			SKTBD_CORE_ASSERT(m_ID2NameTag.count(meshID), "No mesh was created with this ID. Invalid lookup.");
//			return m_ID2NameTag.at(meshID);
//		}
//
//		// This seems bad, but the compiler is likely to optimise the copies on Release. See https://www.scribd.com/document/316704010/Want-Speed-Pass-by-Value
//		std::string GenerateNameTag(MeshID meshID) const
//		{
//			std::string result;
//
//			SKTBD_CORE_ASSERT(m_ID2NameTag.count(meshID), "No mesh was created with this ID. Invalid lookup.");
//			result = m_ID2NameTag.at(meshID);
//
//			const uint32_t instanceCount = GetInstanceCount(meshID);
//			if (instanceCount)
//			{
//				result.append("(");
//				result.append(std::to_string(instanceCount));
//				result.append(")");
//			}
//			return result;
//		}
//
//		[[nodiscard("")]] uint32_t GetTotalInstanceCount() const
//		{
//			uint32_t result = 0u;
//			for (uint32_t i = 0u; i < GetMeshCount(); ++i)
//				result += v_InstanceCount[i];
//			return result;
//		}
//
//		[[nodiscard("")]] uint32_t GetInstanceCount(MeshID meshID) const
//		{
//			SKTBD_CORE_ASSERT(static_cast<uint32_t>(meshID < v_InstanceCount.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(m_ID2NameTag.count(meshID), "No mesh was created with this ID. Ensure you input a valid ID.");
//			return v_InstanceCount[meshID];
//		}
//
//		InstanceID AddInstance(MeshID meshID)
//		{
//			SKTBD_CORE_ASSERT(meshID < static_cast<uint32_t>(v_InstanceCount.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(meshID < static_cast<uint32_t>(v_vInstanceTrackers.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(m_ID2NameTag.count(meshID), "No mesh was created with this ID. Ensure you input a valid ID.");
//
//			// Returns a unique instance ID
//			SKTBD_CORE_TRACE("Added a {0} instance.", m_ID2NameTag.at(meshID).c_str());
//			v_InstanceCount[meshID]++;
//
//			// Check if any instance slot is available
//			std::vector<bool>& slots = v_vInstanceTrackers[meshID];
//			for(uint32_t i = 0u; i < static_cast<uint32_t>(slots.size()); ++i)
//				if (!slots[i])
//				{
//					slots[i] = true;
//					return i;
//				}
//
//			// If no slot was free, then push a new instance on the container
//			// The ID is zero-index based
//			const InstanceID instanceID = static_cast<InstanceID>(slots.size());
//			slots.emplace_back(true);
//			return instanceID;
//		}
//		void RemoveInstance(MeshID meshID, uint32_t instanceID)
//		{
//			SKTBD_CORE_ASSERT(meshID < static_cast<uint32_t>(v_InstanceCount.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(meshID < static_cast<uint32_t>(v_vInstanceTrackers.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(m_ID2NameTag.count(meshID), "No mesh was created with this ID. Ensure you input a valid ID.");
//			SKTBD_CORE_TRACE("Removed a {0} instance.", m_ID2NameTag.at(meshID).c_str());
//
//			// When removing an instance, we'll simply mark it as 'dead'
//			v_vInstanceTrackers[meshID][instanceID] = false;
//			v_InstanceCount[meshID]--;
//		}
//
//		uint32_t GetInstanceBufferIndex(MeshID meshID, uint32_t instanceID) const
//		{
//			SKTBD_CORE_ASSERT(meshID < static_cast<uint32_t>(v_InstanceCount.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(meshID < static_cast<uint32_t>(v_vInstanceTrackers.size()), "The input mesh ID does not exist.");
//			SKTBD_CORE_ASSERT(m_ID2NameTag.count(meshID), "No mesh was created with this ID. Ensure you input a valid ID.");
//
//			uint32_t returnVal = 0u;
//			for (uint32_t i = 0u; i < meshID; ++i)
//				returnVal += v_InstanceCount[i];
//
//			uint32_t freeSlotID = 0u;
//			for (uint32_t i = 0u; i < instanceID; ++i)	// This is rather ugly, but we don't want to consider 'dead' instances when indexing for the buffer
//				if (v_vInstanceTrackers[meshID][i])
//					freeSlotID++;
//			returnVal += freeSlotID;
//			return returnVal;
//		}
//
//		MeshID AddMesh(const char* nameTag)
//		{
//			if (m_NameTag2ID.count(nameTag))
//				return m_NameTag2ID.at(nameTag);
//
//			SKTBD_CORE_TRACE("Added mesh: {0}", nameTag);
//			const MeshID meshID = m_NextMeshID++;
//			m_ID2NameTag.insert({ meshID, std::string(nameTag) });
//			m_NameTag2ID.insert({ std::string(nameTag), meshID });
//			v_InstanceCount.push_back(0u);
//			v_vInstanceTrackers.push_back(std::vector<bool>());
//			return meshID;
//		}
//
//		MeshID GetMeshID(const char* nametag) const
//		{
//			if (!m_NameTag2ID.count(nametag))
//				return MESHID_ERROR;
//			return m_NameTag2ID.at(nametag);
//		}
//		bool IsMeshValid(MeshID meshID) const { return m_ID2NameTag.count(meshID); }
//		inline uint32_t GetMeshCount() const { return m_NextMeshID; }
//
//	private:
//		MeshID m_NextMeshID = 0u;
//		std::unordered_map<MeshID, std::string> m_ID2NameTag;
//		std::unordered_map<std::string, MeshID> m_NameTag2ID;
//		// The idea is that each mesh can have zero, one or many instances. We could just track the count in a game where the instances are not
//		// being created or deleted on the fly.
//		std::vector<uint32_t> v_InstanceCount;
//		// So for flexibility, we'll employ a system that tracks if and Instance is 'alive' with a bool for a corresponding uint32_t instance ID
//		// That way, when adding an instance we check if any was dead. If not, we simply emplace on the vector
//		std::vector<std::vector<bool>> v_vInstanceTrackers;
//
//	};
//}