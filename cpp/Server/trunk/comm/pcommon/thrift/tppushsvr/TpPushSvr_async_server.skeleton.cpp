// This autogenerated skeleton file illustrates one way to adapt a synchronous
// interface into an asynchronous interface. You should copy it to another
// filename to avoid overwriting it and rewrite as asynchronous any functions
// that would otherwise introduce unwanted latency.

#include "TpPushSvr.h"
#include <thrift/protocol/TBinaryProtocol.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::async;

using boost::shared_ptr;

class TpPushSvrAsyncHandler : public TpPushSvrCobSvIf {
 public:
  TpPushSvrAsyncHandler() {
    syncHandler_ = std::auto_ptr<TpPushSvrHandler>(new TpPushSvrHandler);
    // Your initialization goes here
  }
  virtual ~TpPushSvrAsyncHandler();

  void proc_notifyChatMsg(tcxx::function<void()> cob, const TChatMsgPush& msg, const int32_t groupid, const bool notify_offline) {
    syncHandler_->proc_notifyChatMsg(msg, groupid, notify_offline);
    return cob();
  }

 protected:
  std::auto_ptr<TpPushSvrHandler> syncHandler_;
};
