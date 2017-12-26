thrift -out commonif/ --gen cpp:cob_style TCommonIf.thrift
thrift -out roomsvr/ --gen cpp:cob_style TRoomSvrIf.thrift
thrift -out usermgrsvr/ --gen cpp:cob_style TUsermgrSvrIf.thrift
thrift -out consumesvr/ --gen cpp:cob_style TConsumeSvrIf.thrift 
thrift -out chatsvr/ --gen cpp:cob_style TChatSvrIf.thrift 
thrift -out tppushsvr/ --gen cpp:cob_style TTpPushSvrIf.thrift

thrift -out pushmsg/ --gen cpp:cob_style TPushMsgIf.thrift 
thrift -out php/ --gen php TPushMsgIf.thrift && thrift -out php/ --gen php TCommonIf.thrift

