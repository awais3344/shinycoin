// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2013 The PPCoin developers
// Copyright (c) 2013-2014 The ShinyCoin developers

#include "alert.h"
#include "key.h"
#include "kernel.h"
#include "checkpoints.h"

#include "ui_interface.h"


static const std::string strAlertMasterPubKey = "0433a92a2e6b9ddc052bee5b742e20e3efbc52f512a3b31dcaa435ab61956e34c232e7ea0d260a484efd0c3f938a00698a006fa2af8cbcc85f5e855b9cf50ea023";

std::map<uint256, CAlert> mapAlerts;
CCriticalSection cs_mapAlerts;

std::string strMintMessage = _("Info: Minting suspended due to locked wallet.");
std::string strMintWarning;

std::string GetWarnings(std::string strFor)
{
    int nPriority = 0;
    std::string strStatusBar;
    std::string strRPC;
    if (GetBoolArg("-testsafemode"))
        strRPC = "test";
    
    // ppcoin: wallet lock warning for minting
    if (strMintWarning != "")
    {
        nPriority = 0;
        strStatusBar = strMintWarning;
    }
    
    // Misc warnings like out of disk space and clock is wrong
    if (strMiscWarning != "")
    {
        nPriority = 1000;
        strStatusBar = strMiscWarning;
    }
    
    // ppcoin: should not enter safe mode for longer invalid chain
    // ppcoin: if sync-checkpoint is too old do not enter safe mode
    if (Checkpoints::IsSyncCheckpointTooOld(60 * 60 * 24 * 10) && !fTestNet)
    {
        nPriority = 100;
        strStatusBar = "WARNING: Checkpoint is too old. Wait for block chain to download, or notify developers of the issue.";
    }
    
    // ppcoin: if detected invalid checkpoint enter safe mode
    if (Checkpoints::hashInvalidCheckpoint != 0)
    {
        nPriority = 3000;
        strStatusBar = strRPC = "WARNING: Invalid checkpoint found! Displayed transactions may not be correct! You may need to upgrade, or notify developers of the issue.";
    }
    
    // ppcoin: if detected unmet upgrade requirement enter safe mode
    // Note: v0.4 upgrade requires blockchain redownload if past protocol switch
    if (IsProtocolV04(nProtocolV04UpgradeTime + 60*60*24)) // 1 day margin
    {
        nPriority = 5000;
        strStatusBar = strRPC = "WARNING: Blockchain redownload required approaching or past v0.4 upgrade deadline.";
    }
    
    // Alerts
    {
        LOCK(cs_mapAlerts);
        BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
        {
            const CAlert& alert = item.second;
            if (alert.AppliesToMe() && alert.nPriority > nPriority)
            {
                nPriority = alert.nPriority;
                strStatusBar = alert.strStatusBar;
                if (nPriority > 1000)
                    strRPC = strStatusBar;  // ppcoin: safe mode for high alert
            }
        }
    }
    
    if (strFor == "statusbar")
        return strStatusBar;
    else if (strFor == "rpc")
        return strRPC;
    assert(!"GetWarnings() : invalid parameter");
    return "error";
}

bool CAlert::ProcessAlert()
{
    if (!CheckSignature())
        return false;
    if (!IsInEffect())
        return false;
    
    {
        LOCK(cs_mapAlerts);
        // Cancel previous alerts
        for (std::map<uint256, CAlert>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();)
        {
            const CAlert& alert = (*mi).second;
            if (Cancels(alert))
            {
                printf("cancelling alert %d\n", alert.nID);
                mapAlerts.erase(mi++);
            }
            else if (!alert.IsInEffect())
            {
                printf("expiring alert %d\n", alert.nID);
                mapAlerts.erase(mi++);
            }
            else
                mi++;
        }
        
        // Check if this alert has been cancelled
        BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
        {
            const CAlert& alert = item.second;
            if (alert.Cancels(*this))
            {
                printf("alert already cancelled by %d\n", alert.nID);
                return false;
            }
        }
        
        // Add to mapAlerts
        mapAlerts.insert(std::make_pair(GetHash(), *this));
    }
    
    printf("accepted alert %d, AppliesToMe()=%d\n", nID, AppliesToMe());
    MainFrameRepaint();
    return true;
}

bool CAlert::CheckSignature() const {
    CKey key;
    if (!key.SetPubKey(ParseHex(strAlertMasterPubKey)))
        return error("CAlert::CheckSignature() : SetPubKey failed");
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CAlert::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedAlert*)this;
    return true;
}

