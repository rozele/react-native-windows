// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <DynamicReader.h>
#include <DynamicWriter.h>
#include <Fabric/ReactNativeConfigProperties.h>
#include <Fabric/WinUI/ComponentView.h>
#include <Fabric/WinUI/FabricUIManagerModule.h>
#include <JSValueWriter.h>
#ifndef CORE_ABI
#include <Fabric/WinUI/Components/View/ViewComponentView.h>
#endif // CORE_ABI
#include <IReactContext.h>
#include <IReactRootView.h>
#ifndef CORE_ABI
#include <IXamlRootView.h>
#endif // CORE_ABI
#include <JSI/jsi.h>
#include <SchedulerSettings.h>
#include <UI.Xaml.Controls.h>
#include <react/components/rnwcore/ComponentDescriptors.h>
#include <react/renderer/componentregistry/ComponentDescriptorProviderRegistry.h>
#include <react/renderer/components/image/ImageComponentDescriptor.h>
#ifndef CORE_ABI
#include <react/renderer/components/slider/SliderComponentDescriptor.h>
#endif // CORE_ABI
#include <react/renderer/components/text/ParagraphComponentDescriptor.h>
#include <react/renderer/components/text/RawTextComponentDescriptor.h>
#include <react/renderer/components/text/TextComponentDescriptor.h>
#include <react/renderer/components/textinput/iostextinput/TextInputComponentDescriptor.h>
#include <react/renderer/components/view/ViewComponentDescriptor.h>
#include <react/renderer/scheduler/AsynchronousEventBeat.h>
#include <react/renderer/scheduler/Scheduler.h>
#include <react/renderer/scheduler/SchedulerToolbox.h>
#include <react/utils/ContextContainer.h>
#include <react/utils/RunLoopObserver.h>
#include <runtimeexecutor/ReactCommon/RuntimeExecutor.h>
#include <winrt/Windows.Graphics.Display.h>
#ifndef CORE_ABI
#include "Components/TextInput/WindowsTextInputComponentDescriptor.h"
#endif // CORE_ABI
#include "Unicode.h"

#pragma warning(push)
#pragma warning(disable : 4305)
#include <react/renderer/components/scrollview/ScrollViewComponentDescriptor.h>
#pragma warning(pop)

namespace Microsoft::ReactNative {

winrt::Microsoft::ReactNative::ReactPropertyId<
    winrt::Microsoft::ReactNative::ReactNonAbiValue<std::shared_ptr<FabricUIManager>>>
FabicUIManagerProperty() noexcept {
  winrt::Microsoft::ReactNative::ReactPropertyId<
      winrt::Microsoft::ReactNative::ReactNonAbiValue<std::shared_ptr<FabricUIManager>>>
      propId{L"ReactNative.Fabric", L"UIManager"};
  return propId;
}

/*static*/ std::shared_ptr<FabricUIManager> FabricUIManager::FromProperties(
    const winrt::Microsoft::ReactNative::ReactPropertyBag &props) {
  return props.Get(FabicUIManagerProperty()).Value();
}

/*static*/ facebook::react::SurfaceId SurfaceIdForView(IComponentView const *view) {
  auto element = static_cast<BaseComponentView const *>(view)->Element();
  do {
    const auto rootView = element.try_as<winrt::Microsoft::ReactNative::ReactRootView>();
    if (rootView) {
      return static_cast<facebook::react::SurfaceId>(rootView.GetTag());
    }
    element = element.Parent().try_as<xaml::FrameworkElement>();

  } while (element);

  return -1;
}

FabricUIManager::FabricUIManager() {}

FabricUIManager::~FabricUIManager() {}

class PlatformRunLoopObserver final : public facebook::react::RunLoopObserver {
 public:
  PlatformRunLoopObserver(
      facebook::react::RunLoopObserver::Activity activities,
      facebook::react::RunLoopObserver::WeakOwner const &owner,
      winrt::Microsoft::ReactNative::ReactContext const &context)
      : facebook::react::RunLoopObserver(activities, owner), m_context{context} {}

  bool isOnRunLoopThread() const noexcept override {
    return m_context.UIDispatcher().HasThreadAccess();
  }

 protected:
  void startObserving() const noexcept override {
    if (isOnRunLoopThread()) {
      subscribeRendering();
    } else {
      m_context.UIDispatcher().Post([this] { subscribeRendering(); });
    }
  }

  void stopObserving() const noexcept override {
    if (isOnRunLoopThread()) {
      unsubscribeRendering();
    } else {
      m_context.UIDispatcher().Post([this] { unsubscribeRendering(); });
    }
  }

