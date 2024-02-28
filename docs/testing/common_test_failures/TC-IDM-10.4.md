
## TC-IDM-10.4

This test ensures that the submitted PICS for an endpoint exactly matches the reported element on the endpoint.

PICS XML files are set per endpoint, and need to include all of the clusters on the endpoint.


### Tooling

Here are some resources that can help with PICS setting, or for narrowing down errors on this test:

PICS generator - this tool will fill in the PICS for all the elements reported on the device. Note that this is not a complete PICS for the device, as there are numerous PICS related to things like manual test ability etc. that cannot be determined from the device. However, it is a good starting point to ease the PICS selection process and ensure an exact match for device elements.

[https://github.com/project-chip/connectedhomeip/tree/master/src/tools/PICS-generator](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/PICS-generator)

Device graph - visual representation of the device endpoints in graph form

[https://github.com/project-chip/connectedhomeip/tree/master/src/tools/device-graph](https://github.com/project-chip/connectedhomeip/tree/master/src/tools/device-graph)

Device minimal representation -  Shows the smallest set of optional elements that fully describe the device. Can be used to get a quick understanding of what is actually implemented. This is written as a python test, and it runs the device conformance test before outputting anything.

[https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/MinimalRepresentation.py](https://github.com/project-chip/connectedhomeip/blob/master/src/python_testing/MinimalRepresentation.py)


### Problems


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
   <td rowspan="2" >At least one PICS error was found for this endpoint
<p>
(when you see this, look up in the logs for the specific problems. Anything marked as ProblemSeverity.ERROR will cause a test failure. The specific location will be marked in the description)
   </td>
   <td>An element found on the device, but the corresponding PICS `&lt;pics>` was not found in pics list
   </td>
   <td rowspan="2" >Both of these problems just indicate that the provided PICS does not match the device exactly. Ensure:
<ul>

<li>a set of PICS XMLs is defined for each individual endpoint, and you are running the test against the correct endpoint with the correct set of PICS XML files.

<li>PICS XML file is present for all the clusters on your endpoint.

<li>the PICS as selected actually matches. The test uses the global attributes for each cluster to determine the correct set of PICS, so reading these attributes will give a ground truth for data on the device
</li>
</ul>
   </td>
  </tr>
  <tr>
   <td>PICS `&lt;pics>` found in PICS list, but not on device
   </td>
  </tr>
</table>

