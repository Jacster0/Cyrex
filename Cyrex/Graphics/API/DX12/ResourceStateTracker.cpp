#include "ResourceStateTracker.h"
#include "Resource.h"
#include "CommandList.h"
#include <cassert>

std::mutex                                    Cyrex::ResourceStateTracker::ms_globalMutex;
bool                                          Cyrex::ResourceStateTracker::ms_isLocked = false;
Cyrex::ResourceStateTracker::ResourceStateMap Cyrex::ResourceStateTracker::ms_globalResourceState;

Cyrex::ResourceStateTracker::ResourceStateTracker()
{}

void Cyrex::ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier) {
    if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
        const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

        // First check if there is already a known "final" state for the given resource.
        // If there is, the resource has been used on the command list before and
        // already has a known state within the command list execution.
        const auto iter = m_finalResourceState.find(transitionBarrier.pResource);

        if (iter != m_finalResourceState.end()) {
            auto& resourceState = iter->second;

            // If the known final state of the resource is different...
            if (transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES 
                && !resourceState.SubresourceState.empty()) 
            {
                for (const auto& subresourceState : resourceState.SubresourceState) {
                    if (transitionBarrier.StateAfter != subresourceState.second) {
                        D3D12_RESOURCE_BARRIER newBarrier = barrier;
                        newBarrier.Transition.Subresource = subresourceState.first;
                        newBarrier.Transition.StateBefore = subresourceState.second;

                        m_resourceBarriers.push_back(newBarrier);
                    }
                }

            }
            else {
                auto finalState = resourceState.GetSubresourceState(transitionBarrier.Subresource);

                if (transitionBarrier.StateAfter != finalState) {
                    //Push a new transition barrier with the correct before state.
                    D3D12_RESOURCE_BARRIER newBarrier = barrier;
                    newBarrier.Transition.StateBefore = finalState;
                    m_resourceBarriers.push_back(newBarrier);
                }
            }
        }
        else {
            // In this case, the resource is being used on the command list for the first time.
            // Add a pending barrier. The pending barriers will be resolved
            // before the command list is executed on the command queue.
            m_pendingResourceBarriers.push_back(barrier);
        }

        m_finalResourceState[transitionBarrier.pResource].SetSubResource(transitionBarrier.Subresource, 
                                                                         transitionBarrier.StateAfter);
    }

    else {
        m_resourceBarriers.push_back(barrier);
    }
}

void Cyrex::ResourceStateTracker::TransitionResource(
    ID3D12Resource* resource, 
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t subResource)
{
    if (resource) {
        ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(
            resource, 
            D3D12_RESOURCE_STATE_COMMON, 
            stateAfter, 
            subResource));
    }
}

void Cyrex::ResourceStateTracker::TransitionResource(
    const Resource& resource, 
    D3D12_RESOURCE_STATES stateAfter, 
    uint32_t subResource)
{
    TransitionResource(resource.GetD3D12Resource().Get(), stateAfter, subResource);
}

void Cyrex::ResourceStateTracker::UAVBarrier(const Resource* resource) {
    ID3D12Resource* pResource = resource != nullptr ? resource->GetD3D12Resource().Get() : nullptr;

    ResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
}

void Cyrex::ResourceStateTracker::AliasBarrier(const Resource* resourceBefore, const Resource* resourceAfter) {
    ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->GetD3D12Resource().Get() : nullptr;
    ID3D12Resource* pResourceAfter  = resourceAfter  != nullptr ? resourceAfter->GetD3D12Resource().Get()  : nullptr;

    ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
}

uint32_t Cyrex::ResourceStateTracker::FlushPendingResourceBarriers(const std::shared_ptr<CommandList>& commandList) {
    assert(ms_isLocked);
    assert(commandList);

    // Resolve the pending resource barriers by checking the global state of the 
   // (sub)resources. Add barriers if the pending state and the global state do
   //  not match.
    ResourceBarriers resourceBarriers;
    // Reserve enough space (worst-case, all pending barriers).
    resourceBarriers.reserve(m_pendingResourceBarriers.size());

    for (auto pendingBarrier : m_pendingResourceBarriers) {
        // Only transition barriers should be pending.
        if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
            const auto pendingTransition = pendingBarrier.Transition;

            const auto& iter = ms_globalResourceState.find(pendingTransition.pResource);

            if (iter != ms_globalResourceState.end()) {
                // If all subresources are being transitioned, and there are multiple
                // subresources of the resource that are in a different state
                const auto& resourceState = iter->second;

                if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                    !resourceState.SubresourceState.empty()) {
                    // Transition all subresources
                    for (const auto& subresourceState : resourceState.SubresourceState) {
                        if (pendingTransition.StateAfter != subresourceState.second) {
                            D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
                            newBarrier.Transition.Subresource = subresourceState.first;
                            newBarrier.Transition.StateBefore = subresourceState.second;
                            resourceBarriers.push_back(newBarrier);
                        }
                    }
                }
                else {
                    // No (sub)resources need to be transitioned. Just add a single transition barrier (if needed).
                    const auto globalState = iter->second.GetSubresourceState(pendingTransition.Subresource);

                    if (pendingTransition.StateAfter != globalState) {
                        pendingBarrier.Transition.StateBefore = globalState;
                        resourceBarriers.push_back(pendingBarrier);
                    }
                }
            }
        }
    }

    const auto numBarriers = static_cast<uint32_t>(resourceBarriers.size());

    if (numBarriers > 0) { 
        auto d3d12CommandList = commandList->GetD3D12CommandList();
        d3d12CommandList->ResourceBarrier(numBarriers, resourceBarriers.data());
    }

    m_pendingResourceBarriers.clear();

    return numBarriers;
}

void Cyrex::ResourceStateTracker::FlushResourceBarriers(const std::shared_ptr<CommandList> commandList) {
    const auto numBarriers = static_cast<uint32_t>(m_resourceBarriers.size());

    if (numBarriers > 0) {
        auto d3d12CommandList = commandList->GetD3D12CommandList();
        d3d12CommandList->ResourceBarrier(numBarriers, m_resourceBarriers.data());
        m_resourceBarriers.clear();
    }
}

void Cyrex::ResourceStateTracker::CommitFinalResourceStates() {
    assert(ms_isLocked);

    for (const auto& resourceState : m_finalResourceState) {
        ms_globalResourceState[resourceState.first] = resourceState.second;
    }

    m_finalResourceState.clear();

}

void Cyrex::ResourceStateTracker::Reset() {
    m_pendingResourceBarriers.clear();
    m_resourceBarriers.clear();
    m_finalResourceState.clear();
}

void Cyrex::ResourceStateTracker::Lock() {
    ms_globalMutex.lock();
    ms_isLocked = true;
}

void Cyrex::ResourceStateTracker::Unlock() {
    ms_globalMutex.unlock();
    ms_isLocked = false;
}

void Cyrex::ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) {
    if (resource) {
        std::lock_guard<std::mutex> lock(ms_globalMutex);
        ms_globalResourceState[resource].SetSubResource(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
    }
}