 private:
  void subscribeRendering() const noexcept {
    m_rendering = xaml::Media::CompositionTarget::Rendering(
        winrt::auto_revoke, [this](const winrt::IInspectable &, const winrt::IInspectable & /*args*/) {
          auto owner = getOwner().lock();
          if (!owner) {
            return;
          }

          activityDidChange(facebook::react::RunLoopObserver::Activity::BeforeWaiting);
        });
  }

  void unsubscribeRendering() const noexcept {
    m_rendering.revoke();
  }

  winrt::Microsoft::ReactNative::ReactContext m_context;
  mutable xaml::Media::CompositionTarget::Rendering_revoker m_rendering;
};

std::shared_ptr<facebook::react::ComponentDescriptorProviderRegistry const> sharedProviderRegistry() {
  static auto providerRegistry = []() -> std::shared_ptr<facebook::react::ComponentDescriptorProviderRegistry> {
    auto providerRegistry = std::make_shared<facebook::react::ComponentDescriptorProviderRegistry>();
    providerRegistry->add(facebook::react::concreteComponentDescriptorProvider<
                          facebook::react::ActivityIndicatorViewComponentDescriptor>());
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::ImageComponentDescriptor>());
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::ParagraphComponentDescriptor>());
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::RawTextComponentDescriptor>());
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::ScrollViewComponentDescriptor>());
#ifndef CORE_ABI
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::SliderComponentDescriptor>());
#endif // CORE_ABI
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::SwitchComponentDescriptor>());
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::TextComponentDescriptor>());
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::ViewComponentDescriptor>());
#ifndef CORE_ABI
    providerRegistry->add(
        facebook::react::concreteComponentDescriptorProvider<facebook::react::WindowsTextInputComponentDescriptor>());
#endif // CORE_ABI
    return providerRegistry;
  }();

  return providerRegistry;
}

void FabricUIManager::installFabricUIManager() noexcept {
  std::shared_ptr<const facebook::react::ReactNativeConfig> config =
      std::make_shared<const ReactNativeConfigProperties>(m_context);

  std::lock_guard<std::mutex> schedulerLock(m_schedulerMutex);

  facebook::react::ContextContainer::Shared contextContainer = std::make_shared<facebook::react::ContextContainer>();

  // This allows access to our ReactContext from the contextContainer thats passed around the fabric codebase
  contextContainer->insert("MSRN.ReactContext", m_context);

  auto runtimeExecutor = SchedulerSettings::GetRuntimeExecutor(m_context.Properties());

  // TODO: T31905686 Create synchronous Event Beat
  facebook::react::EventBeat::Factory synchronousBeatFactory =
      [/*eventBeatManager,*/ runtimeExecutor,
       context = m_context](facebook::react::EventBeat::SharedOwnerBox const &ownerBox) {
        auto runLoopObserver = std::make_unique<PlatformRunLoopObserver>(
            facebook::react::RunLoopObserver::Activity::BeforeWaiting, ownerBox->owner, context);
        return std::make_unique<facebook::react::AsynchronousEventBeat>(std::move(runLoopObserver), runtimeExecutor);
      };

  facebook::react::EventBeat::Factory asynchronousBeatFactory =
      [/*eventBeatManager,*/ runtimeExecutor,
       context = m_context](facebook::react::EventBeat::SharedOwnerBox const &ownerBox) {
        auto runLoopObserver = std::make_unique<PlatformRunLoopObserver>(
            facebook::react::RunLoopObserver::Activity::BeforeWaiting, ownerBox->owner, context);
        return std::make_unique<facebook::react::AsynchronousEventBeat>(std::move(runLoopObserver), runtimeExecutor);
      };

  contextContainer->insert("ReactNativeConfig", config);

  auto toolbox = facebook::react::SchedulerToolbox{};
  toolbox.contextContainer = contextContainer;
  toolbox.componentRegistryFactory = [](facebook::react::EventDispatcher::Weak const &eventDispatcher,
                                        facebook::react::ContextContainer::Shared const &contextContainer)
      -> facebook::react::ComponentDescriptorRegistry::Shared {
    auto registry = sharedProviderRegistry()->createComponentDescriptorRegistry({eventDispatcher, contextContainer});
    // Enabling the fallback component will require us to run the view component codegen to generate
    // UnimplementedNativeViewComponentDescriptor
    /*
    auto mutableRegistry = std::const_pointer_cast<facebook::react::ComponentDescriptorRegistry>(registry);
    mutableRegistry->setFallbackComponentDescriptor(
        std::make_shared<facebook::react::UnimplementedNativeViewComponentDescriptor>(
            facebook::react::ComponentDescriptorParameters{
                eventDispatcher, contextContainer, nullptr}));
    */
    return registry;
  };
  toolbox.runtimeExecutor = runtimeExecutor;
  toolbox.synchronousEventBeatFactory = synchronousBeatFactory;
  toolbox.asynchronousEventBeatFactory = asynchronousBeatFactory;
  toolbox.backgroundExecutor = [context = m_context,
                                dispatcher = Mso::DispatchQueue::MakeLooperQueue()](std::function<void()> &&callback) {
    if (context.UIDispatcher().HasThreadAccess()) {
      callback();
      return;
    }

    dispatcher.Post(std::move(callback));
  };

  m_scheduler = std::make_shared<facebook::react::Scheduler>(
      toolbox, (/*animationDriver_ ? animationDriver_.get() :*/ nullptr), this);
  m_surfaceManager = std::make_shared<facebook::react::SurfaceManager>(*m_scheduler);
}

