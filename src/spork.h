// Copyright (c) 2014-2016 The Dash developers
// Copyright (c) 2016-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPORK_H
#define SPORK_H

#include "base58.h"
#include "key.h"
#include "main.h"
#include "net.h"
#include "sync.h"
#include "util.h"

#include "protocol.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

/*
    Don't ever reuse these IDs for other sporks
    - This would result in old clients getting confused about which spork is for what
*/
#define SPORK_START 10001
#define SPORK_END 10028

#define SPORK_2_SWIFTTX 10001
#define SPORK_3_SWIFTTX_BLOCK_FILTERING 10002
#define SPORK_5_MAX_VALUE 10004
#define SPORK_7_MASTERNODE_SCANNING 10006
#define SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT 10007
#define SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT 10008
#define SPORK_10_MASTERNODE_PAY_UPDATED_NODES 10009
#define SPORK_11_RESET_BUDGET 10010
#define SPORK_12_RECONSIDER_BLOCKS 10011
#define SPORK_13_ENABLE_SUPERBLOCKS 10012
#define SPORK_14_NEW_PROTOCOL_ENFORCEMENT 10013
#define SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2 10014
#define SPORK_16_MN_WINNER_MINIMUM_AGE 10015
#define SPORK_17_COLLAT_00 10016
#define SPORK_18_CURRENT_MN_COLLATERAL 10017
#define SPORK_19_COLLAT_01 10018
#define SPORK_20_COLLAT_02 10019
#define SPORK_21_COLLAT_03 10020
#define SPORK_22_COLLAT_04 10021
#define SPORK_23_COLLAT_05 10022
#define SPORK_24_COLLAT_06 10023
#define SPORK_25_COLLAT_07 10024
#define SPORK_26_COLLAT_08 10025
#define SPORK_27_COLLAT_09 10026
#define SPORK_28_COLLAT_10 10027
#define SPORK_29_COLLAT_11 10028
#define SPORK_30_COLLAT_12 10029
#define SPORK_31_COLLAT_13 10030
#define SPORK_32_COLLAT_14 10031
#define SPORK_33_COLLAT_15 10032
#define SPORK_34_COLLAT_16 10033
#define SPORK_35_COLLAT_17 10034
#define SPORK_36_COLLAT_18 10035
#define SPORK_37_COLLAT_19 10036
#define SPORK_38_COLLAT_20 10037
#define SPORK_39_COLLAT_21 10038
#define SPORK_40_COLLAT_22 10039
#define SPORK_41_COLLAT_23 10040
#define SPORK_42_COLLAT_24 10041
#define SPORK_43_COLLAT_25 10042
#define SPORK_44_COLLAT_26 10043
#define SPORK_45_COLLAT_27 10044
#define SPORK_46_COLLAT_28 10045
#define SPORK_47_COLLAT_29 10046
#define SPORK_48_COLLAT_30 10047

#define SPORK_2_SWIFTTX_DEFAULT 978307200                         //2001-1-1
#define SPORK_3_SWIFTTX_BLOCK_FILTERING_DEFAULT 1424217600        //2015-2-18
#define SPORK_5_MAX_VALUE_DEFAULT 1000                            //1000 HASH
#define SPORK_7_MASTERNODE_SCANNING_DEFAULT 978307200             //2001-1-1
#define SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT_DEFAULT 4070908800 //OFF
#define SPORK_9_MASTERNODE_BUDGET_ENFORCEMENT_DEFAULT 4070908800  //OFF
#define SPORK_10_MASTERNODE_PAY_UPDATED_NODES_DEFAULT 4070908800  //OFF
#define SPORK_11_RESET_BUDGET_DEFAULT 0
#define SPORK_12_RECONSIDER_BLOCKS_DEFAULT 0
#define SPORK_13_ENABLE_SUPERBLOCKS_DEFAULT 4070908800            //OFF
#define SPORK_14_NEW_PROTOCOL_ENFORCEMENT_DEFAULT 4070908800      //OFF
#define SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2_DEFAULT 4070908800    //OFF
#define SPORK_16_MN_WINNER_MINIMUM_AGE_DEFAULT 8000               // Age in seconds. This should be > MASTERNODE_REMOVAL_SECONDS to avoid
                                                                  // misconfigured new nodes in the list.
                                                                  // Set this to zero to emulate classic behaviour
