// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "DependentAdditionAnimatedNode.h"
#include "DependentDiffClampAnimatedNode.h"
#include "DependentDivisionAnimatedNode.h"
#include "DependentInterpolationAnimatedNode.h"
#include "DependentModulusAnimatedNode.h"
#include "DependentMultiplicationAnimatedNode.h"
#include "DependentNativeAnimatedNodeManager.h"
#include "DependentStyleAnimatedNode.h"
#include "DependentSubtractionAnimatedNode.h"
#include "DependentTrackingAnimatedNode.h"

#include "DependentDecayAnimationDriver.h"
#include "DependentFrameBasedAnimationDriver.h"
#include "DependentSpringAnimationDriver.h"

#include <Modules/NativeUIManager.h>
#include <Modules/PaperUIManagerModule.h>
#include <UI.Xaml.Media.h>
#include <Windows.Foundation.h>
#include <queue>

namespace Microsoft::ReactNative {
void DependentNativeAnimatedNodeManager::CreateAnimatedNode(
    int64_t tag,
    const folly::dynamic &config,
    const Mso::CntPtr<Mso::React::IReactContext> &context,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager) {
  if (m_transformNodes.count(tag) > 0 || m_propsNodes.count(tag) > 0 || m_styleNodes.count(tag) > 0 ||
      m_valueNodes.count(tag) > 0) {
    throw std::invalid_argument("AnimatedNode with tag " + std::to_string(tag) + " already exists.");
    return;
  }

  const auto type = config.find("type").dereference().second.getString();
  if (type == "style") {
    m_styleNodes.emplace(tag, std::make_unique<DependentStyleAnimatedNode>(tag, config, manager));
  } else if (type == "value") {
    m_valueNodes.emplace(tag, std::make_unique<DependentValueAnimatedNode>(tag, config, manager));
  } else if (type == "props") {
    m_propsNodes.emplace(tag, std::make_unique<DependentPropsAnimatedNode>(tag, config, context, manager));
  } else if (type == "interpolation") {
    m_valueNodes.emplace(tag, std::make_unique<DependentInterpolationAnimatedNode>(tag, config, manager));
  } else if (type == "addition") {
    m_valueNodes.emplace(tag, std::make_unique<DependentAdditionAnimatedNode>(tag, config, manager));
  } else if (type == "subtraction") {
    m_valueNodes.emplace(tag, std::make_unique<DependentSubtractionAnimatedNode>(tag, config, manager));
  } else if (type == "division") {
    m_valueNodes.emplace(tag, std::make_unique<DependentDivisionAnimatedNode>(tag, config, manager));
  } else if (type == "multiplication") {
    m_valueNodes.emplace(tag, std::make_unique<DependentMultiplicationAnimatedNode>(tag, config, manager));
  } else if (type == "modulus") {
    m_valueNodes.emplace(tag, std::make_unique<DependentModulusAnimatedNode>(tag, config, manager));
  } else if (type == "diffclamp") {
    m_valueNodes.emplace(tag, std::make_unique<DependentDiffClampAnimatedNode>(tag, config, manager));
  } else if (type == "transform") {
    m_transformNodes.emplace(tag, std::make_unique<DependentTransformAnimatedNode>(tag, config, manager));
  } else if (type == "tracking") {
    m_trackingNodes.emplace(tag, std::make_unique<DependentTrackingAnimatedNode>(tag, config, manager));
  }
}

void DependentNativeAnimatedNodeManager::GetValue(int64_t animatedNodeTag, const Callback &saveValueCallback) {
  if (const auto valueNode = m_valueNodes.at(animatedNodeTag).get()) {
    saveValueCallback(std::vector<folly::dynamic>{folly::dynamic(valueNode->Value())});
  }
}

void DependentNativeAnimatedNodeManager::ConnectAnimatedNodeToView(int64_t propsNodeTag, int64_t viewTag) {
  m_propsNodes.at(propsNodeTag)->ConnectToView(viewTag);
  m_updatedNodes.insert(propsNodeTag);
  EnsureRendering();
}

void DependentNativeAnimatedNodeManager::DisconnectAnimatedNodeToView(int64_t propsNodeTag, int64_t viewTag) {
  m_propsNodes.at(propsNodeTag)->DisconnectFromView(viewTag);
}

void DependentNativeAnimatedNodeManager::ConnectAnimatedNode(int64_t parentNodeTag, int64_t childNodeTag) {
  if (const auto parentNode = GetAnimatedNode(parentNodeTag)) {
    parentNode->AddChild(childNodeTag);
    m_updatedNodes.insert(childNodeTag);
    EnsureRendering();
  }
}

void DependentNativeAnimatedNodeManager::DisconnectAnimatedNode(int64_t parentNodeTag, int64_t childNodeTag) {
  if (const auto parentNode = GetAnimatedNode(parentNodeTag)) {
    parentNode->RemoveChild(childNodeTag);
    m_updatedNodes.insert(childNodeTag);
    EnsureRendering();
  }
}

void DependentNativeAnimatedNodeManager::StopAnimation(int64_t animationId) {
  DoCallback(animationId, false);
  m_activeAnimations.erase(animationId);
}

void DependentNativeAnimatedNodeManager::StartAnimatingNode(
    int64_t animationId,
    int64_t animatedNodeTag,
    const folly::dynamic &animationConfig,
    const Callback &endCallback,
    const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager) {
  const auto type = animationConfig.find("type").dereference().second.getString();
  if (type == "decay") {
    m_activeAnimations.emplace(
        animationId,
        std::make_shared<DependentDecayAnimationDriver>(
            animationId, animatedNodeTag, animationConfig, endCallback, manager));
  } else if (type == "frames") {
    m_activeAnimations.emplace(
        animationId,
        std::make_shared<DependentFrameBasedAnimationDriver>(
            animationId, animatedNodeTag, animationConfig, endCallback, manager));
  } else if (type == "spring") {
    m_activeAnimations.emplace(
        animationId,
        std::make_shared<DependentSpringAnimationDriver>(
            animationId, animatedNodeTag, animationConfig, endCallback, manager));
  }

  EnsureRendering();
}

void DependentNativeAnimatedNodeManager::DropAnimatedNode(int64_t tag) {
  m_valueNodes.erase(tag);
  m_propsNodes.erase(tag);
  m_styleNodes.erase(tag);
  m_transformNodes.erase(tag);
  m_updatedNodes.erase(tag);
}

void DependentNativeAnimatedNodeManager::SetAnimatedNodeValue(int64_t tag, double value) {
  if (const auto valueNode = m_valueNodes.at(tag).get()) {
    valueNode->RawValue(value);
    StopAnimationsForNode(tag);
    m_updatedNodes.insert(tag);
    EnsureRendering();
  }
}

void DependentNativeAnimatedNodeManager::SetAnimatedNodeOffset(int64_t tag, double offset) {
  if (const auto valueNode = m_valueNodes.at(tag).get()) {
    valueNode->Offset(offset);
    m_updatedNodes.insert(tag);
    EnsureRendering();
  }
}

void DependentNativeAnimatedNodeManager::FlattenAnimatedNodeOffset(int64_t tag) {
  if (const auto valueNode = m_valueNodes.at(tag).get()) {
    valueNode->FlattenOffset();
  }
}

void DependentNativeAnimatedNodeManager::ExtractAnimatedNodeOffset(int64_t tag) {
  if (const auto valueNode = m_valueNodes.at(tag).get()) {
    valueNode->ExtractOffset();
  }
}

DependentAnimatedNode *DependentNativeAnimatedNodeManager::GetAnimatedNode(int64_t tag) {
  if (m_valueNodes.count(tag)) {
    return m_valueNodes.at(tag).get();
  }
  if (m_styleNodes.count(tag)) {
    return m_styleNodes.at(tag).get();
  }
  if (m_propsNodes.count(tag)) {
    return m_propsNodes.at(tag).get();
  }
  if (m_transformNodes.count(tag)) {
    return m_transformNodes.at(tag).get();
  }
  if (m_trackingNodes.count(tag)) {
    return m_trackingNodes.at(tag).get();
  }
  return static_cast<DependentAnimatedNode *>(nullptr);
}

DependentValueAnimatedNode *DependentNativeAnimatedNodeManager::GetValueAnimatedNode(int64_t tag) {
  if (m_valueNodes.count(tag)) {
    return m_valueNodes.at(tag).get();
  }
  return static_cast<DependentValueAnimatedNode *>(nullptr);
}

DependentPropsAnimatedNode *DependentNativeAnimatedNodeManager::GetPropsAnimatedNode(int64_t tag) {
  if (m_propsNodes.count(tag)) {
    return m_propsNodes.at(tag).get();
  }
  return static_cast<DependentPropsAnimatedNode *>(nullptr);
}

DependentStyleAnimatedNode *DependentNativeAnimatedNodeManager::GetStyleAnimatedNode(int64_t tag) {
  if (m_styleNodes.count(tag)) {
    return m_styleNodes.at(tag).get();
  }
  return static_cast<DependentStyleAnimatedNode *>(nullptr);
}

DependentTransformAnimatedNode *DependentNativeAnimatedNodeManager::GetTransformAnimatedNode(int64_t tag) {
  if (m_transformNodes.count(tag)) {
    return m_transformNodes.at(tag).get();
  }
  return static_cast<DependentTransformAnimatedNode *>(nullptr);
}

DependentTrackingAnimatedNode *DependentNativeAnimatedNodeManager::GetTrackingAnimatedNode(int64_t tag) {
  if (m_trackingNodes.count(tag)) {
    return m_trackingNodes.at(tag).get();
  }
  return nullptr;
}

void DependentNativeAnimatedNodeManager::RestoreDefaultValues(int64_t tag) {
  if (const auto propsNode = GetPropsAnimatedNode(tag)) {
    propsNode->RestoreDefaultValues();
  }
}

void DependentNativeAnimatedNodeManager::RunUpdates(winrt::TimeSpan renderingTime) {
  auto hasFinishedAnimations = false;
  m_updatingNodes = std::move(m_updatedNodes);
  UpdateActiveAnimationIds();

  // Increment animation drivers
  for (auto id : m_activeAnimationIds) {
    auto &animation = m_activeAnimations.at(id);
    animation->RunAnimationStep(renderingTime);
    m_updatingNodes.insert(animation->AnimatedValueTag());
    if (animation->IsComplete()) {
      hasFinishedAnimations = true;
    }
  }

  UpdateNodes();
  m_updatingNodes.clear();

  if (hasFinishedAnimations) {
    for (auto id : m_activeAnimationIds) {
      auto &animation = m_activeAnimations.at(id);
      if (animation->IsComplete()) {
        DoCallback(id, true);
        m_activeAnimations.erase(id);
      }
    }
  }
}

void DependentNativeAnimatedNodeManager::DoCallback(int64_t tag, bool finished) {
  if (m_activeAnimations.count(tag)) {
    auto &animation = m_activeAnimations.at(tag);
    if (animation->EndCallback()) {
      animation->EndCallback()(std::vector<folly::dynamic>{folly::dynamic::object("finished", finished)});
    }
  }
}

void DependentNativeAnimatedNodeManager::EnsureRendering() {
  m_renderingRevoker = xaml::Media::CompositionTarget::Rendering(
      winrt::auto_revoke, {this, &DependentNativeAnimatedNodeManager::OnRendering});
}

void DependentNativeAnimatedNodeManager::OnRendering(winrt::IInspectable const& sender, winrt::IInspectable const& args) {
  if (m_activeAnimations.size() > 0 || m_updatedNodes.size() > 0) {
    if (const auto renderingArgs = args.try_as<xaml::Media::RenderingEventArgs>()) {
      RunUpdates(renderingArgs.RenderingTime());
    }
  } else {
    m_renderingRevoker.revoke();
  }
}

void DependentNativeAnimatedNodeManager::StopAnimationsForNode(int64_t tag) {
  UpdateActiveAnimationIds();
  for (auto id : m_activeAnimationIds) {
    auto &animation = m_activeAnimations.at(id);
    if (tag == animation->AnimatedValueTag()) {
      DoCallback(id, false);
      m_activeAnimations.erase(id);         
    }
  }
}

void DependentNativeAnimatedNodeManager::UpdateActiveAnimationIds() {
  m_activeAnimationIds.clear();
  for (const auto &pair : m_activeAnimations) {
    m_activeAnimationIds.push_back(pair.first);
  }
}

void DependentNativeAnimatedNodeManager::UpdateNodes() {
  auto activeNodesCount = 0;
  auto updatedNodesCount = 0;

  // STEP 1.
  // BFS over graph of nodes starting from ones from `m_updatingNodes` and ones that are attached to
  // active animations (from `m_activeAnimations)`. Update `activeIncomingNodes` property for each node
  // during that BFS. Store number of visited nodes in `activeNodesCount`. We "execute" active
  // animations as a part of this step.

  m_animatedGraphBFSColor++; /* use new color */
  if (m_animatedGraphBFSColor == 0) {
    // value "0" is used as an initial color for a new node, using it in BFS may cause some nodes to be skipped.
    m_animatedGraphBFSColor++;
  }

  std::queue<int64_t> nodesQueue{};
  for (auto id : m_updatingNodes) {
    if (auto node = GetAnimatedNode(id)) {
      if (node->bfsColor != m_animatedGraphBFSColor) {
        node->bfsColor = m_animatedGraphBFSColor;
        activeNodesCount++;
        nodesQueue.push(id);
      }
    }
  }

  while (nodesQueue.size() > 0) {
    auto id = nodesQueue.front();
    nodesQueue.pop();
    if (auto node = GetAnimatedNode(id)) {
      for (auto& childId : node->Children()) {
        if (auto child = GetAnimatedNode(childId)) {
          child->activeIncomingNodes++;
          if (child->bfsColor != m_animatedGraphBFSColor) {
            child->bfsColor = m_animatedGraphBFSColor;
            activeNodesCount++;
            nodesQueue.push(childId);
          }
        }
      }
    }
  }

  // STEP 2
  // BFS over the graph of active nodes in topological order -> visit node only when all its
  // "predecessors" in the graph have already been visited. It is important to visit nodes in that
  // order as they may often use values of their predecessors in order to calculate "next state"
  // of their own. We start by determining the starting set of nodes by looking for nodes with
  // `ActiveIncomingNodes = 0` (those can only be the ones that we start BFS in the previous
  // step). We store number of visited nodes in this step in `updatedNodesCount`

  m_animatedGraphBFSColor++;
  if (m_animatedGraphBFSColor == 0) {
    // see reasoning for this check a few lines above
    m_animatedGraphBFSColor++;
  }

  // find nodes with zero "incoming nodes", those can be either nodes from `mUpdatedNodes` or
  // ones connected to active animations
  for (auto id : m_updatingNodes) {
    if (auto node = GetAnimatedNode(id)) {
      if (node->activeIncomingNodes == 0 && node->bfsColor != m_animatedGraphBFSColor) {
        node->bfsColor = m_animatedGraphBFSColor;
        updatedNodesCount++;
        nodesQueue.push(id);
      }
    }
  }

  // Run main "update" loop
  while (nodesQueue.size() > 0) {
    auto id = nodesQueue.front();
    nodesQueue.pop();
    if (auto node = GetAnimatedNode(id)) {
      node->Update();
      if (auto propsNode = GetPropsAnimatedNode(id)) {
        propsNode->UpdateView();
      } else if (auto valueNode = GetValueAnimatedNode(id)) {
        valueNode->OnValueUpdate();
      }

      for (auto& childId : node->Children()) {
        if (auto child = GetAnimatedNode(childId)) {
          child->activeIncomingNodes--;
          if (child->bfsColor != m_animatedGraphBFSColor && child->activeIncomingNodes == 0) {
            child->bfsColor = m_animatedGraphBFSColor;
            updatedNodesCount++;
            nodesQueue.push(childId);
          }
        }
      }
    }
  }

  // Verify that we've visited *all* active nodes. Throw otherwise as this would mean there is a
  // cycle in animated node graph. We also take advantage of the fact that all active nodes are
  // visited in the step above so that all the nodes properties `ActiveIncomingNodes` are set to
  // zero
  assert(activeNodesCount == updatedNodesCount);
}
} // namespace Microsoft::ReactNative
