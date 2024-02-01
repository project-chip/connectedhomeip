## TC-RR-1.1 

TC-RR-1.1 is a stress test of the DUT, where it attempts to verify that the DUT meets all the specification required minimums, and can handle having its variable length attributes filled completely to their stated supported sizes at the same time. This includes the fabric table, the group key table, groups clusters, labels, ACLs, subscriptions, CASE sessions, read paths etc.

This test is run on every push to the CI. It is verified to work on all-clusters, so the SDK implementation is correct. However, this is a stress test. Any failure in this test is most likely an indication of a real failure on the DUT either stemming from heap exhaustion or some kind of buffer starvation. 

