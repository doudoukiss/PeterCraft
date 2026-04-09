
#pragma once

#include <AzCore/Component/Component.h>

#include <PeterCraftRuntime/PeterCraftRuntimeBus.h>

namespace PeterCraftRuntime
{
    class PeterCraftRuntimeSystemComponent
        : public AZ::Component
        , protected PeterCraftRuntimeRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(PeterCraftRuntimeSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        PeterCraftRuntimeSystemComponent();
        ~PeterCraftRuntimeSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // PeterCraftRuntimeRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
