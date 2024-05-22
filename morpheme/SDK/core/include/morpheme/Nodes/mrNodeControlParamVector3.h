// Copyright (c) 2010 NaturalMotion.  All Rights Reserved.
// Not to be copied, adapted, modified, used, distributed, sold,
// licensed or commercially exploited in any manner without the
// written consent of NaturalMotion.
//
// All non public elements of this software are the confidential
// information of NaturalMotion and may not be disclosed to any
// person nor used for any purpose not expressly approved by
// NaturalMotion in writing.

//----------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
  #pragma once
#endif
#ifndef MR_NODE_CONTROL_PARAM_VECTOR3_H
#define MR_NODE_CONTROL_PARAM_VECTOR3_H
//----------------------------------------------------------------------------------------------------------------------
#include "morpheme/mrNetwork.h"
//----------------------------------------------------------------------------------------------------------------------

namespace MR
{

//----------------------------------------------------------------------------------------------------------------------
// OutputCP op declarations.
AttribData* nodeControlParamVector3EmittedCPUpdateVector3(
  NodeDef* node,
  PinIndex outputCPPinIndex, ///< The output pin we have been asked to update.
  Network* net);

//----------------------------------------------------------------------------------------------------------------------
void nodeControlParamVector3InitInstance(
  NodeDef* node,
  Network* net);

} // namespace MR

//----------------------------------------------------------------------------------------------------------------------
#endif // MR_NODE_CONTROL_PARAM_VECTOR3_H
//----------------------------------------------------------------------------------------------------------------------
