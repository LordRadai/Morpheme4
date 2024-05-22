/*
 * Copyright (c) 2013 NaturalMotion Ltd. All rights reserved.
 *
 * Not to be copied, adapted, modified, used, distributed, sold,
 * licensed or commercially exploited in any manner without the
 * written consent of NaturalMotion.
 *
 * All non public elements of this software are the confidential
 * information of NaturalMotion and may not be disclosed to any
 * person nor used for any purpose not expressly approved by
 * NaturalMotion in writing.
 *
 */

//----------------------------------------------------------------------------------------------------------------------
//                                  This file is auto-generated
//----------------------------------------------------------------------------------------------------------------------

// module def dependencies
#include "Head.h"
#include "HeadSupport.h"
#include "HeadSupportPackaging.h"
#include "MyNetwork.h"
#include "BodyFrame.h"
#include "SittingBodyBalance.h"
#include "BalanceManagement.h"
#include "HeadPackaging.h"
#include "MyNetworkPackaging.h"

// misc
#include "euphoria/erEuphoriaLogger.h"
#include "euphoria/erDebugDraw.h"
#include "physics/mrPhysicsSerialisationBuffer.h"



namespace NM_BEHAVIOUR_LIB_NAMESPACE
{

//----------------------------------------------------------------------------------------------------------------------
HeadSupport::HeadSupport(ER::ModuleDataAllocator* mdAllocator, ER::ModuleCon* connectionSet) : ER::Module(mdAllocator, connectionSet)
{
  in = (HeadSupportInputs*)mdAllocator->alloc(ER::ModuleDataAllocator::Inputs, sizeof(HeadSupportInputs), __alignof(HeadSupportInputs));
  out = (HeadSupportOutputs*)mdAllocator->alloc(ER::ModuleDataAllocator::Outputs, sizeof(HeadSupportOutputs), __alignof(HeadSupportOutputs));

  m_apiBase = (HeadSupportAPIBase*)NMPMemoryAllocAligned(sizeof(HeadSupportAPIBase), 16);
  m_updatePackage = (HeadSupportUpdatePackage*)NMPMemoryAllocAligned(sizeof(HeadSupportUpdatePackage), 16);
  m_feedbackPackage = (HeadSupportFeedbackPackage*)NMPMemoryAllocAligned(sizeof(HeadSupportFeedbackPackage), 16);
}

//----------------------------------------------------------------------------------------------------------------------
HeadSupport::~HeadSupport()
{
  owner = 0;

  NMP::Memory::memFree(m_apiBase);
  NMP::Memory::memFree(m_updatePackage);
  NMP::Memory::memFree(m_feedbackPackage);
}

//----------------------------------------------------------------------------------------------------------------------
void HeadSupport::create(ER::Module* parent, int childIndex)
{
  ER::Module::create(parent, childIndex);
  owner = (Head*)parent;
  if (getConnections())
    getConnections()->create(this, (ER::Module*)owner);


  new(m_apiBase) HeadSupportAPIBase( in, owner->m_apiBase);
  new(m_updatePackage) HeadSupportUpdatePackage(
    in, out, owner->m_apiBase  );
  new(m_feedbackPackage) HeadSupportFeedbackPackage(
    in, owner->m_apiBase  );
}

//----------------------------------------------------------------------------------------------------------------------
void HeadSupport::clearAllData()
{
  in->clear();
  out->clear();
}

//----------------------------------------------------------------------------------------------------------------------
bool HeadSupport::storeState(MR::PhysicsSerialisationBuffer& savedState)
{
  savedState.addValue(*in);
  savedState.addValue(*out);
  storeStateChildren(savedState);
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool HeadSupport::restoreState(MR::PhysicsSerialisationBuffer& savedState)
{
  *in = savedState.getValue<HeadSupportInputs>();
  *out = savedState.getValue<HeadSupportOutputs>();
  restoreStateChildren(savedState);
  return true;
}

//----------------------------------------------------------------------------------------------------------------------
const char* HeadSupport::getChildName(int32_t) const
{
  NMP_ASSERT_FAIL();
  return 0;
}

//----------------------------------------------------------------------------------------------------------------------
void HeadSupport::update(float timeStep)
{
  const ER::RootModule* rootModule = getRootModule();
  MR_DEBUG_MODULE_ENTER(rootModule->getDebugInterface(), getClassName());
  m_updatePackage->update(timeStep, rootModule->getDebugInterface());
#if NM_CALL_VALIDATORS
#endif // NM_CALL_VALIDATORS
}

//----------------------------------------------------------------------------------------------------------------------
// No feedback() required

} // namespace NM_BEHAVIOUR_LIB_NAMESPACE


