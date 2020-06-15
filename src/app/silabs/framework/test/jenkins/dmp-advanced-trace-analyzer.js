//
// File: dmp-advanced-trace-analyzer.js
//
// Usage: 
//  - $STUDIO_PATH/build/studio/studio_na -script dmp-advanced-trace-analyzer.js -isd input-file.isd
//
// Usage notes: 
//  - This script uses the trace scripting functionality of Simplicity Studio, needs studio_na. 
//  - The input should normally be an ISD file (Network Analyzer trace file) which has 
//    DMP advanced testing and tuning measurements (dmp-advanced-testing-tuning.bsh) recorded.
//
// Scope: 
//  - This script is for exporting various data from DMP Advanced Testing and Tuning measurements. 
//  - TODO

// var isDebug = false
var isDebug = true
 
// This is how you can access java classes without typing full name all the time
var Time = Java.type('com.silabs.java.utils.Time');
var RadioInfoMap = Java.type('com.ember.workbench.decoder.radio.RadioInfoMap');
var FrameDecorator = Java.type('com.ember.workbench.decoder.FrameDecorator');
var Event = Java.type('com.silabs.capture.event.Event')
var FrameUtil = Java.type('com.silabs.java.utils.FrameUtil');
var EventKey = Java.type('com.silabs.capture.type.EventKey');
var EventUtil = Java.type('com.ember.workbench.core.EventUtil');
   
var waitMacAck = -1

var eventNumber = 0
var nodes = []
var messages = []
var transactions = []
var transactionCntr = 0
var zclTestPacketCounterAct = 0

var tests = []

