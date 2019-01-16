// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2016-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "spork.h"
#include "base58.h"
#include "key.h"
#include "main.h"
#include "masternode-budget.h"
#include "masternode-helpers.h"
#include "net.h"
#include "protocol.h"
#include "sync.h"
#include "sporkdb.h"
#include "util.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

class CSporkMessage;
class CSporkManager;

CSporkManager sporkManager;

std::map<uint256, CSporkMessage> mapSporks;
std::map<int, CSporkMessage> mapSporksActive;

// HASH: on startup load spork values from previous session if they exist in the sporkDB
void LoadSporksFromDB()
{
    for (int i = SPORK_START; i <= SPORK_END; ++i) {
        // Since not all spork IDs are in use, we have to exclude undefined IDs
        std::string strSpork = sporkManager.GetSporkNameByID(i);
        if (strSpork == "Unknown") continue;

        // attempt to read spork from sporkDB
        CSporkMessage spork;
        if (!pSporkDB->ReadSpork(i, spork)) {
            LogPrintf("%s : no previous value for %s found in database\n", __func__, strSpork);
            continue;
        }

        // add spork to memory
        mapSporks[spork.GetHash()] = spork;
        mapSporksActive[spork.nSporkID] = spork;
        std::time_t result = spork.nValue;
        // If SPORK Value is greater than 1,000,000 assume it's actually a Date and then convert to a more readable format
        if (spork.nValue > 1000000) {
            LogPrintf("%s : loaded spork %s with value %d : %s", __func__,
                      sporkManager.GetSporkNameByID(spork.nSporkID), spork.nValue,
                      std::ctime(&result));
        } else {
            LogPrintf("%s : loaded spork %s with value %d\n", __func__,
                      sporkManager.GetSporkNameByID(spork.nSporkID), spork.nValue);
        }
    }
}

void ProcessSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv)
{
    if (fLiteMode) return; //disable all masternode related functionality

    if (strCommand == "spork") {
        //LogPrintf("ProcessSpork::spork\n");
        CDataStream vMsg(vRecv);
        CSporkMessage spork;
        vRecv >> spork;

        if (chainActive.Tip() == NULL) return;

        // Ignore spork messages about unknown/deleted sporks
        std::string strSpork = sporkManager.GetSporkNameByID(spork.nSporkID);
        if (strSpork == "Unknown") return;

        uint256 hash = spork.GetHash();
        if (mapSporksActive.count(spork.nSporkID)) {
            if (mapSporksActive[spork.nSporkID].nTimeSigned >= spork.nTimeSigned) {
                if (fDebug) LogPrintf("spork - seen %s block %d \n", hash.ToString(), chainActive.Tip()->nHeight);
                return;
            } else {
                if (fDebug) LogPrintf("spork - got updated spork %s block %d \n", hash.ToString(), chainActive.Tip()->nHeight);
            }
        }

        LogPrintf("spork - new %s ID %d Time %d bestHeight %d\n", hash.ToString(), spork.nSporkID, spork.nValue, chainActive.Tip()->nHeight);

        if (!sporkManager.CheckSignature(spork)) {
            LogPrintf("spork - invalid signature\n");
            Misbehaving(pfrom->GetId(), 100);
            return;
        }

        mapSporks[hash] = spork;
        mapSporksActive[spork.nSporkID] = spork;
        sporkManager.Relay(spork);

        // HASH: add to spork database.
        pSporkDB->WriteSpork(spork.nSporkID, spork);
    }
    if (strCommand == "getsporks") {
        std::map<int, CSporkMessage>::iterator it = mapSporksActive.begin();

        while (it != mapSporksActive.end()) {
            pfrom->PushMessage("spork", it->second);
            it++;
        }
    }
}


