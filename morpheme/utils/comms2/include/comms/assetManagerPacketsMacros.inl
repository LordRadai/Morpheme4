// Copyright (c) 2009 NaturalMotion.  All Rights Reserved.
// Not to be copied, adapted, modified, used, distributed, sold,
// licensed or commercially exploited in any manner without the
// written consent of NaturalMotion.
//
// All non public elements of this software are the confidential
// information of NaturalMotion and may not be disclosed to any
// person nor used for any purpose not expressly approved by
// NaturalMotion in writing.

//----------------------------------------------------------------------------------------------------------------------
// includes in various places, notably packet.h, to define and iterate over
// the various internal packet types.
//----------------------------------------------------------------------------------------------------------------------

// commands
PACKET(SetAnimBrowserAnimCmd)
PACKET(SetAnimBrowserNetworkCmd)

PACKET(LoadAnimBrowserDataCmd)
PACKET(AnimBrowserDataLoaded)

PACKET(SetAnimBrowserTimeCmd)

PACKET(DownloadAnimBrowserEventDetectionDataCmd)

PACKET(BeginAnimBrowser)
PACKET(AnimBrowserSourceData)
PACKET(EndAnimBrowser)

PACKET(AnimBrowserEventDetectionData)

//Unrelated to the assetManager but this is necessary so that the packets have the same IDs morphemeConnect 3.6.2 expects them to have
PACKET(ScatterBlendWeights)
PACKET(ConnectScriptCommand)
PACKET(NodeEventMessages)