// We iterate over all event...
for each ( var event in isdFile.events() ) {
 

  // 1. Find transport keys
  //    --> Build up a chain of nodes with short address, count how many nodes we got
  // 2. Look for specific ZCL Packets
  //    --> zigbeeApplicationSupport.clusterIdV2 = 0x0001
  //    --> zclPayload.cmdId = 0x01
  //
  // 3. Check counter 
  //    --> zclPayload.manufCode
  //    --> if increased check for packet counter (or something) so that ACK can be identified for this
  //         --> zigbeeApplicationSupport.apsCounter
  //    --> if goes back to 0x00 --> Go Evaluate
  //
  // 4. Store time of transmission (hoping it always TX)
  //
  // Evaluate
  //    --> Average each transmission between each nodes 
  //    --> Print out Througput between each nodes
  //    --> Average each transmission between start and end nodes 
  //    --> Print out Througput between start and end nodes

  // MAC sequence
  var macSequence = event.data().get(EventKey.FRAME_FIFTEENFOUR, 'sequence')

  // New message to take care of, all the rest is just dropped blindly
  var isNewMessage = false

  // There are a few MAC ACKs that we are interested in.
  // These are the one that is responsed at the end of an APS transaction,
  // so that we have a good enough idea on the timing of that mesasge too.
  var isMacAck = false

  // Ignore all non-packets and almost all acks.
  if (!event.type().isPacket()) continue;
  // ack might be needed if it's for last packet of a transaction
  if ( EventUtil.isAck(event)) {
    if ( (waitMacAck > -1) && (macSequence == waitMacAck) ) {
      waitMacAck = -1
      isNewMessage = true
      isMacAck = true
    } else {
     continue;
    }

  }

  var eventTime = event.startTime()

  // Originator ID (build up from WSTK nickname + JLINK serial number)
  var originatorId = Event.getOriginatorId(event);

  // ***** MAC sturff *******
  // Extract long and short addresses
  var src = event.data().get(EventKey.FRAME_FIFTEENFOUR, 'source')
  var dest = event.data().get(EventKey.FRAME_FIFTEENFOUR, 'dest')
  var longSrc = event.data().get(EventKey.FRAME_FIFTEENFOUR, 'longSource')
  var longDest = event.data().get(EventKey.FRAME_FIFTEENFOUR, 'longDest')

  // Extract packet length
  // var pktLen = event.data().get(EventKey.FRAME_FIFTEENFOUR, 'packetLength1Byte')
  // var pktLen = 70
  

  // ***** NWK addresses *******
  var nwkDest = event.data().get(EventKey.FRAME_ZIGBEENETWORK, 'dest')

  // ***** transport key event ******
  // zigbeeApsCommand.commandId == 0x05
  var apsCommandId = event.data().get(EventKey.FRAME_ZIGBEEAPSCOMMAND, 'commandId')

  // ***** ZCL messages for test ******

  // zclPayload.cmdId = 0x01
  var zclCommandId = event.data().get(EventKey.FRAME_ZCLPAYLOAD, 'cmdId')
  // zigbeeApplicationSupport.clusterIdV2 = 0x0001
  var apsClusterId = event.data().get(EventKey.FRAME_ZIGBEEAPPLICATIONSUPPORT, 'clusterIdV2')

  // ***** APS ACK ******
  var isApsAck = false

  // zigbeeApplicationSupport.frameType = 0x02
  var apsFrameType = event.data().get(EventKey.FRAME_ZIGBEEAPPLICATIONSUPPORT, 'frameType')

  // APS counter
  var apsCounter = event.data().get(EventKey.FRAME_ZIGBEEAPPLICATIONSUPPORT, 'apsCounter')

  // APS payload
  var apsPayload = event.data().get(EventKey.FRAME_PAYLOAD)
  var pktLen = (apsPayload == null) ? 0 : (apsPayload.frameSize() + 5)

  // Catch every transport key event
  if (apsCommandId == 0x05) {
    // isDebug = true

    // store short ID for all identfied nodes
    // store source address too if we didn't find any nodes yet, 
    // as we need both nodes from the ver first event
    if (!nodes.length) {
      nodes.push({originator: originatorId, shortAddr: src})
    }

    // store the destination address for all such events
    // we dont know originator yet (as originator is tied to the source)
    var newNode = true      
    for each ( var node in nodes ) {
      if (node.shortAddr == dest) newNode = false
    }
    if (newNode) {
      nodes.push({originator: "?", shortAddr: dest})
    }

    // store originator ID if we happened to find this for a node we already stored previously
    for each ( var node in nodes ) {
      if ((node.shortAddr == src) && (node.originator == "?")) {
        node.originator = originatorId
      }
    }

  // Catch every ZCL messages with command ID 0x01 and APS cluster ID 0x0001
  } else if ((zclCommandId == 0x01) && (apsClusterId == 0x0001)) {
    // zcl packet "test counter"
    // zclPayload.manufCode
    var zclTestPacketCounter = event.data().get(EventKey.FRAME_ZCLPAYLOAD, 'manufCode')

    // New message from the APS source
    // (ie. same packet goes through each nodes in the chain)
    if (zclTestPacketCounterAct != zclTestPacketCounter) {
      zclTestPacketCounterAct = zclTestPacketCounter

      // new test packet counter means new transaction,
      // so take care of the previous ones
      transactions.push(messages.slice())
      messages = []

      // No need to wait for MAC ACK anymore if we didn't find it yet
      waitMacAck = -1

      // New test if the test packet counter is back to 0
      if (zclTestPacketCounter == 0) {
        // copy the whole messages object array to a new test
        tests.push(transactions.slice())
        transactions = []
      }
    }

    // This is supposed to be the last message of the transaction
    // Wait for MAC ACK so that timing can be stored for this too
    if (nwkDest == dest) {
      waitMacAck = macSequence
    }

    // isDebug = true
    isNewMessage = true

  // Message of an APS ACK transaction
  } else if (apsFrameType == 0x02) {

    // This is supposed to be the last message of the transaction
    // Wait for MAC ACK so that timing can be stored for this too
    if (nwkDest == dest) {
      waitMacAck = macSequence
    }

    // isDebug = true
    isApsAck = true
    isNewMessage = true
  } else if (!isNewMessage) {
    continue
  }

  // store new message if meaningful
  if (isNewMessage) {
    // messages.push({time: eventTime, isMacAck: isMacAck, pktLen: payload.length, source: src, destination: dest, nwkDestination: nwkDest, isApsAck: isApsAck, zclCounter: zclTestPacketCounterAct})
    messages.push({time: eventTime, isMacAck: isMacAck, pktLen: pktLen, source: src, destination: dest, nwkDestination: nwkDest, isApsAck: isApsAck, zclCounter: zclTestPacketCounterAct})
    isNewMessage = false
  }

  if (isDebug) {
    // ... and print out index, microsecond time, and summary for each.
    print("\n"+Time.formatMicrosecondTime(event.startTime()) + ' s: ')
    // print(event.startTime())
    print(eventTime)
    print("   - isApsAck " + isApsAck + " " + isApsAck.toString())
    print('   - Event #' + eventNumber + ': ' + event.summary())

    // ... determine if it is rx or rx...
    if ( event.type().isRx() ) {
      print('   - ' + originatorId + ': RX at ' + Time.formatMicrosecondTime(event.startTime()) )
    } else if ( event.type().isTx() ) {
      print('   - ' + originatorId + ': TX at ' + Time.formatMicrosecondTime(event.startTime()) )
    } else {
      print('   - ' + originatorId + ': event type OTHER at ' + Time.formatMicrosecondTime(event.startTime()) )
    }
   
    // ... and print out the radio info map.
    var rm = RadioInfoMap.get(event);
    if ( rm != null ) {
      for each ( var oid in rm.originators() ) {
        var t = rm.getTime(oid)
        print('   - reported by ' + oid + ': at '+ Time.formatMicrosecondTime(t) );
      }
    } else {
        print('   - NULL radio info reported.' );
    }
   
   
    // Create nice strings for source and destination
    var sourceString = ( longSrc == null
                         ? ( src == null ? '<NONE>' : FrameUtil.hex(src, 4) )
                         : FrameUtil.formatEUI64(longSrc) );
    var destString = ( longDest == null
                         ? ( dest == null ? '<NONE>' : FrameUtil.hex(dest, 4) )
                         : FrameUtil.formatEUI64(longDest) );
   
    // Print the line out.
    print('   - ' + sourceString  + ' => ' + destString);
  }

  // isDebug = false

  eventNumber++
}