// grab the value of the spork on the network, or the default
int64_t GetSporkValue(int nSporkID)
{
    int64_t r = -1;

    if (mapSporksActive.count(nSporkID)) {
        r = mapSporksActive[nSporkID].nValue;
    } else {
        if (nSporkID == SPORK_2_SWIFTTX) r = SPORK_2_SWIFTTX_DEFAULT;
        if (nSporkID == SPORK_3_SWIFTTX_BLOCK_FILTERING) r = SPORK_3_SWIFTTX_BLOCK_FILTERING_DEFAULT;
        if (nSporkID == SPORK_5_MAX_VALUE) r = SPORK_5_MAX_VALUE_DEFAULT;
        if (nSporkID == SPORK_7_MASTERNODE_SCANNING) r = SPORK_7_MASTERNODE_SCANNING_DEFAULT;
        if (nSporkID == SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT) r = SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT_DEFAULT;
        if (nSporkID == SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT) r = SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT_DEFAULT;
        if (nSporkID == SPORK_10_MASTERNODE_PAY_UPDATED_NODES) r = SPORK_10_MASTERNODE_PAY_UPDATED_NODES_DEFAULT;
        if (nSporkID == SPORK_11_RESET_BUDGET) r = SPORK_11_RESET_BUDGET_DEFAULT;
        if (nSporkID == SPORK_12_RECONSIDER_BLOCKS) r = SPORK_12_RECONSIDER_BLOCKS_DEFAULT;
        if (nSporkID == SPORK_13_ENABLE_SUPERBLOCKS) r = SPORK_13_ENABLE_SUPERBLOCKS_DEFAULT;
        if (nSporkID == SPORK_14_NEW_PROTOCOL_ENFORCEMENT) r = SPORK_14_NEW_PROTOCOL_ENFORCEMENT_DEFAULT;
        if (nSporkID == SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2) r = SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2_DEFAULT;
        if (nSporkID == SPORK_16_MN_WINNER_MINIMUM_AGE) r = SPORK_16_MN_WINNER_MINIMUM_AGE_DEFAULT;
	if (nSporkID == SPORK_17_COLLAT_00) r = SPORK_17_COLLAT_00_DEFAULT;
	if (nSporkID == SPORK_18_CURRENT_MN_COLLATERAL) r = SPORK_18_CURRENT_MN_COLLATERAL_DEFAULT;
	if (nSporkID == SPORK_19_COLLAT_01) r = SPORK_19_COLLAT_01_DEFAULT;
	if (nSporkID == SPORK_20_COLLAT_02) r = SPORK_20_COLLAT_02_DEFAULT;
	if (nSporkID == SPORK_21_COLLAT_03) r = SPORK_21_COLLAT_03_DEFAULT;
	if (nSporkID == SPORK_22_COLLAT_04) r = SPORK_22_COLLAT_04_DEFAULT;
	if (nSporkID == SPORK_23_COLLAT_05) r = SPORK_23_COLLAT_05_DEFAULT;
	if (nSporkID == SPORK_24_COLLAT_06) r = SPORK_24_COLLAT_06_DEFAULT;
	if (nSporkID == SPORK_25_COLLAT_07) r = SPORK_25_COLLAT_07_DEFAULT;
	if (nSporkID == SPORK_26_COLLAT_08) r = SPORK_26_COLLAT_08_DEFAULT;
	if (nSporkID == SPORK_27_COLLAT_09) r = SPORK_27_COLLAT_09_DEFAULT;
	if (nSporkID == SPORK_28_COLLAT_10) r = SPORK_28_COLLAT_10_DEFAULT;
	if (nSporkID == SPORK_29_COLLAT_11) r = SPORK_29_COLLAT_11_DEFAULT;
	if (nSporkID == SPORK_30_COLLAT_12) r = SPORK_30_COLLAT_12_DEFAULT;
	if (nSporkID == SPORK_31_COLLAT_13) r = SPORK_31_COLLAT_13_DEFAULT;
	if (nSporkID == SPORK_32_COLLAT_14) r = SPORK_32_COLLAT_14_DEFAULT;
	if (nSporkID == SPORK_33_COLLAT_15) r = SPORK_33_COLLAT_15_DEFAULT;
	if (nSporkID == SPORK_34_COLLAT_16) r = SPORK_34_COLLAT_16_DEFAULT;
	if (nSporkID == SPORK_35_COLLAT_17) r = SPORK_35_COLLAT_17_DEFAULT;
	if (nSporkID == SPORK_36_COLLAT_18) r = SPORK_36_COLLAT_18_DEFAULT;
	if (nSporkID == SPORK_37_COLLAT_19) r = SPORK_37_COLLAT_19_DEFAULT;
	if (nSporkID == SPORK_38_COLLAT_20) r = SPORK_38_COLLAT_20_DEFAULT;
	if (nSporkID == SPORK_39_COLLAT_21) r = SPORK_39_COLLAT_21_DEFAULT;
	if (nSporkID == SPORK_40_COLLAT_22) r = SPORK_40_COLLAT_22_DEFAULT;
	if (nSporkID == SPORK_41_COLLAT_23) r = SPORK_41_COLLAT_23_DEFAULT;
	if (nSporkID == SPORK_42_COLLAT_24) r = SPORK_42_COLLAT_24_DEFAULT;
	if (nSporkID == SPORK_43_COLLAT_25) r = SPORK_43_COLLAT_25_DEFAULT;
	if (nSporkID == SPORK_44_COLLAT_26) r = SPORK_44_COLLAT_26_DEFAULT;
	if (nSporkID == SPORK_45_COLLAT_27) r = SPORK_45_COLLAT_27_DEFAULT;
	if (nSporkID == SPORK_46_COLLAT_28) r = SPORK_46_COLLAT_28_DEFAULT;
	if (nSporkID == SPORK_47_COLLAT_29) r = SPORK_47_COLLAT_29_DEFAULT;
	if (nSporkID == SPORK_48_COLLAT_30) r = SPORK_48_COLLAT_30_DEFAULT;

        if (r == -1) LogPrintf("GetSpork::Unknown Spork %d\n", nSporkID);
    }

    return r;
}