#define SPORK_17_COLLAT_00_DEFAULT 99999999
#define SPORK_18_CURRENT_MN_COLLATERAL_DEFAULT 10000
#define SPORK_19_COLLAT_01_DEFAULT 99999999
#define SPORK_20_COLLAT_02_DEFAULT 99999999
#define SPORK_21_COLLAT_03_DEFAULT 99999999
#define SPORK_22_COLLAT_04_DEFAULT 99999999
#define SPORK_23_COLLAT_05_DEFAULT 99999999
#define SPORK_24_COLLAT_06_DEFAULT 99999999
#define SPORK_25_COLLAT_07_DEFAULT 99999999
#define SPORK_26_COLLAT_08_DEFAULT 99999999
#define SPORK_27_COLLAT_09_DEFAULT 99999999
#define SPORK_28_COLLAT_10_DEFAULT 99999999
#define SPORK_29_COLLAT_11_DEFAULT 99999999
#define SPORK_30_COLLAT_12_DEFAULT 99999999
#define SPORK_31_COLLAT_13_DEFAULT 99999999
#define SPORK_32_COLLAT_14_DEFAULT 99999999
#define SPORK_33_COLLAT_15_DEFAULT 99999999
#define SPORK_34_COLLAT_16_DEFAULT 99999999
#define SPORK_35_COLLAT_17_DEFAULT 99999999
#define SPORK_36_COLLAT_18_DEFAULT 99999999
#define SPORK_37_COLLAT_19_DEFAULT 99999999
#define SPORK_38_COLLAT_20_DEFAULT 99999999
#define SPORK_39_COLLAT_21_DEFAULT 99999999
#define SPORK_40_COLLAT_22_DEFAULT 99999999
#define SPORK_41_COLLAT_23_DEFAULT 99999999
#define SPORK_42_COLLAT_24_DEFAULT 99999999
#define SPORK_43_COLLAT_25_DEFAULT 99999999
#define SPORK_44_COLLAT_26_DEFAULT 99999999
#define SPORK_45_COLLAT_27_DEFAULT 99999999
#define SPORK_46_COLLAT_28_DEFAULT 99999999
#define SPORK_47_COLLAT_29_DEFAULT 99999999
#define SPORK_48_COLLAT_30_DEFAULT 99999999

class CSporkMessage;
class CSporkManager;

extern std::map<uint256, CSporkMessage> mapSporks;
extern std::map<int, CSporkMessage> mapSporksActive;
extern CSporkManager sporkManager;

void LoadSporksFromDB();
void ProcessSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);
int64_t GetSporkValue(int nSporkID);
bool IsSporkActive(int nSporkID);
void ReprocessBlocks(int nBlocks);

//
// Spork Class
// Keeps track of all of the network spork settings
//

class CSporkMessage
{
public:
    std::vector<unsigned char> vchSig;
    int nSporkID;
    int64_t nValue;
    int64_t nTimeSigned;

    uint256 GetHash()
    {
        uint256 n = HashQuark(BEGIN(nSporkID), END(nTimeSigned));
        return n;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(nSporkID);
        READWRITE(nValue);
        READWRITE(nTimeSigned);
        READWRITE(vchSig);
    }
};


class CSporkManager
{
private:
    std::vector<unsigned char> vchSig;
    std::string strMasterPrivKey;

public:
    CSporkManager()
    {
    }

    std::string GetSporkNameByID(int id);
    int GetSporkIDByName(std::string strName);
    bool UpdateSpork(int nSporkID, int64_t nValue);
    bool SetPrivKey(std::string strPrivKey);
    bool CheckSignature(CSporkMessage& spork);
    bool Sign(CSporkMessage& spork);
    void Relay(CSporkMessage& msg);
};

#endif
