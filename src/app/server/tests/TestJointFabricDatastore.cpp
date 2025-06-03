#include "app/server/JointFabricDatastore.h"
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

namespace {

class DummyListener : public JointFabricDatastore::Listener
{
public:
    void MarkNodeListChanged() override { mNotified = true; }
    void Reset() { mNotified = false; }

    bool mNotified = false;
};

TEST(JointFabricDatastoreTest, AddPendingNodeNotifiesListener)
{
    JointFabricDatastore store;
    DummyListener listener;

    store.AddListener(listener);

    // Add a pending node — should notify the listener via MarkNodeListChange
    store.AddPendingNode(123, CharSpan::fromCharString("controller-a"));

    EXPECT_TRUE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, RemoveListenerPreventsNotification)
{
    JointFabricDatastore store;
    DummyListener listener;

    store.AddListener(listener);
    store.RemoveListener(listener);
    listener.Reset();

    store.AddPendingNode(456, CharSpan::fromCharString("controller-b"));

    EXPECT_FALSE(listener.mNotified);
}

} // namespace