// grab the spork value, and see if it's off
bool IsSporkActive(int nSporkID)
{
    int64_t r = GetSporkValue(nSporkID);
    if (r == -1) return false;
    return r < GetTime();
}

void ReprocessBlocks(int nBlocks)
{
    std::map<uint256, int64_t>::iterator it = mapRejectedBlocks.begin();
    while (it != mapRejectedBlocks.end()) {
        //use a window twice as large as is usual for the nBlocks we want to reset
        if ((*it).second > GetTime() - (nBlocks * 60 * 5)) {
            BlockMap::iterator mi = mapBlockIndex.find((*it).first);
            if (mi != mapBlockIndex.end() && (*mi).second) {
                LOCK(cs_main);

                CBlockIndex* pindex = (*mi).second;
                LogPrintf("ReprocessBlocks - %s\n", (*it).first.ToString());

                CValidationState state;
                ReconsiderBlock(state, pindex);
            }
        }
        ++it;
    }

    CValidationState state;
    {
        LOCK(cs_main);
        DisconnectBlocksAndReprocess(nBlocks);
    }

    if (state.IsValid()) {
        ActivateBestChain(state);
    }
}

bool CSporkManager::CheckSignature(CSporkMessage& spork)
{
    //note: need to investigate why this is failing
    std::string strMessage = boost::lexical_cast<std::string>(spork.nSporkID) + boost::lexical_cast<std::string>(spork.nValue) + boost::lexical_cast<std::string>(spork.nTimeSigned);
    CPubKey pubkeynew(ParseHex(Params().SporkKey()));
    std::string errorMessage = "";
    if (masternodeSigner.VerifyMessage(pubkeynew, spork.vchSig, strMessage, errorMessage)) {
        return true;
    }

    return false;
}

bool CSporkManager::Sign(CSporkMessage& spork)
{
    std::string strMessage = boost::lexical_cast<std::string>(spork.nSporkID) + boost::lexical_cast<std::string>(spork.nValue) + boost::lexical_cast<std::string>(spork.nTimeSigned);

    CKey key2;
    CPubKey pubkey2;
    std::string errorMessage = "";

    if (!masternodeSigner.SetKey(strMasterPrivKey, errorMessage, key2, pubkey2)) {
        LogPrintf("CMasternodePayments::Sign - ERROR: Invalid masternodeprivkey: '%s'\n", errorMessage);
        return false;
    }

    if (!masternodeSigner.SignMessage(strMessage, errorMessage, spork.vchSig, key2)) {
        LogPrintf("CMasternodePayments::Sign - Sign message failed");
        return false;
    }

    if (!masternodeSigner.VerifyMessage(pubkey2, spork.vchSig, strMessage, errorMessage)) {
        LogPrintf("CMasternodePayments::Sign - Verify message failed");
        return false;
    }

    return true;
}

bool CSporkManager::UpdateSpork(int nSporkID, int64_t nValue)
{
    CSporkMessage msg;
    msg.nSporkID = nSporkID;
    msg.nValue = nValue;
    msg.nTimeSigned = GetTime();

    if (Sign(msg)) {
        Relay(msg);
        mapSporks[msg.GetHash()] = msg;
        mapSporksActive[nSporkID] = msg;
        return true;
    }

    return false;
}

void CSporkManager::Relay(CSporkMessage& msg)
{
    CInv inv(MSG_SPORK, msg.GetHash());
    RelayInv(inv);
}

bool CSporkManager::SetPrivKey(std::string strPrivKey)
{
    CSporkMessage msg;

    // Test signing successful, proceed
    strMasterPrivKey = strPrivKey;

    Sign(msg);

    if (CheckSignature(msg)) {
        LogPrintf("CSporkManager::SetPrivKey - Successfully initialized as spork signer\n");
        return true;
    } else {
        return false;
    }
}

