# Adding unit test for matter bridge cluster support
 - [Prerequisite](#prerequisites)
 - [Implementing Cluster Test](#implementing-cluster-test)

## Prerequisites
 - [Matter Bridge UT framework](./README.md)
 - [ClusterTestContext](#clustertestcontext)
 
### ClusterTestContext
The ClusterTestContext(unify::matter_bridge::Test::ClusterContext) is a template implemented to help test the clusters supported by matter bridge. The class template derives from UnifyBridgeContext class which in turn derives from chip::Test::AppContext and hence can be used as test context for any matter unit test suite.
It has 2 template arguments 
- `AttributeHandler class`:
    This is to be set to the attribute translator/access class which zap generated for the cluster to be tested
- `CommandHandler class`:
  This is to be set to command tranlater or CommandHandler class zap generated for the cluster to be tested
#### Attribute Test Template Function
This template function mocks a MQTT publish with given topic and payload and also mocks a Read atrribute from matter and checks if the read value matches expected value for the topic and payload. 

Additionally, there is template function that mocks a WriteAttribute and checks if the resulting MQTT message that the bridge publishes in response to command matches with the expected topic and payload passed to this function.

#### Command Test Template Function
This template function mocks the requested matter command and checks if the topic and payload of MQTT message that the bridge publishes in response to command matches with the expected topic and payload passed to this function.

Refer [ClusterTestContext.h](ClusterTestContext.h) for more details.

## Implementing Cluster Test
  1. For the test context of the test suite use the [ClusterTestContext](#clustertestcontext) template 
  2. In Initialize method given to test suite, 
     - call `TestContext::Initialize` method initialize matter UT framework
     - Initialize and register a endpoint support the required cluster and configure the required attributes within the cluster
        - Use `get_endpoint` method within testcontext defined in step 1 to fetch endpoint handle
        - Use `emplace_cluster` method in ep handle to create and add a cluster to ep
        - Fill the `attributes` and `supported_commands` members with required attributes and commands of the cluster to test
        - Use `register_endpoint` method within test context to registered the configured endpoint
  3. In Each test case added to the test suite,
     - Use [attribute_test](#attribute-test-template-function) template function within test context to test ReadAttribute for attributes of the cluster 
     - Use [attribute_write_test](#attribute-test-template-function) template function within test context to test WriteAttribute for attributes of the cluster 
     - Use [command_test](#command-test-template-function) template function within the test context to test commands of the cluster
