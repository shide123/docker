#include "node.h"
#include "node_impl.h"

INode * INode::CreateNode(key_type key, svr_name_type & svr_name, 
        service_type & service, IWatcher & watcher)
{
    return new CNode(key, svr_name, service, watcher);
}

