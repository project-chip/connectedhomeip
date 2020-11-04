#!/usr/bin/env python

import argparse
import asyncio
import coloredlogs
import logging
from dataclasses import dataclass

from construct import *


def EndsWithEmpty(x, lst, ctx):
  return not x


class QNameValidator(Validator):

  def _validate(self, obj, context, path):
    return obj[-1] == ""


class QNameArrayAdapter(Adapter):

  def _decode(self, obj, context, path):
    return ".".join(map(str, obj[:-1]))

  def _encode(self, obj, context, path):
    return list(map(str, obj.split("."))) + [""]


@dataclass
class AnswerPtr:
  offset: int


class AnswerPart(Subconstruct):

  def __init__(self):
    self.name = "AnswerPart"
    self.subcon = PascalString
    self.flagbuildnone = False
    self.parsed = None

  def _parse(self, stream, context, path):
    # read from the stream
    # return object
    len = stream.read(1)[0]

    if (len & 0xC0) == 0xC0:
      l2 = stream.read(1)[0]
      return AnswerPtr(((len & 0x3F) << 8) | l2)
    else:
      return stream.read(len)

  def _build(self, obj, stream, context, path):
    # write obj to the stream
    # return same value (obj) or a modified value
    # that will replace the context dictionary entry
    raise Error("Answer part build not yet implemented")

  def _sizeof(self, context, path):
    # return computed size (when fixed size or depends on context)
    # or raise SizeofError (when variable size or unknown)
    raise SizeofError("Answer part has avariable size")


def EndsWithEmptyOrPointer(x, lst, ctx):
  return (not x) or isinstance(x, AnswerPtr)


class IpAddressAdapter(Adapter):

  def _decode(self, obj, context, path):
    return ".".join(map(str, obj))

  def _encode(self, obj, context, path):
    return list(map(int, obj.split(".")))


IpAddress = IpAddressAdapter(Byte[4])

HEX = HexDump(GreedyBytes)

QNAME = QNameArrayAdapter(
    QNameValidator(RepeatUntil(EndsWithEmpty, PascalString(Byte, "utf8"))))

DNSAnswer = Struct(
    "NAME" / RepeatUntil(EndsWithEmptyOrPointer, AnswerPart()),
    "TYPE" / Enum(
        Int16ub,
        A=1,
        NS=2,
        CNAME=5,
        SOA=6,
        WKS=11,
        PTR=12,
        MX=15,
        TXT=16,
        AAA=28,
        SRV=33,
    ),
    "CLASS" / BitStruct(
        "FlushCache" / Flag,
        "CLASS" / Enum(BitsInteger(15), IN=1),
    ),
    "TTL" / Int32ub,
    "RDATA" / Prefixed(
        Int16ub,
        Switch(
            this.TYPE, {
                "TXT": GreedyRange(PascalString(Byte, "utf8")),
                "A": IpAddress,
                "AAAA": Array(16, Byte),
                "PTR": RepeatUntil(EndsWithEmptyOrPointer, AnswerPart()),
            },
            default=GreedyBytes)),
)

DNSQuery = Struct(
    "ID" / Int16ub,
    "Control" / BitStruct(
        "QR" / Default(Flag, False),
        "OpCode" /
        Default(Enum(BitsInteger(4), QUERY=0, IQUERY=1, STATUS=2), "QUERY"),
        "AA" / Default(Flag, False),
        "TC" / Default(Flag, False),
        "RD" / Default(Flag, False),
        "RA" / Default(Flag, False),
        "Z" / Padding(1),
        "AD" / Default(Flag, False),
        "CD" / Default(Flag, False),
        "Rcode" / Default(
            Enum(
                BitsInteger(4),
                OK=0,
                FORMAT_ERROR=1,
                SERVER_FAILURE=2,
                NAME_ERROR=3,
                NOT_IMPLEMENTED=4,
                REFUSED=5,
            ),
            "OK",
        ),
    ),
    "QuestionCount" / Rebuild(Int16ub, len_(this.Questions)),
    "AnswerCount" / Rebuild(Int16ub, len_(this.Answers)),
    "AuthorityCount" / Rebuild(Int16ub, len_(this.Authorities)),
    "AdditionalCount" / Rebuild(Int16ub, len_(this.Additionals)),
    "Questions" / Array(
        this.QuestionCount,
        Struct(
            "QNAME" / QNAME,
            "QTYPE" / Default(
                Enum(
                    Int16ub,
                    A=1,
                    NS=2,
                    CNAME=5,
                    SOA=6,
                    WKS=11,
                    PTR=12,
                    MX=15,
                    SRV=33,
                    AAAA=28,
                    ANY=255,
                ), "ANY"),
            "QCLASS" / BitStruct(
                "Unicast" / Default(Flag, False),
                "Class" / Default(Enum(BitsInteger(15), IN=1, ANY=255), "IN"),
            ),
        ),
    ),
    "Answers" / Default(Array(this.AnswerCount, DNSAnswer), []),
    "Authorities" / Default(Array(this.AuthorityCount, DNSAnswer), []),
    "Additionals" / Default(Array(this.AdditionalCount, DNSAnswer), []),
)


class EchoClientProtocol:

  def __init__(self, on_con_lost):
    self.on_con_lost = on_con_lost
    self.transport = None

  def connection_made(self, transport):
    self.transport = transport

    query = DNSQuery.build({
        "ID": 0,
        "Questions": [
            {
                "QNAME": "_googlecast._tcp.local",
                # "QNAME": "octopi.local",
                "QCLASS": {
                    "Unicast": True
                }
            },
        ],
        "Answers": [],
        "Authorities": [],
        "Additionals": [],
    })

    logging.info("Connection made")
    logging.info("Sending:\n%s", DNSQuery.parse(query))
    logging.info("BINARY:\n%s", HEX.parse(query))

    self.transport.sendto(query, ("224.0.0.251", 5353))
    logging.info("Query sent")

  def datagram_received(self, data, addr):
    logging.info("Received reply from: %r", addr)
    logging.debug(HEX.parse(data))
    logging.info(DNSQuery.parse(data))

  def error_received(self, exc):
    logging.error("Error")

  def connection_lost(self, exc):
    logging.error("Lost connection")
    self.on_con_lost.set_result(True)


async def main():
  loop = asyncio.get_running_loop()

  client_done = loop.create_future()

  transport, protocol = await loop.create_datagram_endpoint(
      lambda: EchoClientProtocol(client_done),
      local_addr=("0.0.0.0", 5388),
  )

  try:
    await client_done
  finally:
    transport.close()


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="mDNS test app")
  parser.add_argument(
      "--log-level",
      default=logging.INFO,
      type=lambda x: getattr(logging, x),
      help="Configure the logging level.",
  )
  args = parser.parse_args()

  logging.basicConfig(
      level=args.log_level,
      format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
  )
  coloredlogs.install(level=args.log_level)

  asyncio.run(main())
