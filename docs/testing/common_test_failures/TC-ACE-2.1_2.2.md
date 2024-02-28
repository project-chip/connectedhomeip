## TC-ACE-2.1,2.2

These tests both test that the attributes on the device enforce proper permissions for reads and writes. Because privilege enforcement for SDK cluster attributes are normally handled in common SDK code, it is uncommon to see a true failure on clusters that are tested in the CI. However, failure here are possible for new clusters, where it's possible for the access to be set incorrectly.

Timeouts are also possible on this test. Because both tests test every access privilege for every attribute on the device, the time required to run the test depends on the number of attributes and the network conditions.

<table>
  <tr>
   <td>Failure
   </td>
   <td>Problem
   </td>
   <td>Probably root cause
   </td>
  </tr>
  <tr>
   <td rowspan="4" >One or more access violations was found
<p>
(when you see this, look up in the logs for the specific problems. Anything marked as ProblemSeverity.ERROR will cause a test failure. The specific attribute will be marked in the description)
   </td>
   <td>Unexpected UnsupportedAccess writing attribute
   </td>
   <td rowspan="4" >Check the XML for the cluster and ensure the access tag matches the spec. Ensure the XML representation of the spec in the SDK data_model/clusters directory is up to date with the current spec
   </td>
  </tr>
  <tr>
   <td>Unexpected error writing attribute - expected Unsupported Access, got …
   </td>
  </tr>
  <tr>
   <td>Unexpected error writing non-writeable attribute - expected Unsupported Write, got {resp[0].Status}
   </td>
  </tr>
  <tr>
   <td>Error reading `&lt;cluster>`:`&lt;attribute>` = `&lt;error>`
   </td>
  </tr>
  <tr>
   <td>asyncio.exceptions.CancelledError
   </td>
   <td>no problem reported - test exception
   </td>
   <td>Test has timed out. If you have many attributes or a slow network, this may not indicate a true failure. If the test still appeared to be running when the timeout happened, try adjusting the timeout on in the config or command line using the `timeout` parameter (set in seconds).
   </td>
  </tr>
</table>
