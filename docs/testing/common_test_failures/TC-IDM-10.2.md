
## TC-IDM-10.2

This is a test of device conformance. It uses the spec data model XML files as the spec ground truth and checks for standard feature, attribute and command conformance for all standard clusters on all endpoints.

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
   <td rowspan="4" >Problems with conformance
<p>
(when you see this, look up in the logs for the specific problems. Anything marked as ProblemSeverity.ERROR will cause a test failure. The specific location will be marked in the description)
   </td>
   <td> Attribute `&lt;x>` is required, but is not present on the DUT. Conformance: `&lt;attribute conformance>`, implemented features: `&lt;features>`
<p>
Also applies for commands and features
   </td>
   <td rowspan="2" >Check the conformance on the device and in the spec. If you believe the device is compliant, please file an issue.
   </td>
  </tr>
  <tr>
   <td> Command `&lt;x>` is included, but disallowed by conformance. Conformance: `&lt;conformance>`, implemented features: `&lt;features>`
   </td>
  </tr>
  <tr>
   <td>Standard attribute found on device, but not in spec
   </td>
   <td rowspan="2" >The spec XML doesn’t include an attribute or feature or command that’s currently on the device. This is possible if the cluster is under heavy flux, but should NOT happen during a standard certification of a device at a specific release.
   </td>
  </tr>
  <tr>
   <td>Disallowed feature with mask `&lt;mask>`
   </td>
  </tr>
</table>
