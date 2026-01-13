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
    CHIP_ERROR err = store.AddPendingNode(123, CharSpan::fromCharString("controller-a"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_TRUE(listener.mNotified);
}

TEST(JointFabricDatastoreTest, RemoveListenerPreventsNotification)
{
    JointFabricDatastore store;
    DummyListener listener;

    store.AddListener(listener);
    store.RemoveListener(listener);
    listener.Reset();

    CHIP_ERROR err = store.AddPendingNode(456, CharSpan::fromCharString("controller-b"));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_FALSE(listener.mNotified);
}

} // namespace