int CSporkManager::GetSporkIDByName(std::string strName)
{
    if (strName == "SPORK_2_SWIFTTX") return SPORK_2_SWIFTTX;
    if (strName == "SPORK_3_SWIFTTX_BLOCK_FILTERING") return SPORK_3_SWIFTTX_BLOCK_FILTERING;
    if (strName == "SPORK_5_MAX_VALUE") return SPORK_5_MAX_VALUE;
    if (strName == "SPORK_7_MASTERNODE_SCANNING") return SPORK_7_MASTERNODE_SCANNING;
    if (strName == "SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT") return SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT;
    if (strName == "SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT") return SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT;
    if (strName == "SPORK_10_MASTERNODE_PAY_UPDATED_NODES") return SPORK_10_MASTERNODE_PAY_UPDATED_NODES;
    if (strName == "SPORK_11_RESET_BUDGET") return SPORK_11_RESET_BUDGET;
    if (strName == "SPORK_12_RECONSIDER_BLOCKS") return SPORK_12_RECONSIDER_BLOCKS;
    if (strName == "SPORK_13_ENABLE_SUPERBLOCKS") return SPORK_13_ENABLE_SUPERBLOCKS;
    if (strName == "SPORK_14_NEW_PROTOCOL_ENFORCEMENT") return SPORK_14_NEW_PROTOCOL_ENFORCEMENT;
    if (strName == "SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2") return SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2;
    if (strName == "SPORK_16_MN_WINNER_MINIMUM_AGE") return SPORK_16_MN_WINNER_MINIMUM_AGE;
    if (strName == "SPORK_17_COLLAT_00") return SPORK_17_COLLAT_00;
    if (strName == "SPORK_18_CURRENT_MN_COLLATERAL") return SPORK_18_CURRENT_MN_COLLATERAL;
    if (strName == "SPORK_19_COLLAT_01") return SPORK_19_COLLAT_01;
    if (strName == "SPORK_20_COLLAT_02") return SPORK_20_COLLAT_02;
    if (strName == "SPORK_21_COLLAT_03") return SPORK_21_COLLAT_03;
    if (strName == "SPORK_22_COLLAT_04") return SPORK_22_COLLAT_04;
    if (strName == "SPORK_23_COLLAT_05") return SPORK_23_COLLAT_05;
    if (strName == "SPORK_24_COLLAT_06") return SPORK_24_COLLAT_06;
    if (strName == "SPORK_25_COLLAT_07") return SPORK_25_COLLAT_07;
    if (strName == "SPORK_26_COLLAT_08") return SPORK_26_COLLAT_08;
    if (strName == "SPORK_27_COLLAT_09") return SPORK_27_COLLAT_09;
    if (strName == "SPORK_28_COLLAT_10") return SPORK_28_COLLAT_10;
    if (strName == "SPORK_29_COLLAT_11") return SPORK_29_COLLAT_11;
    if (strName == "SPORK_30_COLLAT_12") return SPORK_30_COLLAT_12;
    if (strName == "SPORK_31_COLLAT_13") return SPORK_31_COLLAT_13;
    if (strName == "SPORK_32_COLLAT_14") return SPORK_32_COLLAT_14;
    if (strName == "SPORK_33_COLLAT_15") return SPORK_33_COLLAT_15;
    if (strName == "SPORK_34_COLLAT_16") return SPORK_34_COLLAT_16;
    if (strName == "SPORK_35_COLLAT_17") return SPORK_35_COLLAT_17;
    if (strName == "SPORK_36_COLLAT_18") return SPORK_36_COLLAT_18;
    if (strName == "SPORK_37_COLLAT_19") return SPORK_37_COLLAT_19;
    if (strName == "SPORK_38_COLLAT_20") return SPORK_38_COLLAT_20;
    if (strName == "SPORK_39_COLLAT_21") return SPORK_39_COLLAT_21;
    if (strName == "SPORK_40_COLLAT_22") return SPORK_40_COLLAT_22;
    if (strName == "SPORK_41_COLLAT_23") return SPORK_41_COLLAT_23;
    if (strName == "SPORK_42_COLLAT_24") return SPORK_42_COLLAT_24;
    if (strName == "SPORK_43_COLLAT_25") return SPORK_43_COLLAT_25;
    if (strName == "SPORK_44_COLLAT_26") return SPORK_44_COLLAT_26;
    if (strName == "SPORK_45_COLLAT_27") return SPORK_45_COLLAT_27;
    if (strName == "SPORK_46_COLLAT_28") return SPORK_46_COLLAT_28;
    if (strName == "SPORK_47_COLLAT_29") return SPORK_47_COLLAT_29;
    if (strName == "SPORK_48_COLLAT_30") return SPORK_48_COLLAT_30;

    return -1;
}

