package com.google.chip.chiptool.commissioner.thread;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.util.concurrent.CompletableFuture;

public interface ThreadCommissionerService {

  // This method returns Credential of a given Thread Network.
  // If the Commissioner Service doesn't possess the Network
  // Credentials already, it will fetch the Credentials from
  // a Border Agent (assisting device) using the Thread
  // Commissioning protocol.
  //
  // The pskc is used to securely connect to the Border Agent device.
  // If no pskc is given, the user will be asked to input it.
  // The Network Credentials will be saved in the Commissioner
  // Service before returning.
  CompletableFuture<ThreadNetworkCredential> FetchThreadNetworkCredential(@NonNull BorderAgentInfo borderAgentInfo, @Nullable byte[] pskc);

  // This method returns Credential of a given Thread Network stored in the database on the phone.
  CompletableFuture<ThreadNetworkCredential> getThreadNetworkCredential(@NonNull BorderAgentInfo borderAgentInfo);

  // This method deletes Credential of a given Thread Network stored in the database on the phone.
  CompletableFuture<Void> deleteThreadNetworkCredential(@NonNull BorderAgentInfo borderAgentInfo);

}
