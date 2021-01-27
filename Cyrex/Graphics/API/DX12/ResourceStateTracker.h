#pragma once
#include "d3dx12.h"
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>

namespace Cyrex {
    class CommandList;
    class Resource;
    class ResourceStateTracker {
    public:
        ResourceStateTracker();
    public:
        void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

        void TransitionResource(
            ID3D12Resource* resource,
            D3D12_RESOURCE_STATES stateAfter,
            uint32_t subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        void TransitionResource(
            const Resource& resource,
            D3D12_RESOURCE_STATES stateAfter,
            uint32_t subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

        void UAVBarrier(const Resource* resource = nullptr);
        void AliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr);

        uint32_t FlushPendingResourceBarriers(const std::shared_ptr<CommandList>& commandList);
        void FlushResourceBarriers(const std::shared_ptr<CommandList> commandList);

        void CommitFinalResourceStates();
        void Reset();
    public:
        static void Lock();
        static void Unlock();
        static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
        static void RemoveGlobalResourceState(ID3D12Resource* resource, bool immediate = false);
        static void RemoveGarbageResources();
    private:
        using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

        ResourceBarriers m_pendingResourceBarriers;
        ResourceBarriers m_resourceBarriers;

        // Tracks the state of a particular resource and all of its subresources.
        struct ResourceState {
            explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
                :
                State(state)
            {}

            void SetSubResource(uint32_t subresource, D3D12_RESOURCE_STATES state) {
                if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
                    State = state;
                    SubresourceState.clear();
                }
                else {
                    SubresourceState[subresource] = state;
                }
            }

            D3D12_RESOURCE_STATES GetSubresourceState(uint32_t subresource) const {
                D3D12_RESOURCE_STATES state = State;
                const auto iter = SubresourceState.find(subresource);
                if (iter != SubresourceState.end()) {
                    state = iter->second;
                }
                return state;
            }

            D3D12_RESOURCE_STATES State;
            std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
        };

        using ResourceList = std::vector<ID3D12Resource*>;
        using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;


        // The final (last known state) of the resources within a command list.
        // The final resource state is committed to the global resource state when the 
        // command list is closed but before it is executed on the command queue.
        ResourceStateMap m_finalResourceState;

        // The global resource state array stores the state of a resource
        // between command list execution.
        static ResourceStateMap ms_globalResourceState;
        static ResourceList ms_garbageResources;

        static std::mutex ms_globalMutex;
        static bool ms_isLocked;
    };
}