## TC-IDM-10.3

This is a test for cluster revision conformance. It uses the spec data model XML files as the spec ground truth for cluster revisions, and checks all standard clusters on all endpoints.


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
   <td rowspan="2" >Problems with cluster revision on at least one cluster
<p>
(when you see this, look up in the logs for the specific problems. Anything marked as ProblemSeverity.ERROR will cause a test failure. The specific location will be marked in the description)
   </td>
   <td>Standard cluster found on device, but is not present in spec data
   </td>
   <td>The spec XML doesn’t have this cluster included. This can happen for clusters that are marked as fully in-progress in the spec. Note that during certification, provisional clusters are not allowed on devices.
   </td>
  </tr>
  <tr>
   <td>Revision found on cluster (`&lt;device cluster id>`) does not match revision listed in the spec (`&lt;spec cluster id>`)
   </td>
   <td>It is likely the zap was not updated to the latest cluster revision before testing.
   </td>
  </tr>
  <tr>
   <td colspan="2"> File "/root/python_testing/spec_parsing_support.py", line 482, in build_xml_clusters
    mask = clusters[descriptor_id].feature_map[code]
   </td>
   <td rowspan="2">
     This test relies on data model XML files, and they were not found. These should be in a directory called data_model. If this was run from the docker directly, you need to mount the data_model directory correctly. Please see TH documentation.
   </td>
   </tr>
   <tr>
    <td colspan="2"> No data model files found in specified directory "directory_path"
    </td>
   </tr>
</table>
