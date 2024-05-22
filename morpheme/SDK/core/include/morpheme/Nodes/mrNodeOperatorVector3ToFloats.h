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
#ifndef MR_NODE_OPERATOR_VECTOR3_TO_FLOATS_H
#define MR_NODE_OPERATOR_VECTOR3_TO_FLOATS_H
//----------------------------------------------------------------------------------------------------------------------
#include "morpheme/mrNetwork.h"
//----------------------------------------------------------------------------------------------------------------------

namespace MR
{

//----------------------------------------------------------------------------------------------------------------------
// OutputCP op declarations.
AttribData* nodeOperatorVector3ToFloatsOutputCPUpdate(
  NodeDef* node,
  PinIndex outputCPPinIndex, /// The output pin we have been asked to update. We have 3:
                             ///  0 = X, 1 = Y, 2 = Z.
  Network* net);

//----------------------------------------------------------------------------------------------------------------------
void nodeOperatorVector3ToFloatsInitInstance(
  NodeDef* node, 
  Network* net);

//----------------------------------------------------------------------------------------------------------------------
// Enumerating the output control parameter pins.
enum NodeOperatorVector3ToFloatsOutCPPinIDs
{
  NODE_OPERATOR_VECTOR3_TO_FLOATS_OUT_CP_PINID_X = 0,
  NODE_OPERATOR_VECTOR3_TO_FLOATS_OUT_CP_PINID_Y,
  NODE_OPERATOR_VECTOR3_TO_FLOATS_OUT_CP_PINID_Z,
  NODE_OPERATOR_VECTOR3_TO_FLOATS_OUT_CP_PINID_COUNT
};

} // namespace MR

//----------------------------------------------------------------------------------------------------------------------
#endif // MR_NODE_OPERATOR_VECTOR3_TO_FLOATS_H
//----------------------------------------------------------------------------------------------------------------------
