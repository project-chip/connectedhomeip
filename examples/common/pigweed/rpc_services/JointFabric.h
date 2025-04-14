#include "joint_fabric_service/joint_fabric_service.rpc.pb.h"

namespace joint_fabric_service {

class JointFabric : public pw_rpc::nanopb::JointFabric::Service<JointFabric> {
 public:
  ::pw::Status TransferOwnership( const ::OwnershipContext& request, ::pw_protobuf_Empty& response);
};

}
