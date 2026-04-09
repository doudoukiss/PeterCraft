
#pragma once

#include <PeterCraftRuntime/PeterCraftRuntimeTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace PeterCraftRuntime
{
    class PeterCraftRuntimeRequests
    {
    public:
        AZ_RTTI(PeterCraftRuntimeRequests, PeterCraftRuntimeRequestsTypeId);
        virtual ~PeterCraftRuntimeRequests() = default;
        // Put your public methods here
    };

    class PeterCraftRuntimeBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using PeterCraftRuntimeRequestBus = AZ::EBus<PeterCraftRuntimeRequests, PeterCraftRuntimeBusTraits>;
    using PeterCraftRuntimeInterface = AZ::Interface<PeterCraftRuntimeRequests>;

} // namespace PeterCraftRuntime