const ComponentViewRegistry &FabricUIManager::GetViewRegistry() const noexcept {
  return m_registry;
}

void FabricUIManager::startSurface(
    facebook::react::IReactRootView *rootview,
    facebook::react::SurfaceId surfaceId,
    const std::string &moduleName,
    const folly::dynamic &initialProps) noexcept {
#ifndef CORE_ABI
  auto xamlRootView = static_cast<IXamlRootView *>(rootview);
  auto rootFE = xamlRootView->GetXamlView().as<xaml::FrameworkElement>();

  m_surfaceRegistry.insert({surfaceId, xamlRootView->GetXamlView()});
#endif // CORE_ABI

  m_context.UIDispatcher().Post([self = shared_from_this(), surfaceId]() {
    self->m_registry.dequeueComponentViewWithComponentHandle(facebook::react::RootShadowNode::Handle(), surfaceId);
  });

  facebook::react::LayoutContext context;

  // TODO: This call wont work with winUI
  context.pointScaleFactor = static_cast<facebook::react::Float>(
      winrt::Windows::Graphics::Display::DisplayInformation::GetForCurrentView().RawPixelsPerViewPixel());

  facebook::react::LayoutConstraints constraints;
#ifndef CORE_ABI
  constraints.minimumSize.height = static_cast<facebook::react::Float>(rootFE.ActualHeight());
  constraints.minimumSize.width = static_cast<facebook::react::Float>(rootFE.ActualWidth());
  constraints.maximumSize.height = static_cast<facebook::react::Float>(rootFE.ActualHeight());
  constraints.maximumSize.width = static_cast<facebook::react::Float>(rootFE.ActualWidth());
  constraints.layoutDirection = rootFE.FlowDirection() == xaml::FlowDirection::LeftToRight
      ? facebook::react::LayoutDirection::LeftToRight
      : facebook::react::LayoutDirection::RightToLeft;
#endif // CORE_ABI

  m_surfaceManager->startSurface(
      surfaceId,
      moduleName,
      initialProps,
      constraints, // layout constraints
      context // layout context
  );
}

void FabricUIManager::stopSurface(facebook::react::SurfaceId surfaceId) noexcept {
  m_surfaceManager->stopSurface(surfaceId);
}

facebook::react::Size FabricUIManager::measureSurface(
    facebook::react::SurfaceId surfaceId,
    const facebook::react::LayoutConstraints &layoutConstraints,
    const facebook::react::LayoutContext &layoutContext) const noexcept {
  return m_surfaceManager->measureSurface(surfaceId, layoutConstraints, layoutContext);
}