// Store the last collection of messages and transactions as well
transactions.push(messages.slice())
tests.push(transactions.slice())



// ************* Print nodes originator and short addresses *************
if (isDebug) {
  print("*********************************");
  for each (node in nodes) {
    print(node.originator + " " + FrameUtil.hex(node.shortAddr, 4))
  }
}

function getNodeIdx(nodeShortAddr) {
  for (var nodeIdx=0; nodeIdx < nodes.length; nodeIdx++) {        
    var node = nodes[nodeIdx]
    if (node.shortAddr == nodeShortAddr) {
      return nodeIdx
    }
  }
  return -1
}


// ************* Get rid of unknown or repeated messages as well as empty transactions *************
for each (test in tests) {
  // Run through all transactions in the test
  for (transactionIdx=0; transactionIdx<test.length; transactionIdx++) {
    transaction = test[transactionIdx]

    // Run through all messages in the transaction
    for (var messageIdx=0; messageIdx < transaction.length; messageIdx++) {
      var message = transaction[messageIdx]

      // Let's look for messages that we should be totally unaware as coming from some unknown nodes
      if (((getNodeIdx(message.source) < 0) || (getNodeIdx(message.destination) < 0)) && (!message.isMacAck)) {
        if (isDebug) {          
          print("*** Message removed (1): "
                + message.time + " " 
                + message.pktLen + " " 
                + ((message.source==null) ? "NULL" : FrameUtil.hex(message.source,4)) + " " 
                + ((message.destination==null) ? "NULL" : FrameUtil.hex(message.destination,4)) + " " 
                + FrameUtil.hex(message.zclCounter,2) + " " 
                + message.isApsAck + " "
          )        
        }
        transaction.splice(messageIdx,1)
        // make sure we don't miss the next one 
        messageIdx--

      // Let's look for repeated messages
      } else {          
        // Run through all following messages in the transaction
        for (var messageNextIdx=messageIdx+1; messageNextIdx < transaction.length; messageNextIdx++) {
          var messageNext = transaction[messageNextIdx]
          // If we find same message wipe it out, so that making the calculation more easy.
          // Note: this is gonna result a worst-case timing, as the message time is going to be 
          // the difference between the very first message and the next message of the destination
          if ( (messageNext.source != null ) &&
               (messageNext.destination != null ) &&
               (messageNext.source == message.source ) &&
               (messageNext.destination == message.destination ) ) {

            if (isDebug) {          
              print("*** Message removed (2): "
                    + messageNext.time + " " 
                    + messageNext.pktLen + " " 
                    + ((messageNext.source==null) ? "NULL" : FrameUtil.hex(messageNext.source,4)) + " " 
                    + ((messageNext.source==null) ? "NULL" : FrameUtil.hex(messageNext.destination,4)) + " " 
                    + FrameUtil.hex(messageNext.zclCounter,2) + " " 
                    + messageNext.isApsAck + " "
              )        
            }
            // wipe out message at next idx
            transaction.splice(messageNextIdx,1)
            // make sure we don't miss the next one 
            messageNextIdx--
          }
        }
      }
    }

    // Get rid of full transaction if we happened to remove all messages from it.
    if (transaction.length == 0) {
      // wipe out message at next idx
      test.splice(transactionIdx,1)
      // make sure we don't miss the next one 
      transactionIdx--
    }
  }
}

