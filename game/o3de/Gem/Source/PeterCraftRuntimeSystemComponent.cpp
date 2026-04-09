
#include <AzCore/Serialization/SerializeContext.h>

#include "PeterCraftRuntimeSystemComponent.h"

#include <PeterCraftRuntime/PeterCraftRuntimeTypeIds.h>

namespace PeterCraftRuntime
{
    AZ_COMPONENT_IMPL(PeterCraftRuntimeSystemComponent, "PeterCraftRuntimeSystemComponent",
        PeterCraftRuntimeSystemComponentTypeId);

    void PeterCraftRuntimeSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<PeterCraftRuntimeSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void PeterCraftRuntimeSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("PeterCraftRuntimeService"));
    }

    void PeterCraftRuntimeSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("PeterCraftRuntimeService"));
    }

    void PeterCraftRuntimeSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void PeterCraftRuntimeSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    PeterCraftRuntimeSystemComponent::PeterCraftRuntimeSystemComponent()
    {
        if (PeterCraftRuntimeInterface::Get() == nullptr)
        {
            PeterCraftRuntimeInterface::Register(this);
        }
    }

    PeterCraftRuntimeSystemComponent::~PeterCraftRuntimeSystemComponent()
    {
        if (PeterCraftRuntimeInterface::Get() == this)
        {
            PeterCraftRuntimeInterface::Unregister(this);
        }
    }

    void PeterCraftRuntimeSystemComponent::Init()
    {
    }

    void PeterCraftRuntimeSystemComponent::Activate()
    {
        PeterCraftRuntimeRequestBus::Handler::BusConnect();
    }

    void PeterCraftRuntimeSystemComponent::Deactivate()
    {
        PeterCraftRuntimeRequestBus::Handler::BusDisconnect();
    }
}
