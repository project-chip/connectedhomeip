/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.chip.casting;

/** @deprecated Use the APIs described in /examples/tv-casting-app/APIs.md instead. */
@Deprecated
public class CommissioningCallbacks {
  /**
   * This is called when the PBKDFParamRequest is received and indicates the start of the session
   * establishment process
   */
  private SuccessCallback<Void> sessionEstablishmentStarted;

  /** This is called when the commissioning session has been established */
  private SuccessCallback<Void> sessionEstablished;

  /**
   * This is called when the PASE establishment failed (such as, when an invalid passcode is
   * provided) or PASE was established fine but then the fail-safe expired (including being expired
   * by the commissioner). The error param indicates the error that occurred during session
   * establishment or the error accompanying the fail-safe timeout.
   */
  private FailureCallback sessionEstablishmentError;

  /**
   * This is called when the PASE establishment failed or PASE was established fine but then the
   * fail-safe expired (including being expired by the commissioner) AND the commissioning window is
   * closed. The window may be closed because the commissioning attempts limit was reached or
   * advertising/listening for PASE failed.
   */
  private FailureCallback sessionEstablishmentStopped;

  /** This is called when the commissioning has been completed */
  private Object commissioningComplete;

  private CommissioningCallbacks(Builder builder) {
    this.sessionEstablishmentStarted = builder.sessionEstablishmentStarted;
    this.sessionEstablished = builder.sessionEstablished;
    this.sessionEstablishmentError = builder.sessionEstablishmentError;
    this.sessionEstablishmentStopped = builder.sessionEstablishmentStopped;
    this.commissioningComplete = builder.commissioningComplete;
  }

  public SuccessCallback<Void> getSessionEstablishmentStarted() {
    return sessionEstablishmentStarted;
  }

  public SuccessCallback<Void> getSessionEstablished() {
    return sessionEstablished;
  }

  public FailureCallback getSessionEstablishmentError() {
    return sessionEstablishmentError;
  }

  public FailureCallback getSessionEstablishmentStopped() {
    return sessionEstablishmentStopped;
  }

  public Object getCommissioningComplete() {
    return commissioningComplete;
  }

  public static class Builder {
    private SuccessCallback<Void> sessionEstablishmentStarted;
    private SuccessCallback<Void> sessionEstablished;
    private FailureCallback sessionEstablishmentError;
    private FailureCallback sessionEstablishmentStopped;
    private Object commissioningComplete;

    public Builder sessionEstablishmentStarted(SuccessCallback<Void> sessionEstablishmentStarted) {
      this.sessionEstablishmentStarted = sessionEstablishmentStarted;
      return this;
    }

    public Builder sessionEstablished(SuccessCallback<Void> sessionEstablished) {
      this.sessionEstablished = sessionEstablished;
      return this;
    }

    public Builder sessionEstablishmentError(FailureCallback sessionEstablishmentError) {
      this.sessionEstablishmentError = sessionEstablishmentError;
      return this;
    }

    public Builder sessionEstablishmentStopped(FailureCallback sessionEstablishmentStopped) {
      this.sessionEstablishmentStopped = sessionEstablishmentStopped;
      return this;
    }

    public Builder commissioningComplete(Object commissioningComplete) {
      this.commissioningComplete = commissioningComplete;
      return this;
    }

    public CommissioningCallbacks build() {
      return new CommissioningCallbacks(this);
    }
  }
}
