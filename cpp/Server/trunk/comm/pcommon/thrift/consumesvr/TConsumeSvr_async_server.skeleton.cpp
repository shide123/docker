// This autogenerated skeleton file illustrates one way to adapt a synchronous
// interface into an asynchronous interface. You should copy it to another
// filename to avoid overwriting it and rewrite as asynchronous any functions
// that would otherwise introduce unwanted latency.

#include "TConsumeSvr.h"
#include <thrift/protocol/TBinaryProtocol.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::async;

using boost::shared_ptr;

class TConsumeSvrAsyncHandler : public TConsumeSvrCobSvIf {
 public:
  TConsumeSvrAsyncHandler() {
    syncHandler_ = std::auto_ptr<TConsumeSvrHandler>(new TConsumeSvrHandler);
    // Your initialization goes here
  }
  virtual ~TConsumeSvrAsyncHandler();

  void proc_modContributionListSwitch(tcxx::function<void(int32_t const& _return)> cob, const int32_t runid, const int32_t groupid, const bool Switch) {
    int32_t _return = 0;
    _return = syncHandler_->proc_modContributionListSwitch(runid, groupid, Switch);
    return cob(_return);
  }

  void proc_modGroupGainSetting(tcxx::function<void(int32_t const& _return)> cob, const int32_t runid, const int32_t groupid, const bool Switch, const int8_t rangeType, const int8_t percent) {
    int32_t _return = 0;
    _return = syncHandler_->proc_modGroupGainSetting(runid, groupid, Switch, rangeType, percent);
    return cob(_return);
  }

  void proc_modGroupUserGainSetting(tcxx::function<void(int32_t const& _return)> cob, const int32_t runid, const int32_t groupid, const int32_t userid, const int8_t percent) {
    int32_t _return = 0;
    _return = syncHandler_->proc_modGroupUserGainSetting(runid, groupid, userid, percent);
    return cob(_return);
  }

 protected:
  std::auto_ptr<TConsumeSvrHandler> syncHandler_;
};