// ************* Calculate throughputs *************
var testThroughputsPointToPoint = []
var testThroughputsEndToEnd = []
var transactionThroughputs = []

// Run through all tests
for each (test in tests) {

  // Create a nodes*nodes sized map to store throughput values
  transactionThroughputs = new Array(nodes.length)
  for (var idx=0; idx<nodes.length; idx++) {
    transactionThroughputs[idx] = new Array(nodes.length)
    for (var idy=0; idy<nodes.length; idy++) {
      transactionThroughputs[idx][idy] = {all:0, count:0, pktLenMax:0, pktLenMin:99999999}
    }
  }

  if (isDebug) print("*********************************");
  if (isDebug) print("test ++");

  // Run through all transactions in the test
  // for each (transaction in test) {
  for (var transactionIdx=0;  transactionIdx<test.length; transactionIdx++) {
    var transaction = test[transactionIdx]
    var transactionNext = test[transactionIdx+1]

    if (isDebug) print("transaction ++");
    
    if (transaction.length == 0) {
      print( "*** Zero length transaction! ")
      continue;
    }

    // Run through all messages in the transaction
    for (var messageIdx=0; messageIdx < transaction.length; messageIdx++) {        
      var message = transaction[messageIdx]
      var messageNext = transaction[messageIdx+1]

      if (isDebug) print("message ++");
      // We have already used timing of MAC ACK with the previous message,
      // so no need for further processing on this.
      if (message.isMacAck) {
        if (isDebug) 
          print(message.time + " " + message.isMacAck)          
        continue
      }

      if (isDebug) {          
        print(message.time + " " 
              + message.pktLen + " " 
              + FrameUtil.hex(message.source,4) + " " 
              + FrameUtil.hex(message.destination,4) + " " 
              + FrameUtil.hex(message.zclCounter,2) + " " 
              + message.isApsAck + " "
        )        
      }

      // Looks we have missed a MAC ACK. This is not terrible but unfortunate.
      // Means no time data for the last message in this transaction.
      if ((messageNext == null) 
          || ((!messageNext.isMacAck) 
               && (message.isApsAck != messageNext.isApsAck))) {          
        continue
      }

      // Make sure messages are ordered according to the theory
      if (!((messageNext.isMacAck) || (message.destination == messageNext.source))) {
        print("ERROR: Message disorder! This should never happen.")
        continue
      }

      // Get idx of source and destination
      var sourceIdx = getNodeIdx(message.source)
      var destinationIdx = getNodeIdx(message.destination)

      // Get and set throughput values
      var messageDuration = Math.abs(message.time - messageNext.time)

      // Store timing parameters of the message for averaging later
      transactionThroughputs[sourceIdx][destinationIdx].all += messageDuration
      transactionThroughputs[sourceIdx][destinationIdx].count += 1

      // Store max and min packet length, so if length would change we have a good estimate
      if (transactionThroughputs[sourceIdx][destinationIdx].pktLenMin > message.pktLen) {
        transactionThroughputs[sourceIdx][destinationIdx].pktLenMin = message.pktLen
      }
      if (transactionThroughputs[sourceIdx][destinationIdx].pktLenMax < message.pktLen) {
        transactionThroughputs[sourceIdx][destinationIdx].pktLenMax = message.pktLen
      }

      // Very first message of the test, let's store some routing data here
      if ((transactionIdx == 0) && (messageIdx==0)) {
        testThroughputsEndToEnd.push({nwkSource: message.source, nwkDestination: message.nwkDestination, all:0, count: 0, pktLenMax: 0, pktLenMin: 9999999})
      }

      if (isDebug) {          
        print(transactionThroughputs[sourceIdx][destinationIdx].all + " " + transactionThroughputs[sourceIdx][destinationIdx].count)
      }
    }

    // Mine out the overall time of the transaction, this is gonna be used for end-to-end throughput calculation
    if (transactionNext != null) {
      var messageFirstOfTransaction = transaction[0]
      var messageFirstOfNextTransaction = transactionNext[0]
      var transactionDuration = Math.abs(messageFirstOfTransaction.time - messageFirstOfNextTransaction.time)
      testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].all += transactionDuration
      testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].count += 1
      // We assume that all related messages have the same packet length during a test
      if (testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].pktLenMin > messageFirstOfTransaction.pktLen) {
        testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].pktLenMin = messageFirstOfTransaction.pktLen
      }
      if (testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].pktLenMax < messageFirstOfTransaction.pktLen) {
        testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].pktLenMax = messageFirstOfTransaction.pktLen
      }
      if (isDebug) {
        print("---> transaction throuput data: " 
              + testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].nwkSource + " " 
              + testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].nwkDestination + " " 
              + testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].all + " " 
              + testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].count + " "
              + testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].pktLenMin + " "
              + testThroughputsEndToEnd[testThroughputsEndToEnd.length-1].pktLenMax + " "
             )
      }
    }
  }

  testThroughputsPointToPoint.push(transactionThroughputs.slice())
}