bool FabricUIManager::synchronouslyUpdateViewOnUIThread(
    facebook::react::Tag tag,
    winrt::Microsoft::ReactNative::JSValueObject const &props) {
  // Check if the component view exists
  if (!m_scheduler) {
    return false;
  }

  // Check if the component view exists
  auto componentView = m_registry.findComponentViewWithTag(tag);
  if (!componentView) {
    return false;
  }

  // Check if the component descriptor exists
  auto viewDescriptor = m_registry.componentViewDescriptorWithTag(tag);
  auto componentDescriptor =
      m_scheduler->findComponentDescriptorByHandle_DO_NOT_USE_THIS_IS_BROKEN(viewDescriptor.componentHandle);
  if (!componentDescriptor) {
    return false;
  }

  // Get the current props for the view
  const auto oldProps = componentView->props();

  // Clone the current props with the dynamic props from Animated
  const auto context =
      facebook::react::PropsParserContext{SurfaceIdForView(componentView.get()), *m_scheduler->getContextContainer()};
  const auto dynamicProps =
      winrt::Microsoft::ReactNative::DynamicWriter::ToDynamic(winrt::Microsoft::ReactNative::MakeJSValueWriter(props));
  const auto newProps = componentDescriptor->cloneProps(context, oldProps, facebook::react::RawProps{dynamicProps});

  // Stash the current prop keys managed by Animated
  std::unordered_set<std::string> propKeys = componentView->propKeysManagedByAnimated_DO_NOT_USE_THIS_IS_BROKEN;

  // Unset the keys managed by Animated so the Animated props can be updated
  componentView->propKeysManagedByAnimated_DO_NOT_USE_THIS_IS_BROKEN = {};
  componentView->updateProps(newProps, oldProps);

  // Update the prop keys managed by Animated
  for (const auto &pair : props) {
    propKeys.insert(pair.first);
  }
  componentView->propKeysManagedByAnimated_DO_NOT_USE_THIS_IS_BROKEN = propKeys;

  // Finalize the prop updates
  componentView->finalizeUpdates(RNComponentViewUpdateMask::Props);

  return true;
}

void FabricUIManager::constraintSurfaceLayout(
    facebook::react::SurfaceId surfaceId,
    const facebook::react::LayoutConstraints &layoutConstraints,
    const facebook::react::LayoutContext &layoutContext) const noexcept {
  m_surfaceManager->constraintSurfaceLayout(surfaceId, layoutConstraints, layoutContext);
}

void FabricUIManager::didMountComponentsWithRootTag(facebook::react::SurfaceId surfaceId) noexcept {
  auto rootComponentViewDescriptor = m_registry.componentViewDescriptorWithTag(surfaceId);
#ifndef CORE_ABI
  auto children = m_surfaceRegistry.at(surfaceId).as<xaml::Controls::Panel>().Children();

  uint32_t index;
  if (!children.IndexOf(static_cast<ViewComponentView &>(*rootComponentViewDescriptor.view).Element(), index)) {
    children.Append(static_cast<ViewComponentView &>(*rootComponentViewDescriptor.view).Element());
  }
#endif // CORE_ABI
}

struct RemoveDeleteMetadata {
  facebook::react::Tag tag;
  facebook::react::Tag parentTag;
  int index;
  bool shouldRemove;
  bool shouldDelete;
};

