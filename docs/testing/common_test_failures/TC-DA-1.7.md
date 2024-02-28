## TC-DA-1.7

This test verifies that the DUTs are correctly following all the requirements for the Device Attestation certificate chain.


<table>
  <tr>
   <td><strong>Error text</strong>
   </td>
   <td><strong>Probable root cause</strong>
   </td>
  </tr>
  <tr>
   <td>Re-raising error and failing: found new invalid PAA: {filename}
   </td>
   <td>The PAA list that was specified for this test contains at least one invalid certificate. Check that all the files in the given directory are valid PAAs in DER format.
   </td>
  </tr>
  <tr>
   <td>This test requires 2 DUTs
   </td>
   <td>Two DUTs are used for this test because we need to verify that the DACs are provisioned per individual unit. This needs to be specified on the command line using <code>--dut-node-id nodeid1 nodeid2</code> (for commissioned nodes).
   </td>
  </tr>
  <tr>
   <td>Found matching public keys in different DUTs
   </td>
   <td>The two devices under test use the same DAC public key (aka, they are the same DAC). This is disallowed under section 6.2.2, which states “All commissionable Matter Nodes SHALL include a Device Attestation Certificate (DAC) and corresponding private key, unique to that Device.
Either this means the company has used a single DAC for all its products, or they’ve attempted to run this two-DUT test against a single DUT.
   </td>
  </tr>
  <tr>
   <td>DUT %d PAI (%s) not matched in PAA trust store
   </td>
   <td>The PAI does not chain up to a PAA in the given PAA directory. Verify the expected PAA and confirm that it is present in the DCL.
   </td>
  </tr>
  <tr>
   <td>PAI AKID must not be in denylist
   </td>
   <td>The PAI chains up to a well known test PAA. Translation: This DUT is using a test DAC from the SDK. This test must be run against production certificates.
   </td>
  </tr>
</table>