// ************* Print throughputs *************
for (testIdx=0; testIdx<testThroughputsPointToPoint.length; testIdx++) {
  print("\n\n***************************************************************");

  if (testThroughputsEndToEnd[testIdx] == null) {
    print("*** Skipped testThroughputsEndToEnd[" + testIdx + "]")
    continue
  }

  //    --> Average each transmission between start and end nodes 
  //    --> Print out Througput between start and end nodes
  var dataLengthAvgBit = (testThroughputsEndToEnd[testIdx].pktLenMax + testThroughputsEndToEnd[testIdx].pktLenMin) / 2 * 8
  var testTimeAvgSec = testThroughputsEndToEnd[testIdx].all / testThroughputsEndToEnd[testIdx].count / 1000 / 1000
  var througputKBitPerSec = dataLengthAvgBit / testTimeAvgSec / 1000

  print("Test #" + testIdx + "\n")
  // Print network source and destination
  print("End-to-end results: 0x"      + FrameUtil.hex(testThroughputsEndToEnd[testIdx].nwkSource,4) 
        + " --> 0x" + FrameUtil.hex(testThroughputsEndToEnd[testIdx].nwkDestination,4) 
        + ": " + Number(througputKBitPerSec.toFixed(3)).toPrecision(5) + " kbit/sec"
        + "\n"
       )

  test = testThroughputsPointToPoint[testIdx]

  // Print heading line with destination node addresses
  print("Node-to-node results:")
  var text = "       "
  for each (node in nodes) {
    text += " 0x" + FrameUtil.hex(node.shortAddr,4) + " <-->"
  }
  print(text.slice(0, text.lastIndexOf("<")))

  //    --> Average each transmission between each nodes 
  //    --> Print out Througput between each nodes
  for (var sourceIdx = 0; sourceIdx < nodes.length; sourceIdx++) {
    // Print heading with source node address
    text = "0x" + FrameUtil.hex(nodes[sourceIdx].shortAddr,4) + " "

    // Print througputs for between all nodes
    for (var destinationIdx = 0; destinationIdx < nodes.length; destinationIdx++) {
      var dataLengthAvgBit = (test[sourceIdx][destinationIdx].pktLenMax + test[sourceIdx][destinationIdx].pktLenMin) / 2 * 8
      var testTimeAvgSec = test[sourceIdx][destinationIdx].all / test[sourceIdx][destinationIdx].count / 1000 / 1000
      var througputKBitPerSec = dataLengthAvgBit / testTimeAvgSec / 1000
      text += isNaN(througputKBitPerSec) ? "  NaN       " :  Number(througputKBitPerSec.toFixed(3)).toPrecision(7) + "    "
    }
    print(text)
  }
}