void FabricUIManager::RCTPerformMountInstructions(
    facebook::react::ShadowViewMutationList const &mutations,
    // facebook::react::RCTComponentViewRegistry* registry,
    // facebook::react::RCTMountingTransactionObserverCoordinator& observerCoordinator,
    facebook::react::SurfaceId surfaceId) {
  for (auto const &mutation : mutations) {
    switch (mutation.type) {
      case facebook::react::ShadowViewMutation::Create: {
        auto &newChildShadowView = mutation.newChildShadowView;
        auto &newChildViewDescriptor = m_registry.dequeueComponentViewWithComponentHandle(
            newChildShadowView.componentHandle, newChildShadowView.tag);
        // observerCoordinator.registerViewComponentDescriptor(newChildViewDescriptor, surfaceId);
        break;
      }

      case facebook::react::ShadowViewMutation::Delete: {
        auto &oldChildShadowView = mutation.oldChildShadowView;
        auto &oldChildViewDescriptor = m_registry.componentViewDescriptorWithTag(oldChildShadowView.tag);
        // observerCoordinator.unregisterViewComponentDescriptor(oldChildViewDescriptor, surfaceId);
        m_registry.enqueueComponentViewWithComponentHandle(
            oldChildShadowView.componentHandle, oldChildShadowView.tag, oldChildViewDescriptor);
        break;
      }

      case facebook::react::ShadowViewMutation::Insert: {
        auto &oldChildShadowView = mutation.oldChildShadowView;
        auto &newChildShadowView = mutation.newChildShadowView;
        auto &parentShadowView = mutation.parentShadowView;
        auto &newChildViewDescriptor = m_registry.componentViewDescriptorWithTag(newChildShadowView.tag);
        auto &parentViewDescriptor = m_registry.componentViewDescriptorWithTag(parentShadowView.tag);

        auto &newChildComponentView = newChildViewDescriptor.view;

        newChildComponentView->updateProps(newChildShadowView.props, oldChildShadowView.props);
        newChildComponentView->updateEventEmitter(newChildShadowView.eventEmitter);
        newChildComponentView->updateState(newChildShadowView.state, oldChildShadowView.state);
        newChildComponentView->updateLayoutMetrics(newChildShadowView.layoutMetrics, oldChildShadowView.layoutMetrics);
        newChildComponentView->finalizeUpdates(RNComponentViewUpdateMask::All);

        parentViewDescriptor.view->mountChildComponentView(*newChildComponentView, mutation.index);
        break;
      }

      case facebook::react::ShadowViewMutation::Remove: {
        auto &oldChildShadowView = mutation.oldChildShadowView;
        auto &parentShadowView = mutation.parentShadowView;
        auto &oldChildViewDescriptor = m_registry.componentViewDescriptorWithTag(oldChildShadowView.tag);
        auto &parentViewDescriptor = m_registry.componentViewDescriptorWithTag(parentShadowView.tag);
        parentViewDescriptor.view->unmountChildComponentView(*oldChildViewDescriptor.view, mutation.index);
        break;
      }

      case facebook::react::ShadowViewMutation::Update: {
        auto &oldChildShadowView = mutation.oldChildShadowView;
        auto &newChildShadowView = mutation.newChildShadowView;
        auto &newChildViewDescriptor = m_registry.componentViewDescriptorWithTag(newChildShadowView.tag);
        auto &newChildComponentView = newChildViewDescriptor.view;

        auto mask = RNComponentViewUpdateMask{};

        if (oldChildShadowView.props != newChildShadowView.props) {
          newChildComponentView->updateProps(newChildShadowView.props, oldChildShadowView.props);
          mask |= RNComponentViewUpdateMask::Props;
        }

        if (oldChildShadowView.eventEmitter != newChildShadowView.eventEmitter) {
          newChildComponentView->updateEventEmitter(newChildShadowView.eventEmitter);
          mask |= RNComponentViewUpdateMask::EventEmitter;
        }

        if (oldChildShadowView.state != newChildShadowView.state) {
          newChildComponentView->updateState(newChildShadowView.state, oldChildShadowView.state);
          mask |= RNComponentViewUpdateMask::State;
        }

        if (oldChildShadowView.layoutMetrics != newChildShadowView.layoutMetrics) {
          newChildComponentView->updateLayoutMetrics(
              newChildShadowView.layoutMetrics, oldChildShadowView.layoutMetrics);
          mask |= RNComponentViewUpdateMask::LayoutMetrics;
        }

        if (mask != RNComponentViewUpdateMask::None) {
          newChildComponentView->finalizeUpdates(mask);
        }

        break;
      }
    }
  }
}

void FabricUIManager::performTransaction(facebook::react::MountingCoordinator::Shared const &mountingCoordinator) {
  auto surfaceId = mountingCoordinator->getSurfaceId();

  mountingCoordinator->getTelemetryController().pullTransaction(
      [&](facebook::react::MountingTransaction const &transaction,
          facebook::react::SurfaceTelemetry const &surfaceTelemetry) {
        //[self.delegate mountingManager:self willMountComponentsWithRootTag:surfaceId];
        // _observerCoordinator.notifyObserversMountingTransactionWillMount(transaction, surfaceTelemetry);
      },
      [&](facebook::react::MountingTransaction const &transaction,
          facebook::react::SurfaceTelemetry const &surfaceTelemetry) {
        RCTPerformMountInstructions(
            transaction.getMutations(), /* _componentViewRegistry, _observerCoordinator,*/ surfaceId);
      },
      [&](facebook::react::MountingTransaction const &transaction,
          facebook::react::SurfaceTelemetry const &surfaceTelemetry) {
        //_observerCoordinator.notifyObserversMountingTransactionDidMount(transaction, surfaceTelemetry);
        didMountComponentsWithRootTag(surfaceId);
        //[self.delegate mountingManager:self didMountComponentsWithRootTag:surfaceId];
      });
}

void FabricUIManager::initiateTransaction(facebook::react::MountingCoordinator::Shared const &mountingCoordinator) {
  if (m_transactionInFlight) {
    m_followUpTransactionRequired = true;
    return;
  }

  do {
    m_followUpTransactionRequired = false;
    m_transactionInFlight = true;
    performTransaction(mountingCoordinator);
    m_transactionInFlight = false;
  } while (m_followUpTransactionRequired);
}

