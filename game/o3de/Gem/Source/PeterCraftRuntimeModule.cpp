
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "PeterCraftRuntimeSystemComponent.h"

#include <PeterCraftRuntime/PeterCraftRuntimeTypeIds.h>

namespace PeterCraftRuntime
{
    class PeterCraftRuntimeModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(PeterCraftRuntimeModule, PeterCraftRuntimeModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(PeterCraftRuntimeModule, AZ::SystemAllocator);

        PeterCraftRuntimeModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                PeterCraftRuntimeSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<PeterCraftRuntimeSystemComponent>(),
            };
        }
    };
}// namespace PeterCraftRuntime

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), PeterCraftRuntime::PeterCraftRuntimeModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_PeterCraftRuntime, PeterCraftRuntime::PeterCraftRuntimeModule)
#endif
