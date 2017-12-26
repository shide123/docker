/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <iostream>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/THttpClient.h>

#include "TUsermgrSvr.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace TUsermgrIf;

int main() {

  boost::shared_ptr<TTransport> transport(new THttpClient("121.12.118.32", 8160, "/"));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  TUsermgrSvrConcurrentClient client(protocol);

  try {
    transport->open();
   
    /*
    TJoinGroup msg;
    msg.userid = 123477;
    msg.groupid = 5;
    msg.inway = 10;
	msg.roletype = 0;
	msg.levelid = 0;
    int ret = client.proc_joingroup(msg);
    */

    //int ret = client.proc_quitgroup(123477, 5);
	
    int ret = client.proc_checkgroupmember(123477, 5);
    cout << "return ret: " << ret << endl;
    
    ret = client.proc_modifyUserRoletype(123477, 5, 3);
    cout << "return ret: " << ret << endl;
    
    ret = client.proc_checkgroupmember(123477, 5);
    cout << "return ret: " << ret << endl;
    transport->close();
  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }
}