void FabricUIManager::schedulerDidFinishTransaction(
    facebook::react::MountingCoordinator::Shared const &mountingCoordinator) {
  // Should cache this locally

  if (m_context.UIDispatcher().HasThreadAccess()) {
    initiateTransaction(mountingCoordinator);
  } else {
    m_context.UIDispatcher().Post(
        [mountingCoordinator, self = shared_from_this()]() { self->initiateTransaction(mountingCoordinator); });
  }
}

void FabricUIManager::schedulerDidRequestPreliminaryViewAllocation(
    facebook::react::SurfaceId surfaceId,
    const facebook::react::ShadowNode &shadowView) {
  /*
    if (m_context.UIDispatcher().HasThreadAccess()) {
    m_registry.dequeueComponentViewWithComponentHandle(shadowView.componentHandle, surfaceId);
  } else {
      m_context.UIDispatcher().Post(
        [componentHandle = shadowView.componentHandle, surfaceId, self = shared_from_this()]() {
          self->m_registry.dequeueComponentViewWithComponentHandle(componentHandle, surfaceId);
          });
  }
  */

  // Not needed on iOS.. so maybe not needed in windows?
  /*
  bool isLayoutableShadowNode = shadowView.layoutMetrics != facebook::react::EmptyLayoutMetrics;

  shadowView.props

  local_ref<ReadableMap::javaobject> props =
      castReadableMap(ReadableNativeMap::newObjectCxxArgs(shadowView.props->rawProps));
  auto component = getPlatformComponentName(shadowView);

  preallocateView(
      localJavaUIManager,
      surfaceId,
      shadowView.tag,
      component.get(),
      props.get(),
      (javaStateWrapper != nullptr ? javaStateWrapper.get() : nullptr),
      isLayoutableShadowNode);

  assert(false);

  */
}

void FabricUIManager::schedulerDidDispatchCommand(
    facebook::react::ShadowView const &shadowView,
    std::string const &commandName,
    folly::dynamic const &arg) {
  if (m_context.UIDispatcher().HasThreadAccess()) {
    auto descriptor = m_registry.componentViewDescriptorWithTag(shadowView.tag);
    descriptor.view->handleCommand(commandName, arg);
  } else {
    m_context.UIDispatcher().Post(
        [wkThis = weak_from_this(), commandName, tag = shadowView.tag, args = folly::dynamic(arg)]() {
          if (auto pThis = wkThis.lock()) {
            auto view = pThis->m_registry.findComponentViewWithTag(tag);
            if (view) {
              view->handleCommand(commandName, args);
            }
          }
        });
  }
}

void FabricUIManager::schedulerDidSetIsJSResponder(
    facebook::react::ShadowView const &shadowView,
    bool isJSResponder,
    bool blockNativeResponder) {}

void FabricUIManager::schedulerDidSendAccessibilityEvent(
    const facebook::react::ShadowView &shadowView,
    std::string const &eventType) {
  assert(false);
}

void FabricUIManager::Initialize(winrt::Microsoft::ReactNative::ReactContext const &reactContext) noexcept {
  m_context = reactContext;

  m_registry.Initialize(reactContext);

  m_context.Properties().Set(FabicUIManagerProperty(), shared_from_this());

  /*
  EventBeatManager eventBeatManager = new EventBeatManager(mReactApplicationContext);
  UIManagerModule nativeModule =
    Assertions.assertNotNull(mReactApplicationContext.getNativeModule(UIManagerModule.class));
  EventDispatcher eventDispatcher = nativeModule.getEventDispatcher();
  FabricUIManager uiManager =
    new FabricUIManager(
      mReactApplicationContext,
      nativeModule.getViewManagerRegistry_DO_NOT_USE(),
      eventDispatcher,
      eventBeatManager);

  Binding binding = new Binding();
  // TODO T31905686: remove this call
  loadClasses();
  MessageQueueThread jsMessageQueueThread =
    mReactApplicationContext
      .getCatalystInstance()
      .getReactQueueConfiguration()
      .getJSQueueThread();
  */

  // binding.register( // This is register in java, which calls into Binding::installFabricUIManager
  installFabricUIManager(
      /*
        mReactApplicationContext.getCatalystInstance().getRuntimeExecutor(),
        uiManager,
        eventBeatManager,
        jsMessageQueueThread,
        mComponentFactory,
        mConfig
        */);
  /*
  Systrace.endSection(Systrace.TRACE_TAG_REACT_JAVA_BRIDGE);
  return uiManager;

*/
}

} // namespace Microsoft::ReactNative
