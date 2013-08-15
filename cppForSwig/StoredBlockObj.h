#ifndef _STORED_BLOCK_OBJ_
#define _STORED_BLOCK_OBJ_

#include <vector>
#include <list>
#include <map>
#include "BinaryData.h"
#include "BtcUtils.h"
#include "BlockObj.h"

#define ARMORY_DB_VERSION   0x00
#define ARMORY_DB_DEFAULT   ARMORY_DB_FULL
#define UTXO_STORAGE        SCRIPT_UTXO_VECTOR

enum BLKDATA_TYPE
{
  NOT_BLKDATA,
  BLKDATA_HEADER,
  BLKDATA_TX,
  BLKDATA_TXOUT
};

enum DB_PREFIX
{
  DB_PREFIX_DBINFO,
  DB_PREFIX_HEADHASH,
  DB_PREFIX_HEADHGT,
  DB_PREFIX_TXDATA,
  DB_PREFIX_TXHINTS,
  DB_PREFIX_SCRIPT,
  DB_PREFIX_UNDODATA,
  DB_PREFIX_TRIENODES,
  DB_PREFIX_COUNT
};


enum ARMORY_DB_TYPE
{
  ARMORY_DB_LITE,
  ARMORY_DB_PARTIAL,
  ARMORY_DB_FULL,
  ARMORY_DB_SUPER,
  ARMORY_DB_WHATEVER
};

enum DB_PRUNE_TYPE
{
  DB_PRUNE_ALL,
  DB_PRUNE_NONE,
  DB_PRUNE_WHATEVER
};


// In ARMORY_DB_PARTIAL and LITE, we may not store full tx, but we will know 
// its block and index, so we can just request the full block from our peer.
enum DB_TX_AVAIL
{
  DB_TX_EXISTS,
  DB_TX_GETBLOCK,
  DB_TX_UNKNOWN
};

enum DB_SELECT
{
  HEADERS,
  BLKDATA,
  DB_COUNT
};


enum TX_SERIALIZE_TYPE
{
  TX_SER_FULL,
  TX_SER_FRAGGED,
  TX_SER_COUNTOUT
};

enum TXOUT_SPENTNESS
{
  TXOUT_UNSPENT,
  TXOUT_SPENT,
  TXOUT_SPENTUNK,
};

enum MERKLE_SER_TYPE
{
  MERKLE_SER_NONE,
  MERKLE_SER_PARTIAL,
  MERKLE_SER_FULL
};

enum SCRIPT_UTXO_TYPE
{
  SCRIPT_UTXO_VECTOR,
  SCRIPT_UTXO_TREE
};

class BlockHeader;
class Tx;
class TxIn;
class TxOut;
class TxRef;
class TxIOPair;

class StoredTx;
class StoredTxOut;
class StoredScriptHistory;


#define DBUtils GlobalDBUtilities::GetInstance()

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Basically making stuff globally accessible through DBUtils singleton
////////////////////////////////////////////////////////////////////////////////
class GlobalDBUtilities
{
public:

   static uint32_t   hgtxToHeight(BinaryData hgtx);
   static uint8_t    hgtxToDupID(BinaryData hgtx);
   static BinaryData heightAndDupToHgtx(uint32_t hgt, uint8_t dup);

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData getBlkDataKey(uint32_t height, 
                            uint8_t  dup);

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData getBlkDataKey(uint32_t height, 
                            uint8_t  dup,
                            uint16_t txIdx);

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData getBlkDataKey(uint32_t height, 
                            uint8_t  dup,
                            uint16_t txIdx,
                            uint16_t txOutIdx);

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData getBlkDataKeyNoPrefix(uint32_t height, 
                                           uint8_t  dup);

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData getBlkDataKeyNoPrefix(uint32_t height, 
                                           uint8_t  dup,
                                           uint16_t txIdx);

   /////////////////////////////////////////////////////////////////////////////
   static BinaryData getBlkDataKeyNoPrefix(uint32_t height, 
                                           uint8_t  dup,
                                           uint16_t txIdx,
                                           uint16_t txOutIdx);



   /////////////////////////////////////////////////////////////////////////////
   static BLKDATA_TYPE readBlkDataKey( BinaryRefReader & brr,
                                       uint32_t & height,
                                       uint8_t  & dupID);

