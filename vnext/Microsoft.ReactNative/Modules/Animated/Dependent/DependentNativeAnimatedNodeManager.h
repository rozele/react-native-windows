#pragma once
// Copyright (c) Microsoft Corporation.
// Portions derived from React Native:
// Copyright (c) 2015-present, Facebook, Inc.
// Licensed under the MIT License.

#include <IReactInstance.h>
#include <cxxreact/CxxModule.h>
#include <folly/dynamic.h>
#include "DependentAnimatedNode.h"
#include "DependentAnimationDriver.h"
#include "DependentPropsAnimatedNode.h"
#include "DependentStyleAnimatedNode.h"
#include "DependentTrackingAnimatedNode.h"
#include "DependentTransformAnimatedNode.h"
#include "DependentValueAnimatedNode.h"

namespace Microsoft::ReactNative {
/// <summary>
/// This is the main class that coordinates how native animated JS
/// implementation drives UI changes.
///
/// It implements a management interface for animated nodes graph and
/// establishes a number of composistion animations and property sets to
/// drive the animating of the nodes to the Xaml elements off the UI Thread
///
/// </summary>

typedef std::function<void(std::vector<folly::dynamic>)> Callback;

class DependentAnimatedNode;
class DependentStyleAnimatedNode;
class DependentPropsAnimatedNode;
class DependentValueAnimatedNode;
class DependentTransformAnimatedNode;
class DependentTrackingAnimatedNode;
class DependentAnimationDriver;
class DependentNativeAnimatedNodeManager {
 public:
  void CreateAnimatedNode(
      int64_t tag,
      const folly::dynamic &config,
      const Mso::CntPtr<Mso::React::IReactContext> &context,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);
  void GetValue(int64_t animatedNodeTag, const Callback &endCallback);
  void ConnectAnimatedNodeToView(int64_t propsNodeTag, int64_t viewTag);
  void DisconnectAnimatedNodeToView(int64_t propsNodeTag, int64_t viewTag);
  void ConnectAnimatedNode(int64_t parentNodeTag, int64_t childNodeTag);
  void DisconnectAnimatedNode(int64_t parentNodeTag, int64_t childNodeTag);
  void StopAnimation(int64_t animationId);
  void StartTrackingAnimatedNode(
      int64_t animationId,
      int64_t animatedNodeTag,
      int64_t animatedToValueTag,
      const folly::dynamic &animationConfig,
      const Callback &endCallback,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager,
      bool track = true);
  void StartAnimatingNode(
      int64_t animationId,
      int64_t animatedNodeTag,
      const folly::dynamic &animationConfig,
      const Callback &endCallback,
      const std::shared_ptr<DependentNativeAnimatedNodeManager> &manager);
  void DropAnimatedNode(int64_t tag);
  void SetAnimatedNodeValue(int64_t tag, double value);
  void SetAnimatedNodeOffset(int64_t tag, double offset);
  void FlattenAnimatedNodeOffset(int64_t tag);
  void ExtractAnimatedNodeOffset(int64_t tag);

  void RestoreDefaultValues(int64_t tag);

  DependentAnimatedNode *GetAnimatedNode(int64_t tag);
  DependentValueAnimatedNode *GetValueAnimatedNode(int64_t tag);
  DependentPropsAnimatedNode *GetPropsAnimatedNode(int64_t tag);
  DependentStyleAnimatedNode *GetStyleAnimatedNode(int64_t tag);
  DependentTransformAnimatedNode *GetTransformAnimatedNode(int64_t tag);
  DependentTrackingAnimatedNode *GetTrackingAnimatedNode(int64_t tag);

 private:
  std::unordered_map<int64_t, std::unique_ptr<DependentValueAnimatedNode>> m_valueNodes{};
  std::unordered_map<int64_t, std::unique_ptr<DependentPropsAnimatedNode>> m_propsNodes{};
  std::unordered_map<int64_t, std::unique_ptr<DependentStyleAnimatedNode>> m_styleNodes{};
  std::unordered_map<int64_t, std::unique_ptr<DependentTransformAnimatedNode>> m_transformNodes{};
  std::unordered_map<int64_t, std::unique_ptr<DependentTrackingAnimatedNode>> m_trackingNodes{};
  std::unordered_map<int64_t, std::shared_ptr<DependentAnimationDriver>> m_activeAnimations{};
  std::unordered_set<int64_t> m_updatedNodes{};
  std::unordered_set<int64_t> m_updatingNodes{};
  std::vector<int64_t> m_activeAnimationIds{};
  int64_t m_animatedGraphBFSColor{};
  xaml::Media::CompositionTarget::Rendering_revoker m_renderingRevoker;

  void DoCallback(int64_t id, bool finished);
  void EnsureRendering();
  void OnRendering(winrt::IInspectable const &sender, winrt::IInspectable const &args);
  void RunUpdates(winrt::TimeSpan renderingTime);
  void StopAnimationsForNode(int64_t tag);
  void UpdateActiveAnimationIds();
  void UpdateNodes();
};
} // namespace Microsoft::ReactNative
