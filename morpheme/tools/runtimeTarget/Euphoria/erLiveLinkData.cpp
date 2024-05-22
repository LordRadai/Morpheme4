/*
* Copyright (c) 2011 NaturalMotion Ltd. All rights reserved.
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
#include "erLiveLinkData.h"
#include "euphoria/erModule.h"
#include "euphoria/erBehaviour.h"
#include "euphoria/erDebugControls.h"
#include "morpheme/mrNetwork.h"
#include "physics/mrPhysicsRigDef.h"
#include "mrPhysicsRigPhysX3Articulation.h"
#include "mrPhysicsScenePhysX3.h"
#include "mrPhysX3Configure.h"
#include "physics/mrPhysics.h"
#include "defaultDataManager.h"
#include "iPhysicsMgr.h"

#include "euphoria/erBody.h"

#include "mrPhysX3Includes.h"

// HARDWIRED FOR NOW, UNTIL WE SUPPORT MULTIPLE LIBS / NS
#include "AutoGenerated/NetworkForceInclude.h"
#include "AutoGenerated/NetworkDescriptor.h"
using namespace NM_BEHAVIOUR_LIB_NAMESPACE;

//----------------------------------------------------------------------------------------------------------------------
ERNetworkDefRecord::ERNetworkDefRecord(
  const MCOMMS::GUID&       guid,
  const char*               name,
  MR::NetworkDef*           networkDef,
  MR::NetworkPredictionDef* netPredictionDef) :
    NetworkDefRecord(guid, name, networkDef, netPredictionDef)
{
  m_behaviourLibrary = new NMBehaviourLibrary;
  m_behaviourLibrary->init();

  m_characterDef.create(
    networkDef,
    m_behaviourLibrary->getBehaviourDefs());
}

//----------------------------------------------------------------------------------------------------------------------
ERNetworkDefRecord::~ERNetworkDefRecord()
{
  m_characterDef.destroy();
  m_behaviourLibrary->term();
  delete m_behaviourLibrary;
}

//----------------------------------------------------------------------------------------------------------------------
ERNetworkInstanceRecord::ERNetworkInstanceRecord(
  MCOMMS::InstanceID id,
  NetworkDefRecord* networkDefRecord,
  MCOMMS::Connection* owner,
  const char* instanceName) :
  NetworkInstanceRecord(id, networkDefRecord, owner, instanceName),
  m_character(0),
  // The proxy character is a capsule that attaches to the pelvis and is roughly the shape of the character
  // it is not used in collisions, only in collision probes, so that behaviours are aware of the other character.
  // Default value is true. It can be set prior to calling init.
  m_useProxyCharacter(true)
{
}

//----------------------------------------------------------------------------------------------------------------------
ERNetworkInstanceRecord::~ERNetworkInstanceRecord()
{
  // Everything is dealt with in term.
}

//----------------------------------------------------------------------------------------------------------------------
void ERNetworkInstanceRecord::term()
{
  // TODO: sort this out, dealing with referencing.
  if (m_character)
  {
    // Free the network ref we created
    ER::RootModule* netToDelete = m_character->m_euphoriaRootModule;
    ERNetworkDefRecord* netDefRecord = (ERNetworkDefRecord*)getNetworkDefRecord();
    netDefRecord->m_characterDef.destroyInstance(m_character);
    netDefRecord->m_behaviourLibrary->destroyNetwork((ER::Module**)&netToDelete);
    m_character = 0;
  }
  if (m_useProxyCharacter)
  {
    m_interactionProxy.term();
  }
}

//----------------------------------------------------------------------------------------------------------------------
static ER::InteractionProxyDef* getInteractionProxyDefFromNetwork(MR::NetworkDef* networkDef, MR::AnimSetIndex animSetIndex)
{
  ER::InteractionProxyDef* result = NULL;
  MR::AttribDataHandle* interactionProxyDefADH = networkDef->getOptionalAttribDataHandle(
    (MR::AttribDataSemantic)ER::ATTRIB_SEMANTIC_INTERACTION_PROXY_DEF, 0, animSetIndex);
  if (!interactionProxyDefADH || !interactionProxyDefADH->m_attribData)
  {
    return 0;
  }
  MR::AttribData* interactionProxyDefAD = interactionProxyDefADH->m_attribData;
  NMP_ASSERT(interactionProxyDefAD->getType() == ER::ATTRIB_TYPE_INTERACTION_PROXY_DEF);
  result = ((ER::AttribDataInteractionProxyDef*)interactionProxyDefAD)->m_interactionProxyDef;
  NMP_ASSERT(result);
  return result;
}



//----------------------------------------------------------------------------------------------------------------------
bool ERNetworkInstanceRecord::init(
  uint32_t            animSetIndex,
  const NMP::Vector3& characterStartPosition,
  const NMP::Quat&    characterStartRotation,
  bool                NMP_UNUSED(stepNetwork))
{
  NetworkInstanceRecord::init(animSetIndex, characterStartPosition, characterStartRotation, false);

  MR::NetworkDef* netDef = getNetworkDefRecord()->getNetDef();

  // Check if the network contains behaviours.
  bool isBehavioural = netDef->containsNodeWithFlagsSet(MR::NodeDef::NODE_FLAG_IS_BEHAVIOURAL);

  MR::AnimRigDef* rig = getNetwork()->getActiveRig();
  MR::PhysicsRigPhysX3* physicsRig = (MR::PhysicsRigPhysX3*) getPhysicsRig(getNetwork());

  // If there's no interaction proxy then don't set the ignore flags on the physics rig. Note that
  // just because we don't have a proxy shape doesn't mean we shouldn't interact with other proxy
  // shapes (and vice versa).
  if (m_useProxyCharacter)
  {
    ER::InteractionProxyDef* interactionProxyDef = getInteractionProxyDefFromNetwork(
      netDef, (MR::AnimSetIndex) animSetIndex);
    if (!interactionProxyDef)
    {
      m_useProxyCharacter = false;
    }
  } 

  if (physicsRig)
  {
    physicsRig->setRigAndAnimToPhysicsMap(
      rig, 
      getAnimToPhysicsMap(getNetwork()->getNetworkDef(), 
      getNetwork()->getActiveAnimSetIndex()));

    if (isBehavioural && physicsRig->getType() == MR::PhysicsRigPhysX3::TYPE_ARTICULATED)
    {
      uint32_t ignoreGroups = (1 << MR::GROUP_CHARACTER_CONTROLLER) | (1 << MR::GROUP_NON_COLLIDABLE) | (1 << MR::GROUP_CHARACTER_PART_WITH_PROXY);
      if (m_useProxyCharacter)
      {
        physicsRig->addQueryFilterFlagToParts(1 << MR::GROUP_CHARACTER_PART_WITH_PROXY, 0, 0, 0);
      }

      ERNetworkDefRecord* netDefRecord = (ERNetworkDefRecord*)getNetworkDefRecord();

      // Create the network using the network description system.
      ER::RootModule* behaviourNetwork = netDefRecord->m_behaviourLibrary->createNetwork();
      // For testing it's convenient to make the behaviour random number generators deterministic,
      // but not all the same. This is entirely optional (a game would probably not do this - i.e.
      // not call setNetworkRandomseed). Here we use a seed based on the position, assuming that
      // multiple characters get instantiated at different locations.
      uint32_t seed = 
        NMP::hashFunction(* ((uint32_t*) &characterStartPosition.x)) + 
        NMP::hashFunction(* ((uint32_t*) &characterStartPosition.y)) + 
        NMP::hashFunction(* ((uint32_t*) &characterStartPosition.z));
      behaviourNetwork->setNetworkRandomSeed(seed);

      m_character = netDefRecord->m_characterDef.createInstance(
          rig,
          (MR::PhysicsRigPhysX3Articulation*)physicsRig,
          behaviourNetwork,
          ignoreGroups,
          !m_useProxyCharacter,
          &m_debugInterface);

      ER::networkSetCharacter(m_network, m_character);

      // Add all known modules to the debug interface with initial debug draw status set to false.
      initDebugInterfaceModuleNames();
      initDebugInterfaceControlNames();
    }
    else
    {
      m_character = 0;
    }
  }

  // We do the init step of the network here, instead of in NetworkInstanceRecord
  MR::Task* task;
  MR::ExecuteResult execResult;

  m_network->startUpdate(0.0f, true);

  while ((execResult = m_network->update(task)) != MR::EXECUTE_RESULT_COMPLETE)
  {
    if (execResult != MR::EXECUTE_RESULT_IN_PROGRESS)
    {
      // should just be getting the internal physics tasks
      NMP_ASSERT(
        task->m_taskid == MR::CoreTaskIDs::MR_TASKID_NETWORKUPDATECHARACTERCONTROLLER ||
        task->m_taskid == MR::CoreTaskIDs::MR_TASKID_NETWORKUPDATEPHYSICS ||
        task->m_taskid == MR::CoreTaskIDs::MR_TASKID_NETWORKUPDATEROOT);

      (void)task;
    }
  }

  // As NetworkInstanceRecord owns the temp allocator instead of the Network, we are responsible for resetting it.
  m_tempAllocator->reset();

  if (m_useProxyCharacter)
  {
    ER::InteractionProxyDef* interactionProxyDef = getInteractionProxyDefFromNetwork(
      netDef, (MR::AnimSetIndex) animSetIndex);
    NMP_ASSERT(interactionProxyDef);
    if (physicsRig)
    {
      m_interactionProxy.init(interactionProxyDef, ER::InteractionProxySetup(physicsRig));
    } 
    else
    {
      physx::PxScene* physicsScene = 0;
      PxGetPhysics().getScenes(&physicsScene, 1);
      // This just chooses a vaguely sensible mass so that characters will not ignore this
      // interaction proxy. For real use, this mass should be similar to the character's mass.
      float mass = PxGetPhysics().getTolerancesScale().mass * 60.0f / 1000.0f;
      ER::InteractionProxySetup interactionProxySetup(
        mass, 
        NMP::Matrix34(characterStartRotation, characterStartPosition),
        physicsScene);
      m_interactionProxy.init(interactionProxyDef, interactionProxySetup);
    }
   } 

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
// consts used here for array sizes
const uint32_t g_maxModules = 120;
const uint32_t g_maxModuleNameLength = 64;
void ERNetworkInstanceRecord::initDebugInterfaceModuleNames()
{
  NMP_ASSERT(m_character);
  char allModuleNames[g_maxModules][g_maxModuleNameLength];
  char allModuleParentNames[g_maxModules][g_maxModuleNameLength];
  char* moduleNames[g_maxModules];
  char* moduleParentNames[g_maxModules];
  for (uint32_t i = 0; i < g_maxModules; ++i)
  {
    moduleNames[i] = (char*) & (allModuleNames[i]);
    moduleParentNames[i] = (char*) & (allModuleParentNames[i]);
  }

  int32_t numModules = 0;
  if (m_character->m_euphoriaRootModule)
  {
    m_character->m_euphoriaRootModule->getModuleNames(
      &allModuleNames[0][0], &allModuleParentNames[0][0], g_maxModuleNameLength, numModules);
  }

  NMP_ASSERT((uint32_t)numModules <= g_maxModules);
  m_debugInterface.initModuleNames(numModules, moduleNames, moduleParentNames);
}

//----------------------------------------------------------------------------------------------------------------------
const uint32_t kMaxControls = 120;
void ERNetworkInstanceRecord::initDebugInterfaceControlNames()
{
  uint32_t totalControls = ER::getEuphoriaDebugControlCount();
  uint32_t totalControlNames = 0;
  const char* controlNames[kMaxControls];

  for (uint32_t i = 0 ; i < totalControls ; i++)
  {
    controlNames[totalControlNames] = ER::getEuphoriaDebugControlName(i);
    if (controlNames[totalControlNames] != NULL)
      totalControlNames++;
  }

  NMP_ASSERT((uint32_t)totalControlNames <= kMaxControls);
  m_debugInterface.initControlNames(totalControlNames, controlNames);
}

//----------------------------------------------------------------------------------------------------------------------
void ERNetworkInstanceRecord::updateInteractionProxy(float deltaTime)
{
  if (m_useProxyCharacter)
  {
    m_interactionProxy.update(deltaTime, *m_network, m_network->getCurrentFrameNo(), true);
  }
}


//----------------------------------------------------------------------------------------------------------------------
void ERNetworkInstanceRecord::updatePrePhysics(float deltaTime)
{
  if (!m_character)
  {
    return;
  }

  // Only update Euphoria if the physics rig exists and is referenced - if it exists and is not
  // referenced it might only have been added to the network after update connections, in which case
  // it wasn't initialised.
  MR::PhysicsRig* physicsRig = getPhysicsRig(getNetwork());
  bool usePhysicsRig = physicsRig && physicsRig->isReferenced();
  if (getNetwork()->areBehaviourNodesActive() && usePhysicsRig)
  {
    m_character->prePhysicsStep(deltaTime);
  }
  else
  {
    m_character->disableBehaviourEffects();
  }

  // TODO: This code will only be called if there is an ER character in the record.  otherwise it's called in
  //  DefaultPhysicsMgr::updatePrePhysics.  This needs cleaning up.
  if (usePhysicsRig)
    physicsRig->updatePrePhysics(deltaTime);
}

//----------------------------------------------------------------------------------------------------------------------
void ERNetworkInstanceRecord::updatePostPhysics(float deltaTime)
{
  NET_LOG_ENTER_FUNC();

  // Only update Euphoria if the physics rig exists and is referenced - if it exists and is not
  // referenced it might only have been added to the network after update connections, in which case
  // it wasn't initialised.
  MR::PhysicsRig* physicsRig = getPhysicsRig(getNetwork());
  bool usePhysicsRig = physicsRig && physicsRig->isReferenced();
  if (m_character && getNetwork()->areBehaviourNodesActive() && usePhysicsRig)
  {
    m_character->postPhysicsStep(deltaTime);
  }

#if 0
  // Enable this to test removal of the character (but not the physics rig) at runtime. However,
  // note that this won't necessarily work, as the character is responsible for starting/stopping
  // behaviours, and that won't be preserved between removing/re-adding the character.
  static bool removeCharacter = false;
  if (removeCharacter)
  {
    ER::networkSetCharacter(m_network, 0);
  }
  else
  {
    ER::networkSetCharacter(m_network, m_character);
  }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ERNetworkInstanceRecord::destroyCharacter()
{
  if (m_character)
  {
    // We need to destroy the character because the physics rig is not available anymore.
    ER::RootModule* netToDelete = m_character->m_euphoriaRootModule;
    ERNetworkDefRecord* netDefRecord = (ERNetworkDefRecord*)getNetworkDefRecord();
    netDefRecord->m_characterDef.destroyInstance(m_character);
    netDefRecord->m_behaviourLibrary->destroyNetwork((ER::Module**)&netToDelete);
    m_character = 0;
  }
}

//----------------------------------------------------------------------------------------------------------------------