   /////////////////////////////////////////////////////////////////////////////
   static BLKDATA_TYPE readBlkDataKey( BinaryRefReader & brr,
                                       uint32_t & height,
                                       uint8_t  & dupID,
                                       uint16_t & txIdx);

   /////////////////////////////////////////////////////////////////////////////
   static BLKDATA_TYPE readBlkDataKey( BinaryRefReader & brr,
                                       uint32_t & height,
                                       uint8_t  & dupID,
                                       uint16_t & txIdx,
                                       uint16_t & txOutIdx);
   /////////////////////////////////////////////////////////////////////////////
   static BLKDATA_TYPE readBlkDataKeyNoPrefix( 
                                       BinaryRefReader & brr,
                                       uint32_t & height,
                                       uint8_t  & dupID);

   /////////////////////////////////////////////////////////////////////////////
   static BLKDATA_TYPE readBlkDataKeyNoPrefix( 
                                       BinaryRefReader & brr,
                                       uint32_t & height,
                                       uint8_t  & dupID,
                                       uint16_t & txIdx);

   /////////////////////////////////////////////////////////////////////////////
   static BLKDATA_TYPE readBlkDataKeyNoPrefix( 
                                       BinaryRefReader & brr,
                                       uint32_t & height,
                                       uint8_t  & dupID,
                                       uint16_t & txIdx,
                                       uint16_t & txOutIdx);
   


   static string getPrefixName(uint8_t prefixInt);
   static string getPrefixName(DB_PREFIX pref);

   static bool checkPrefixByte(       BinaryRefReader brr, 
                                      DB_PREFIX prefix,
                                      bool rewindWhenDone=false);
   static bool checkPrefixByteWError( BinaryRefReader brr, 
                                      DB_PREFIX prefix,
                                      bool rewindWhenDone);

   static void setArmoryDbType(ARMORY_DB_TYPE adt) { armoryDbType_ = adt; }
   static void setDbPruneType( DB_PRUNE_TYPE dpt)  { dbPruneType_  = dpt; }

   static ARMORY_DB_TYPE getArmoryDbType(void) { return armoryDbType_; }
   static DB_PRUNE_TYPE  getDbPruneType(void)  { return dbPruneType_;  }

   static GlobalDBUtilities& GetInstance(void)
   {
      if(theOneUtilsObj_==NULL)
      {
         theOneUtilsObj_ = new GlobalDBUtilities;
      
         // Default database structure
         theOneUtilsObj_->setArmoryDbType(ARMORY_DB_FULL);
         theOneUtilsObj_->setDbPruneType(DB_PRUNE_NONE);
      }
      return (*theOneUtilsObj_);
   }


   
private:
   GlobalDBUtilities(void) {}
   static GlobalDBUtilities* theOneUtilsObj_; 
   static DB_PRUNE_TYPE  dbPruneType_;
   static ARMORY_DB_TYPE armoryDbType_;
};


////////////////////////////////////////////////////////////////////////////////
class StoredDBInfo
{
public:
   StoredDBInfo(void) : 
      magic_(0),
      topBlkHgt_(UINT32_MAX),
      topBlkHash_(0),
      armoryVer_(ARMORY_DB_VERSION),
      armoryType_(DBUtils.getArmoryDbType()),
      pruneType_(DBUtils.getDbPruneType())   {}

   bool isInitialized(void) const { return magic_.getSize() > 0; }

   static BinaryData getDBKey(void);
   
   void       unserializeDBValue(BinaryRefReader & brr);
   void         serializeDBValue(BinaryWriter &    bw ) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(void) const;
   void       unserializeDBKey(BinaryDataRef key) {}

   void pprintOneLine(uint32_t indent=3);

   BinaryData      magic_;
   uint32_t        topBlkHgt_;
   BinaryData      topBlkHash_;
   uint32_t        armoryVer_;
   ARMORY_DB_TYPE  armoryType_;
   DB_PRUNE_TYPE   pruneType_;
};


////////////////////////////////////////////////////////////////////////////////
class StoredHeader
{
public:
   StoredHeader(void) : dataCopy_(0), 
                        thisHash_(0), 
                        numTx_(UINT32_MAX), 
                        numBytes_(UINT32_MAX), 
                        blockHeight_(UINT32_MAX), 
                        duplicateID_(UINT8_MAX), 
                        merkle_(0), 
                        isMainBranch_(false),
                        blockAppliedToDB_(false) {}
                           

