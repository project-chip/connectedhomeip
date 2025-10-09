/*
 *   Copyright (c) 2025 Project CHIP Authors
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
package chip.platform;

import android.nfc.Tag;
import android.nfc.tech.IsoDep;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;
import java.io.IOException;

public class AndroidNfcCommissioningManager implements NfcCommissioningManager {

  private static final String TAG = AndroidNfcCommissioningManager.class.getSimpleName();
  private AndroidChipPlatform mPlatform;
  private NfcCallback mNfcCallback;
  private IsoDep mIsoDep = null;

  private static final int TYPE4_SIMPLE_APDU_MAX_TX_SIZE =
      245; // NB: 245 is the optimum size to get the biggest TX chunks and thus the max throughput
  private static final int TYPE4_SIMPLE_APDU_MAX_RX_SIZE =
      250; // NB: 250 is the optimum size to get the biggest RX chunks and thus the max throughput

  // Max length for a Type4 R-APDU (as defined in the NFC Forum Type4 Tag TS)
  private static final int TYPE4_MAX_RAPDU_SIZE = 246;
  // Max length for a Type4 C-APDU (as defined in the NFC Forum Type4 Tag TS)
  private static final int TYPE4_MAX_CAPDU_SIZE = 246;
  private static final int TYPE4_HEADER_SIZE = 4;
  private static final byte TYPE4_CMD_SELECT = (byte) 0xA4;
  private static final byte TYPE4_CMD_SELECT_BY_NAME = (byte) 0x04;
  private static final byte TYPE4_CMD_SELECT_BY_FILE_ID = (byte) 0x00;
  private static final byte TYPE4_CMD_FIRST_OR_ONLY_OCCURENCE = (byte) 0x0C;
  private static final byte TYPE4_CMD_READ_BINARY = (byte) 0xB0;

  private static final int ISODEP_TIMEOUT_IN_MS = 5000;

  private static final int MAX_CHAINED_RESPONSE_SIZE = 2048; // arbitrary value
  private static byte[] chainedResponseBuffer = new byte[MAX_CHAINED_RESPONSE_SIZE];
  private static int chainedResponseLen;

  private HandlerThread handlerThread = null;
  private Handler workerHandler = null;

  @Override
  public void setNfcCallback(NfcCallback nfcCallback) {
    mNfcCallback = nfcCallback;
  }

  @Override
  public NfcCallback getCallback() {
    return mNfcCallback;
  }

  @Override
  public int init() {
    // Create and start the NfcWorkerThread
    handlerThread = new HandlerThread("NfcWorkerThread");
    handlerThread.start();
    Log.d(TAG, "NfcWorkerThread created");

    // Create a Handler associated with the HandlerThread's Looper
    workerHandler =
        new Handler(handlerThread.getLooper()) {
          @Override
          public void handleMessage(Message msg) {
            try {
              Log.d(TAG, "handleMessage");
              byte[] buf = (byte[]) msg.obj;
              byte[] response = sendChainedAPDUs(buf);
              if (response != null) {
                mPlatform.onNfcTagResponse(response);
              }
            } catch (Exception e) {
              e.printStackTrace();
              mPlatform.onNfcTagError();
            }
          }
        };

    return 0;
  }

  @Override
  public void shutdown() {
    if (handlerThread != null) {
      handlerThread.quitSafely();
      handlerThread = null;
    }

    // Prevent the use of the workerHandler
    workerHandler = null;
  }

  @Override
  public void setAndroidChipPlatform(AndroidChipPlatform platform) {
    mPlatform = platform;
  }

  @Override
  public void sendToNfcTag(byte[] buf) {
    Log.d(TAG, "AndroidNfcCommissioningManager sendToNfcTag");

    if (workerHandler == null) {
      Log.e(TAG, "NFC workerHandler is null!");
      return;
    }

    Message message = workerHandler.obtainMessage();
    message.obj = buf.clone(); // Create a copy of the byte array to ensure data integrity
    workerHandler.sendMessage(message);
  }

  /**
   * @brief Function to set the NFC Tag.
   * @param androidTag : NFC Tag (as defined in android.nfc.Tag)
   * @return None
   */
  public void setNFCTag(Tag androidTag) {
    mIsoDep = IsoDep.get(androidTag);
    if (mIsoDep == null) {
      Log.e(TAG, "mIsoDep is null");
      return;
    }

    boolean isExtendedLengthApduSupported = mIsoDep.isExtendedLengthApduSupported();
    Log.d(TAG, "isExtendedLengthApduSupported: " + isExtendedLengthApduSupported);

    workerHandler.post(
        new Runnable() {
          @Override
          public void run() {
            try {
              selectMatterApplication();
              setIsoDepTimeout(ISODEP_TIMEOUT_IN_MS);

            } catch (IOException e) {
              e.printStackTrace();
              mPlatform.onNfcTagError();
            }
          }
        });
  }

  ///////////////////////////////////////////////////////////////////////////////////

  // SW1=0x90 SW2=0x00 indicate a successful command
  boolean IsStatusSuccess(byte sw1, byte sw2) {
    return ((sw1 == ((byte) 0x90)) && (sw2 == 0x00));
  }

  // SW1=0x61 SW2=0xXX indicate that the last command was successful and that the tag is
  // transmitting
  // a chained response. It is used when the response is too long to be transmitted in one shot.
  // SW2 indicates the size of the data in the next block. It can be read by calling
  // 'GetResponse' command.
  boolean IsResponseBlockAvailable(byte sw1) {
    return (sw1 == ((byte) 0x61));
  }

  private void setIsoDepTimeout(int timeout) throws IOException {

    if (mIsoDep == null) {
      Log.e(TAG, "Error! mIsoDep is null!");
      return;
    }

    if (!mIsoDep.isConnected()) {
      mIsoDep.close();
      mIsoDep.connect();
    }

    Log.d(TAG, "setIsoDepTimeout to " + timeout);
    mIsoDep.setTimeout(timeout);
  }

  private byte[] selectMatterApplication() throws IOException {
    byte[] response;
    byte[] frame = new byte[TYPE4_HEADER_SIZE + 11];

    frame[0] = 0x00; // CLA
    frame[1] = TYPE4_CMD_SELECT; // INS
    frame[2] = TYPE4_CMD_SELECT_BY_NAME; // P1
    frame[3] = 0x0C; // P2
    frame[4] = (byte) 0x09; // Lc

    // AID for Matter: A0 00 00 09 09 8A 77 E4 01 (9 bytes)
    frame[5] = (byte) 0xA0;
    frame[6] = (byte) 0x00;
    frame[7] = (byte) 0x00;
    frame[8] = (byte) 0x09;
    frame[9] = (byte) 0x09;
    frame[10] = (byte) 0x8A;
    frame[11] = (byte) 0x77;
    frame[12] = (byte) 0xE4;
    frame[13] = (byte) 0x01;

    frame[14] = (byte) 0x00; // Le

    response = transceive("selectMatterApplication", frame);

    if ((response == null) || (response.length < 2)) {
      throw new IOException("Invalid NFC Type4 response");
    }

    int len = response.length;
    byte sw1 = response[len - 2];
    byte sw2 = response[len - 1];

    if (IsStatusSuccess(sw1, sw2)) {
      // Command OK
      if (len == 2) {
        // There are only the 2 status bytes
        return null;
      } else {
        // Return the result data, without the 2 status bytes
        byte[] result = new byte[len - 2];
        System.arraycopy(response, 0, result, 0, result.length);
        return result;
      }
    } else {
      printSw1Sw2(sw1, sw2);
      throw new IOException("Error! selectMatterApplication failed!");
    }
  }

  // Send Transport command via simple APDUs.
  // If the data doesn't fit in a single APDU, it will be splitted in
  // several chained APDUs.
  // If the response doesn't fit in 255 bytes, it will be chained also.
  // When the tag response is fully received, it will be transmitted
  // by onNfcTagResponse() callback.
  private byte[] sendChainedAPDUs(byte[] data) throws IOException {
    byte[] response = null;
    int totalLength = data.length;
    int nbrOfBytesSent = 0;
    int nbrOfBytesRemaining = data.length;

    Log.d(TAG, "sendChainedAPDUs (" + nbrOfBytesRemaining + " bytes)");

    while (nbrOfBytesRemaining > 0) {

      // Prepare a buffer containing the data to send in a single APDU
      int nbrOfBytesToSend = Math.min(nbrOfBytesRemaining, TYPE4_SIMPLE_APDU_MAX_TX_SIZE);
      byte[] dataToSend = new byte[nbrOfBytesToSend];
      System.arraycopy(data, nbrOfBytesSent, dataToSend, 0, dataToSend.length);

      boolean isLastBlock = (nbrOfBytesToSend == nbrOfBytesRemaining);

      response = sendTransportAPDU(dataToSend, isLastBlock, totalLength);
      if ((response == null) || (response.length < 2)) {
        throw new IOException("Invalid NFC Type4 response");
      }

      int len = response.length;
      byte sw1 = response[len - 2];
      byte sw2 = response[len - 1];

      if (isLastBlock) {
        // The last block has been sent.
        // The possible response are:
        //   SW1=0x90 SW2=0x00 if the transmission was successful and the recipient has
        //     transmitted a short response (less than 256 bytes).
        //   SW1=0x61 SW2=0xXX if the transmission was successful and the recipient has
        //     transmitted the first part of a chained response. SW2 indicates the size of
        //     the data in the next block.
        if ((IsStatusSuccess(sw1, sw2)) || (IsResponseBlockAvailable(sw1))) {
          // Response will be processed outside of the while loop
        } else {
          // Any others values are an error
          printSw1Sw2(sw1, sw2);
          throw new IOException("Error during chained APDUs");
        }
      } else {
        // This is an intermediate block so the only valid response is 0x90 0x00
        if (IsStatusSuccess(sw1, sw2)) {
          // The command was successfully sent
          // Continue with the next block
        } else {
          printSw1Sw2(sw1, sw2);
          throw new IOException("Error during chained APDUs");
        }
      }

      // APDU sent successfully
      nbrOfBytesSent += nbrOfBytesToSend;
      nbrOfBytesRemaining -= nbrOfBytesToSend;
    }

    if (nbrOfBytesRemaining != 0) {
      throw new IOException("Error during APDUs transmission!");
    }

    // The command has been successfully sent and we have received the first response block.
    // Process it and read the next blocks (if any)
    return processAPDUResponse(response);
  }

  // Process an APDU response.
  // If SW1=0x61, it means that the response is too big to be transmitted in a single packet.
  // A chained response is used in that case. SW2 indicates the size of the data in the next
  // response packet. The next response packet can be read thanks to a call to getResponse()
  // command.
  private byte[] processAPDUResponse(byte[] response) throws IOException {

    // Response should at least contain the 2 status bytes
    if ((response == null) || (response.length < 2)) {
      throw new IOException("Error! Invalid Type4 response");
    }

    resetChainedResponseBuffer();

    int len = response.length;
    byte sw1 = response[len - 2];
    byte sw2 = response[len - 1];

    if (IsStatusSuccess(sw1, sw2)) {
      // Response fits in a single block.
      // Simply drop the 2 status bytes and return it
      byte[] result = new byte[len - 2];
      System.arraycopy(response, 0, result, 0, result.length);
      return result;

    } else if (IsResponseBlockAvailable(sw1)) {
      // SW1=0x61 indicates a chained response.

      // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
      addDataToChainedResponseBuffer(response, len - 2);

      // Loop until there is no more response block.
      // SW2 indicates the number of bytes contained in the next block.
      while (IsResponseBlockAvailable(sw1)) {
        // If SW2 is 0x00 or if it is higher than TYPE4_SIMPLE_APDU_MAX_RX_SIZE, we clamp it to
        // TYPE4_SIMPLE_APDU_MAX_RX_SIZE.
        byte nextBlockLength =
            ((sw2 == 0x00) || ((int) sw2 > TYPE4_SIMPLE_APDU_MAX_RX_SIZE))
                ? (byte) TYPE4_SIMPLE_APDU_MAX_RX_SIZE
                : sw2;
        response = getResponse(nextBlockLength);

        if ((response == null) || (response.length < 2)) {
          throw new IOException("Invalid NFC Type4 response");
        }

        len = response.length;
        sw1 = response[len - 2];
        sw2 = response[len - 1];

        if (IsStatusSuccess(sw1, sw2)) {
          Log.d(TAG, "Chained response received successfully");

          // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
          addDataToChainedResponseBuffer(response, len - 2);

        } else if (IsResponseBlockAvailable(sw1)) {
          // We have successfully received a block and it is not the last one

          // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
          addDataToChainedResponseBuffer(response, len - 2);

        } else {
          printSw1Sw2(sw1, sw2);
          throw new IOException("Error! Invalid APDU response");
        }

        // If SW1 still equals to 0x61, continue to read the next response block
      }

      // Chained response received successfully
      byte[] result = getChainedResponseBufferData();
      return result;

    } else {
      printSw1Sw2(sw1, sw2);
      throw new IOException("Error! Invalid APDU response");
    }
  }

  private void printSw1Sw2(byte sw1, byte sw2) {
    Log.d(TAG, "SW1=" + String.format("0x%02X", sw1) + " SW2=" + String.format("0x%02X", sw2));
  }

  private void dumpResponse(byte[] response) {
    if ((response == null) || (response.length == 0)) {
      Log.e(TAG, "Error! Empty response!");
      return;
    }

    String frame = String.format("Response: %s", convertHexByteArrayToString(response));
    Log.d(TAG, frame);
  }

  /////////////////////////////////////////////////////////////////

  // Send a simple Transport APDU.
  // data contains the APDU data to send (This data might one part of chained APDUs).
  // isLastBlock indicates if this is the last APDU.
  // totalLength indicates the total length of chained APDU data.
  private byte[] sendTransportAPDU(byte[] data, boolean isLastBlock, int totalLength)
      throws IOException {
    byte[] frame = new byte[5 + data.length + 1];

    frame[0] = isLastBlock ? (byte) 0x80 : (byte) 0x90; // CLA
    frame[1] = 0x20; // INS
    frame[2] = (byte) ((totalLength >> 8) & 0xFF); // P1 (contains the totalLength's MSB)
    frame[3] = (byte) (totalLength & 0xFF); // P2 (contains the totalLength's LSB)
    frame[4] = (byte) data.length; // Lc

    System.arraycopy(data, 0, frame, 5, data.length);

    frame[5 + data.length] = (byte) TYPE4_SIMPLE_APDU_MAX_RX_SIZE; // Le

    byte[] response = transceive("sendTransportAPDU", frame);

    return response;
  }

  private byte[] getResponse(byte length) throws IOException {
    byte[] frame = new byte[5];

    frame[0] = 0x00; // CLA
    frame[1] = (byte) 0xC0; // INS
    frame[2] = 0x00;
    frame[3] = 0x00;
    frame[4] = length; // Le

    byte[] response = transceive("getResponse", frame);

    return response;
  }

  // An exception is returned if the response doesn't contained the 2 status Words SW1-SW2
  private byte[] transceive(String commandName, byte[] data) throws IOException {
    byte[] response;

    if (mIsoDep == null) {
      throw new IOException("Error! mIsoDep is null!");
    }

    if (!mIsoDep.isConnected()) {
      mIsoDep.close();
      mIsoDep.connect();
    }

    Log.d(
        TAG,
        "==> Send "
            + commandName
            + " command ("
            + data.length
            + " bytes) : "
            + convertHexByteArrayToString(data));

    response = mIsoDep.transceive(data);

    if ((response == null) || (response.length < 2)) {
      throw new IOException("Invalid NFC Type4 response");
    }

    dumpResponse(response);

    return response;
  }

  /////////////////////////////////////////////////////////////////
  // Functions managing "chainedResponseBuffer"

  private void resetChainedResponseBuffer() {
    Log.d(TAG, "resetChainedResponseBuffer()");
    chainedResponseLen = 0;
  }

  private void addDataToChainedResponseBuffer(byte[] data, int dataLen) throws IOException {
    if ((chainedResponseLen + dataLen) >= chainedResponseBuffer.length) {
      throw new IOException("Error! Too many data for chainedResponseBuffer!");
    }

    Log.d(TAG, "Add " + dataLen + " bytes to chainedResponseBuffer");

    System.arraycopy(data, 0, chainedResponseBuffer, chainedResponseLen, dataLen);

    chainedResponseLen += dataLen;
  }

  private byte[] getChainedResponseBufferData() {
    // chainedResponseBuffer contains 'chainedResponseLen' bytes
    byte[] result = new byte[chainedResponseLen];

    System.arraycopy(chainedResponseBuffer, 0, result, 0, chainedResponseLen);

    Log.d(
        TAG,
        "chainedResponse ("
            + chainedResponseLen
            + " bytes): "
            + convertHexByteArrayToString(result));

    return result;
  }

  /////////////////////////////////////////////////////////////////

  private static byte[] convertIntTo2BytesHexaFormat(int numberToConvert) throws IOException {
    if (numberToConvert >= 0 && numberToConvert <= 65535) {
      byte[] convertedNumber =
          new byte[] {(byte) ((numberToConvert & '\uff00') >> 8), (byte) (numberToConvert & 255)};
      return convertedNumber;
    } else {
      throw new IOException("Failed to convert Int to Hex!");
    }
  }

  private static String convertHexByteArrayToString(byte[] in) {
    final StringBuilder builder = new StringBuilder();
    for (byte b : in) {
      builder.append(String.format("%02x ", b));
    }
    return builder.toString();
  }

  /////////////////////////////////////////////////////////////////

}
