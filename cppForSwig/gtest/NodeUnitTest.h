////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (C) 2019, goatpig.                                              //
//  Distributed under the MIT license                                         //
//  See LICENSE-MIT or https://opensource.org/licenses/MIT                    //                                      
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _H_NODEUNITTEST
#define _H_NODEUNITTEST

#include <memory>
#include <vector>
#include <map>

#include "../BinaryData.h"
#include "../BtcUtils.h"

#include "../BitcoinP2p.h"
#include "../Blockchain.h"
#include "../ScriptRecipient.h"
#include "../BlockDataMap.h"

class NodeUnitTest : public BitcoinP2P
{
   struct MempoolObject
   {
      BinaryData rawTx_;
      BinaryData hash_;
      unsigned order_;

      bool operator<(const MempoolObject& rhs) const { return order_ < rhs.order_; }
   };

   std::map<BinaryDataRef, MempoolObject> mempool_;
   std::atomic<unsigned> counter_;
   
   std::shared_ptr<Blockchain> blockchain_;
   std::shared_ptr<BlockFiles> filesPtr_;

public:
   NodeUnitTest(
      const std::string& addr, const std::string& port, 
      uint32_t magic_word, 
      std::shared_ptr<Blockchain> bcPtr,
      std::shared_ptr<BlockFiles> filesPtr);

   //virtuals
   void connectToNode(bool async) {}

   void shutdown(void)
   {
      //clean up remaining lambdas
      BitcoinP2P::shutdown();
   }

   //locals
   void mockNewBlock(void);
   void mineNewBlock(const BinaryData& h160);
   void pushZC(const std::vector<BinaryData>& txVec);
   std::shared_ptr<Payload> getTx(const InvEntry& ie, uint32_t timeout);
};

#endif