   bool isInitialized(void) const {return dataCopy_.getSize() > 0;}
   bool haveFullBlock(void) const;
   BlockHeader getBlockHeaderCopy(void) const;
   BinaryData getSerializedBlock(void) const;
   BinaryData getSerializedBlockHeader(void) const;
   void createFromBlockHeader(BlockHeader & bh);

   void addTxToMap(uint16_t txIdx, Tx & tx);
   void addStoredTxToMap(uint16_t txIdx, StoredTx & tx);

   void setKeyData(uint32_t hgt, uint8_t dupID=UINT8_MAX);
   void setHeightAndDup(uint32_t hgt, uint8_t dupID);
   void setHeightAndDup(BinaryData hgtx);

   void unserialize(BinaryData const & header80B);
   void unserialize(BinaryDataRef header80B);
   void unserialize(BinaryRefReader brr);

   void unserializeFullBlock(BinaryDataRef block, 
                             bool doFrag=true,
                             bool withPrefix8=false);

   void unserializeFullBlock(BinaryRefReader brr, 
                             bool doFrag=true,
                             bool withPrefix8=false);

   bool serializeFullBlock( BinaryWriter & bw) const;

   void unserializeDBValue( DB_SELECT         db,
                            BinaryRefReader & brr,
                            bool              ignoreMerkle = false);
   void   serializeDBValue( DB_SELECT         db,
                            BinaryWriter &    bw) const;

   void unserializeDBValue(DB_SELECT db, BinaryData const & bd, bool ignMrkl=false);
   void unserializeDBValue(DB_SELECT db, BinaryDataRef bdr,     bool ignMrkl=false);
   BinaryData serializeDBValue(DB_SELECT db) const;
   void       unserializeDBKey(DB_SELECT db, BinaryDataRef key);

   BinaryData getDBKey(bool withPrefix=true) const;

   bool isMerkleCreated(void) { return (merkle_.getSize() != 0);}


   void pprintOneLine(uint32_t indent=3);
   void pprintFullBlock(uint32_t indent=3);
   
   BinaryData     dataCopy_;
   BinaryData     thisHash_;
   uint32_t       numTx_;
   uint32_t       numBytes_;
   uint32_t       blockHeight_;
   uint8_t        duplicateID_;
   BinaryData     merkle_;
   bool           merkleIsPartial_;
   bool           isMainBranch_;
   bool           blockAppliedToDB_;

   bool           isPartial_;
   map<uint16_t, StoredTx> stxMap_;

   // We don't actually enforce these members.  They're solely for recording
   // the values that were unserialized with everything else, so that we can
   // leter check that DB data matches what we were expecting
   uint32_t        unserArmVer_;
   uint32_t        unserBlkVer_;
   ARMORY_DB_TYPE  unserDbType_;
   DB_PRUNE_TYPE   unserPrType_;
   MERKLE_SER_TYPE unserMkType_;
   
};


////////////////////////////////////////////////////////////////////////////////
class StoredTx
{
public:
   StoredTx(void) : thisHash_(0), 
                    dataCopy_(0), 
                    blockHeight_(UINT32_MAX),
                    duplicateID_(UINT8_MAX),
                    txIndex_(UINT16_MAX),
                    numTxOut_(UINT16_MAX),
                    numBytes_(UINT32_MAX),
                    fragBytes_(UINT32_MAX) {}
   
   bool       isInitialized(void) const {return dataCopy_.getSize() > 0;}
   bool       haveAllTxOut(void) const;

   StoredTx&  createFromTx(Tx & tx, 
                           bool doFrag=true, 
                           bool withTxOuts=true);
   StoredTx & createFromTx(BinaryDataRef rawTx, 
                           bool doFrag=true, 
                           bool withTxOuts=true);

   BinaryData getSerializedTx(void) const;
   BinaryData getSerializedTxFragged(void) const;
   Tx         getTxCopy(void) const;
   void       setKeyData(uint32_t height, uint8_t dup, uint16_t txIdx); 

   void addTxOutToMap(uint16_t idx, TxOut & txout);
   void addStoredTxOutToMap(uint16_t idx, StoredTxOut & txout);

   void unserialize(BinaryData const & data, bool isFragged=false);
   void unserialize(BinaryDataRef data,      bool isFragged=false);
   void unserialize(BinaryRefReader & brr,   bool isFragged=false);