std::string CSporkManager::GetSporkNameByID(int id)
{
    if (id == SPORK_2_SWIFTTX) return "SPORK_2_SWIFTTX";
    if (id == SPORK_3_SWIFTTX_BLOCK_FILTERING) return "SPORK_3_SWIFTTX_BLOCK_FILTERING";
    if (id == SPORK_5_MAX_VALUE) return "SPORK_5_MAX_VALUE";
    if (id == SPORK_7_MASTERNODE_SCANNING) return "SPORK_7_MASTERNODE_SCANNING";
    if (id == SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT) return "SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT";
    if (id == SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT) return "SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT";
    if (id == SPORK_10_MASTERNODE_PAY_UPDATED_NODES) return "SPORK_10_MASTERNODE_PAY_UPDATED_NODES";
    if (id == SPORK_11_RESET_BUDGET) return "SPORK_11_RESET_BUDGET";
    if (id == SPORK_12_RECONSIDER_BLOCKS) return "SPORK_12_RECONSIDER_BLOCKS";
    if (id == SPORK_13_ENABLE_SUPERBLOCKS) return "SPORK_13_ENABLE_SUPERBLOCKS";
    if (id == SPORK_14_NEW_PROTOCOL_ENFORCEMENT) return "SPORK_14_NEW_PROTOCOL_ENFORCEMENT";
    if (id == SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2) return "SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2";
    if (id == SPORK_16_MN_WINNER_MINIMUM_AGE) return "SPORK_16_MN_WINNER_MINIMUM_AGE";
    if (id == SPORK_17_COLLAT_00) return "SPORK_17_COLLAT_00";
    if (id == SPORK_18_CURRENT_MN_COLLATERAL) return "SPORK_18_CURRENT_MN_COLLATERAL";
    if (id == SPORK_19_COLLAT_01) return "SPORK_19_COLLAT_01";
    if (id == SPORK_20_COLLAT_02) return "SPORK_20_COLLAT_02";
    if (id == SPORK_21_COLLAT_03) return "SPORK_21_COLLAT_03";
    if (id == SPORK_22_COLLAT_04) return "SPORK_22_COLLAT_04";
    if (id == SPORK_23_COLLAT_05) return "SPORK_23_COLLAT_05";
    if (id == SPORK_24_COLLAT_06) return "SPORK_24_COLLAT_06";
    if (id == SPORK_25_COLLAT_07) return "SPORK_25_COLLAT_07";
    if (id == SPORK_26_COLLAT_08) return "SPORK_26_COLLAT_08";
    if (id == SPORK_27_COLLAT_09) return "SPORK_27_COLLAT_09";
    if (id == SPORK_28_COLLAT_10) return "SPORK_28_COLLAT_10";
    if (id == SPORK_29_COLLAT_11) return "SPORK_29_COLLAT_11";
    if (id == SPORK_30_COLLAT_12) return "SPORK_30_COLLAT_12";
    if (id == SPORK_31_COLLAT_13) return "SPORK_31_COLLAT_13";
    if (id == SPORK_32_COLLAT_14) return "SPORK_32_COLLAT_14";
    if (id == SPORK_33_COLLAT_15) return "SPORK_33_COLLAT_15";
    if (id == SPORK_34_COLLAT_16) return "SPORK_34_COLLAT_16";
    if (id == SPORK_35_COLLAT_17) return "SPORK_35_COLLAT_17";
    if (id == SPORK_36_COLLAT_18) return "SPORK_36_COLLAT_18";
    if (id == SPORK_37_COLLAT_19) return "SPORK_37_COLLAT_19";
    if (id == SPORK_38_COLLAT_20) return "SPORK_38_COLLAT_20";
    if (id == SPORK_39_COLLAT_21) return "SPORK_39_COLLAT_21";
    if (id == SPORK_40_COLLAT_22) return "SPORK_40_COLLAT_22";
    if (id == SPORK_41_COLLAT_23) return "SPORK_41_COLLAT_23";
    if (id == SPORK_42_COLLAT_24) return "SPORK_42_COLLAT_24";
    if (id == SPORK_43_COLLAT_25) return "SPORK_43_COLLAT_25";
    if (id == SPORK_44_COLLAT_26) return "SPORK_44_COLLAT_26";
    if (id == SPORK_45_COLLAT_27) return "SPORK_45_COLLAT_27";
    if (id == SPORK_46_COLLAT_28) return "SPORK_46_COLLAT_28";
    if (id == SPORK_47_COLLAT_29) return "SPORK_47_COLLAT_29";
    if (id == SPORK_48_COLLAT_30) return "SPORK_48_COLLAT_30";

    return "Unknown";
}
