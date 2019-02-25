// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2017-2018 The Hash developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternode-helpers.h"
#include "init.h"
#include "main.h"
#include "masternodeman.h"
#include "activemasternode.h"
#include "masternode-payments.h"
#include "amount.h"
#include "swifttx.h"

// A helper object for signing messages from Masternodes
CMasternodeSigner masternodeSigner;

void ThreadMasternodePool()
{
    if (fLiteMode) return; //disable all Masternode related functionality

    // Make this thread recognisable
    RenameThread("hash-mnpool");

    unsigned int c = 0;

    while (true) {
        MilliSleep(1000);

        // try to sync from all available nodes, one step at a time
        masternodeSync.Process();

        if (masternodeSync.IsBlockchainSynced()) {
            c++;

            // check if we should activate or ping every few minutes,
            // start right after sync is considered to be done
            if (c % MASTERNODE_PING_SECONDS == 0) activeMasternode.ManageStatus();

            if (c % 60 == 0) {
                mnodeman.CheckAndRemove();
                mnodeman.ProcessMasternodeConnections();
                masternodePayments.CleanPaymentList();
                CleanTransactionLocksList();
            }
        }
    }
}

bool CMasternodeSigner::IsVinAssociatedWithPubkey(CTxIn& vin, CPubKey& pubkey)
{
    CScript payee2;
    payee2 = GetScriptForDestination(pubkey.GetID());

    CAmount collateral = 7500 * COIN;

    CTransaction txVin;
    uint256 hash;
    if (GetTransaction(vin.prevout.hash, txVin, hash, true)) {
        BlockMap::iterator iter = mapBlockIndex.find(hash);
        if (iter != mapBlockIndex.end()) {
                int txnheight = iter->second->nHeight;

                if (txnheight <= GetSporkValue(SPORK_19_COLLAT_01)) {
                    collateral = 10000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_20_COLLAT_02)) {
                    collateral = 15000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_21_COLLAT_03)) {
                    collateral = 20000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_22_COLLAT_04)) {
                    collateral = 30000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_23_COLLAT_05)) {
                    collateral = 40000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_24_COLLAT_06)) {
                    collateral = 50000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_25_COLLAT_07)) {
                    collateral = 60000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_26_COLLAT_08)) {
                    collateral = 70000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_27_COLLAT_09)) {
                    collateral = 80000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_28_COLLAT_10)) {
                    collateral = 900000 * COIN;
                } else if (txnheight <= GetSporkValue(SPORK_29_COLLAT_11)) {
                    collateral = 100000 * COIN;
                }

        }

        BOOST_FOREACH (CTxOut out, txVin.vout) {
            if (out.nValue == collateral) {
                if (out.scriptPubKey == payee2) return true;
            }
        }
    }

    return false;
}

bool CMasternodeSigner::SetKey(std::string strSecret, std::string& errorMessage, CKey& key, CPubKey& pubkey)
{
    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(strSecret);

    if (!fGood) {
        errorMessage = _("Invalid private key.");
        return false;
    }

    key = vchSecret.GetKey();
    pubkey = key.GetPubKey();

    return true;
}

bool CMasternodeSigner::GetKeysFromSecret(std::string strSecret, CKey& keyRet, CPubKey& pubkeyRet)
{
    CBitcoinSecret vchSecret;

    if (!vchSecret.SetString(strSecret)) return false;

    keyRet = vchSecret.GetKey();
    pubkeyRet = keyRet.GetPubKey();

    return true;
}

bool CMasternodeSigner::SignMessage(std::string strMessage, std::string& errorMessage, vector<unsigned char>& vchSig, CKey key)
{
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    if (!key.SignCompact(ss.GetHash(), vchSig)) {
        errorMessage = _("Signing failed.");
        return false;
    }

    return true;
}

bool CMasternodeSigner::VerifyMessage(CPubKey pubkey, vector<unsigned char>& vchSig, std::string strMessage, std::string& errorMessage)
{
    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    CPubKey pubkey2;
    if (!pubkey2.RecoverCompact(ss.GetHash(), vchSig)) {
        errorMessage = _("Error recovering public key.");
        return false;
    }

    if (fDebug && pubkey2.GetID() != pubkey.GetID())
        LogPrintf("CMasternodeSigner::VerifyMessage -- keys don't match: %s %s\n", pubkey2.GetID().ToString(), pubkey.GetID().ToString());

    return (pubkey2.GetID() == pubkey.GetID());
}

bool CMasternodeSigner::SetCollateralAddress(std::string strAddress)
{
    CBitcoinAddress address;
    if (!address.SetString(strAddress)) {
        LogPrintf("CMasternodeSigner::SetCollateralAddress - Invalid collateral address\n");
        return false;
    }
    collateralPubKey = GetScriptForDestination(address.Get());
    return true;
}

bool CMasternodeSigner::IsCollateralAmount(const CAmount& amount)
{
    return
            amount == 10000  * COIN ||
            amount == 15000  * COIN ||
            amount == 20000  * COIN ||
            amount == 30000  * COIN ||
            amount == 40000  * COIN ||
            amount == 50000  * COIN ||
            amount == 60000  * COIN ||
            amount == 70000  * COIN ||
            amount == 80000  * COIN ||
            amount == 90000  * COIN ||
            amount == 100000  * COIN;
}