   void       unserializeDBValue(BinaryRefReader & brr);
   void         serializeDBValue(BinaryWriter &    bw ) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(void) const;
   void       unserializeDBKey(BinaryDataRef key);

   BinaryData getDBKey(bool withPrefix=true) const;
   BinaryData getDBKeyOfChild(uint16_t i, bool withPrefix=true) const;

   void pprintOneLine(uint32_t indent=3);
   void pprintFullTx(uint32_t indent=3);


   BinaryData           thisHash_;
   uint32_t             lockTime_;

   BinaryData           dataCopy_;
   bool                 isFragged_;
   uint32_t             version_;
   uint32_t             blockHeight_;
   uint8_t              duplicateID_;
   uint16_t             txIndex_;
   uint16_t             numTxOut_;
   uint32_t             numBytes_;
   uint32_t             fragBytes_;
   map<uint16_t, StoredTxOut> stxoMap_;

   // We don't actually enforce these members.  They're solely for recording
   // the values that were unserialized with everything else, so that we can
   // leter check that it
   uint32_t          unserArmVer_;
   uint32_t          unserTxVer_; 
   TX_SERIALIZE_TYPE unserTxType_;
};


////////////////////////////////////////////////////////////////////////////////
class StoredTxOut
{
public:
   StoredTxOut(void) : txVersion_(UINT32_MAX), 
                       dataCopy_(0), 
                       blockHeight_(UINT32_MAX), 
                       duplicateID_(UINT8_MAX), 
                       parentHash_(0),
                       txIndex_(UINT16_MAX), 
                       txOutIndex_(UINT16_MAX), 
                       spentness_(TXOUT_SPENTUNK), 
                       isCoinbase_(false), 
                       spentByTxInKey_(0) {}

   bool isInitialized(void) const {return dataCopy_.getSize() > 0;}
   void unserialize(BinaryData const & data);
   void unserialize(BinaryDataRef data);
   void unserialize(BinaryRefReader & brr);

   void       unserializeDBValue(BinaryRefReader &  brr);
   void         serializeDBValue(BinaryWriter & bw, bool forceSaveSpent=false) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(bool forceSaveSpent=false) const;
   void       unserializeDBKey(BinaryDataRef key);

   BinaryData getDBKey(bool withPrefix=true) const;
   BinaryData getDBKeyOfParentTx(bool withPrefix=true) const;

   StoredTxOut & createFromTxOut(TxOut & txout); 
   BinaryData    getSerializedTxOut(void) const;
   TxOut         getTxOutCopy(void) const;

   BinaryData    getScrAddress(void) const;
   BinaryDataRef getScriptRef(void) const;
   uint64_t      getValue(void) const;

   bool matchesDBKey(BinaryDataRef dbkey) const;

   uint64_t getValue(void) 
   { 
      if(dataCopy_.getSize()>=8)
         return READ_UINT64_LE(dataCopy_.getPtr());
      else
         return UINT64_MAX;
   }
         

   void pprintOneLine(uint32_t indent=3);

   uint32_t          txVersion_;
   BinaryData        dataCopy_;
   uint32_t          blockHeight_;
   uint8_t           duplicateID_;
   uint16_t          txIndex_;
   uint16_t          txOutIndex_;
   BinaryData        parentHash_;
   TXOUT_SPENTNESS   spentness_;
   bool              isCoinbase_;
   BinaryData        spentByTxInKey_;

   // We don't actually enforce these members.  They're solely for recording
   // the values that were unserialized with everything else, so that we can
   // leter check that it
   uint32_t          unserArmVer_;
};


////////////////////////////////////////////////////////////////////////////////
class StoredScriptHistory
{
public:

   StoredScriptHistory(void) : uniqueKey_(0), 
                               version_(UINT32_MAX),
                               alreadyScannedUpToBlk_(0) { }
                               

   bool isInitialized(void) { return uniqueKey_.getSize() > 0; }

   void       unserializeDBValue(BinaryRefReader & brr);
   void         serializeDBValue(BinaryWriter    & bw ) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(void) const;
   void       unserializeDBKey(BinaryDataRef key, bool withPrefix=true);

   BinaryData    getDBKey(bool withPrefix=true) const;
   SCRIPT_PREFIX getScriptType(void) const;

   void pprintOneLine(uint32_t indent=3);
   void pprintFullSSH(uint32_t indent=3);

   uint64_t getScriptReceived(bool withMultisig=false);
   uint64_t getScriptBalance(bool withMultisig=false);

   BinaryData     uniqueKey_;  // includes the prefix byte!
   uint32_t       version_;
   uint32_t       alreadyScannedUpToBlk_;

   // TODO: Maybe these vectors should instead be set<>'s or map<>'s for
   //       efficient search and update...?   On the other hand, it's 
   //       currently sorted chronologically which has other benefits
   vector<TxIOPair> txioVect_;
   vector<BinaryData> multisigDBKeys_;
};




////////////////////////////////////////////////////////////////////////////////
// TODO:  it turns out that outPointsAddedByBlock_ is not "right."  If a Tx has
//        20 txOuts, there's no reason to write 36 * 20 = 720 bytes when you 
//        can just as easily write the header once, and the numTxOut and create
//        the OutPoints yourself.  Will fix this later.
class StoredUndoData
{
public:
   StoredUndoData(void) {}

   bool isInitialized(void) { return (outPointsAddedByBlock_.size() > 0);}

   void       unserializeDBValue(BinaryRefReader & brr);
   void         serializeDBValue(BinaryWriter    & bw ) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(void) const;

   BinaryData getDBKey(bool withPrefix=true) const;

   BinaryData  blockHash_;
   uint32_t    blockHeight_;
   uint8_t     duplicateID_;

   vector<StoredTxOut>  stxOutsRemovedByBlock_;
   vector<OutPoint>     outPointsAddedByBlock_;
};


////////////////////////////////////////////////////////////////////////////////
class StoredTxHints
{
public:
   StoredTxHints(void) : txHashPrefix_(0), dbKeyList_(0), preferredDBKey_(0) {}

   bool isInitialized(void) { return txHashPrefix_.getSize() > 0; }

   uint32_t      getNumHints(void) const   { return dbKeyList_.size();      }
   BinaryDataRef getHint(uint32_t i) const { return dbKeyList_[i].getRef(); }

   void setPreferredTx(uint32_t height, uint8_t dupID, uint16_t txIndex) 
      { preferredDBKey_ = DBUtils.getBlkDataKeyNoPrefix(height,dupID,txIndex); }
   void setPreferredTx(BinaryData dbKey6B_) { preferredDBKey_ = dbKey6B_; }

   void       unserializeDBValue(BinaryRefReader & brr);
   void         serializeDBValue(BinaryWriter    & bw ) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(void) const;
   void       unserializeDBKey(BinaryDataRef key, bool withPrefix=true);

   BinaryData getDBKey(bool withPrefix=true) const;

   BinaryData         txHashPrefix_; 
   vector<BinaryData> dbKeyList_;
   BinaryData         preferredDBKey_;
};

////////////////////////////////////////////////////////////////////////////////
class StoredHeadHgtList
{
public:
   StoredHeadHgtList(void) : height_(UINT32_MAX), preferredDup_(UINT8_MAX) {}

   void       unserializeDBValue(BinaryRefReader & brr);
   void         serializeDBValue(BinaryWriter    & bw ) const;
   void       unserializeDBValue(BinaryData const & bd);
   void       unserializeDBValue(BinaryDataRef      bd);
   BinaryData   serializeDBValue(void) const;
   void       unserializeDBKey(BinaryDataRef key);

   void addDupAndHash(uint8_t dup, BinaryDataRef hash)
   {
      for(uint32_t i=0; i<dupAndHashList_.size(); i++)
      {
         if(dupAndHashList_[i].first == dup)
         {
            if(dupAndHashList_[i].second != hash)
               LOGERR << "Pushing different hash into existing HHL dupID"; 
            dupAndHashList_[i] = pair<uint8_t, BinaryData>(dup,hash);
            return;
         }
      }
      dupAndHashList_.push_back(pair<uint8_t, BinaryData>(dup,hash));
   }

   BinaryData getDBKey(bool withPrefix=true) const;

   bool isInitialized(void) { return (height_ != UINT32_MAX);}

   void setPreferredDupID(uint8_t newDup) {preferredDup_ = newDup;}

   uint32_t           height_;
   vector<pair<uint8_t, BinaryData> > dupAndHashList_;
   uint8_t            preferredDup_;
};




#endif

